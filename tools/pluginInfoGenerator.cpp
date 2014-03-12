/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

bool processDescription(const QString &key, const QString &value,
                        QJsonObject &descriptions)
{
    static const QRegExp langRe("\\[(.+)\\]");
    QString destKey;
    QString lang;
    if (key == "Name" || key == "Comment") {
        destKey = key;
        lang = "en";
    } else if (key.startsWith("Name[")) {
        if (langRe.indexIn(key) == -1) {
            // no language, but it was a Name, so pretend we processed it
            return true;
        }

        lang = langRe.cap(1);
        destKey = "Name";
    } else if (key.startsWith("Comment[")) {
        if (langRe.indexIn(key) == -1) {
            // no language, but it was a Comment, so pretend we processed it
            return true;
        }

        lang = langRe.cap(1);
        destKey = "Comment";
    } else {
        return false;
    }


    QJsonObject obj = descriptions[lang].toObject();
    obj.insert(destKey, value);
    descriptions[lang] = obj;
    return true;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("pluginInfoGenerator");
    QCoreApplication::setApplicationVersion("0.1");

    QCommandLineParser opts;
    opts.setApplicationDescription("Merges translations from .desktop files into json files");
    opts.addHelpOption();
    opts.addVersionOption();

    QCommandLineOption desktopFileOpt("d",
                                   QCoreApplication::translate("main", "Path to the .desktop file"),
                                   QCoreApplication::translate("main", ".desktop file"));
    opts.addOption(desktopFileOpt);

    QCommandLineOption jsonFileOpt("j",
                                   QCoreApplication::translate("main", "Path to the json file"),
                                   QCoreApplication::translate("main", "json file")
                                  );
    opts.addOption(jsonFileOpt);


    QCommandLineOption migrateOption(QStringList() << "m" << "migrate",
                                     QCoreApplication::translate("main", "Performs a full migration of all values in the desktop file; otherwise only translated entries will be merged."));
    opts.addOption(migrateOption);

    opts.process(app);

    if (!opts.isSet(desktopFileOpt)) {
        opts.showHelp();
    }

    QFile desktopFile(opts.value(desktopFileOpt));
    if (!desktopFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open .desktop file:" << opts.value(desktopFileOpt);
        return -1;
    }

    const bool migrating = opts.isSet(migrateOption);
//     qDebug() << "*******************************" <<  migrating;
    //
    // Our various configuration value definitions
    ///
    QSet<QString> desktopListEntries;
    desktopListEntries << "Implements" << "Dependencies";

    QSet<QString> jsonArrayEntries;
    jsonArrayEntries << "Categories" << "Authors" << "Implements" << "Dependencies";

    // empty strings mean it is recognized, but deprecated and therefore skipped
    QHash<QString, QString> pluginInfoKeyDict;
    pluginInfoKeyDict.insert("X-KDE-ServiceTypes", "Implements");
    pluginInfoKeyDict.insert("ServiceTypes", "Implements");
    pluginInfoKeyDict.insert("X-KDE-PluginInfo-Author", "Authors");
    pluginInfoKeyDict.insert("X-KDE-PluginInfo-Version", "Version");
    pluginInfoKeyDict.insert("X-KDE-PluginInfo-License", "License");
    pluginInfoKeyDict.insert("X-KDE-PluginInfo-Depends", "Dependencies");
    pluginInfoKeyDict.insert("X-KDE-PluginInfo-EnabledByDefault", "EnabledByDefault");
    pluginInfoKeyDict.insert("Type", QString());
    pluginInfoKeyDict.insert("X-KDE-PluginInfo-Name", QString());
    pluginInfoKeyDict.insert("X-KDE-Library", QString());
    pluginInfoKeyDict.insert("Icon", "Icon");
    pluginInfoKeyDict.insert("Hidden", "Hidden");
    pluginInfoKeyDict.insert("X-KDE-PluginInfo-Category", "Categories");

    QHash<QString, QString> contactKeyDict;
    contactKeyDict.insert("X-KDE-PluginInfo-Email", "Email");
    contactKeyDict.insert("X-KDE-PluginInfo-Website", "Website");

    const char *desktopGroupName = "[Desktop Entry]";

    //
    // Read in the current json file, if any, and go from there
    //
    QString jsonFilePath = opts.value(jsonFileOpt);
    if (jsonFilePath.isEmpty()) {
        jsonFilePath = opts.value(desktopFileOpt).replace(QRegExp("\\.desktop$"), ".json");
    } else if (!jsonFilePath.endsWith(".json")) {
        jsonFilePath.append(".json");
    }

    QJsonDocument json;
    QFile jsonFile(jsonFilePath);
//     qDebug() << jsonFilePath;
    if (jsonFile.open(QIODevice::ReadOnly)) {
        const QByteArray data = jsonFile.readAll();
        jsonFile.close();

        QJsonParseError error;
        json = QJsonDocument::fromJson(data, &error);
        if (error.error != QJsonParseError::NoError) {
            qCritical() << "Malformed json in" << jsonFilePath;
            return -1;
        }
//         qDebug() << "read that fucker!" << data;
    }

    QJsonObject topObj = json.object();
    QJsonObject pluginInfo = topObj["PluginInfo"].toObject();
    QJsonObject descriptions = pluginInfo["Description"].toObject();
    QJsonObject contacts = pluginInfo["Contacts"].toObject();

    //
    // Start processing the .desktop file and populating the json objects
    //
    char buf[2048];
    const QRegExp keyValueRe("(.*)=(.*)");
    bool inDesktopGroup = false;

    while (1) {
        qint64 len = desktopFile.readLine(buf, sizeof(buf));
        if (len == -1) {
            break;
        }

        if (buf[0] == '\0') {
            continue;
        }

        if (buf[len - 1] == '\n') {
            // get rid of the trailing new line
            buf[len - 1] = '\0';
        }

        if (buf[0] == '[') {
            // Group section check; we really only care about DesktopGroup
            if (inDesktopGroup) {
                break;
            } else if (qstrcmp(buf, desktopGroupName) == 0) {
                inDesktopGroup = true;
                continue;
            }
        } else if (inDesktopGroup) {
            // being in the desktop group, now process the key
            if (keyValueRe.indexIn(buf) == -1) {
                continue;
            }
            const QString key = keyValueRe.cap(1);

            const QString value = keyValueRe.cap(2).trimmed();
            if (value.isEmpty()) {
                continue;
            }

            if (processDescription(key, value, descriptions) || !migrating) {
                continue;
            } else if (pluginInfoKeyDict.contains(key)) {
                const QString destKey = pluginInfoKeyDict.value(key);
//                 qDebug() << key << "is a pluginfo entry mapping to" << destKey;
                if (destKey.isEmpty()) {
                    continue;
                }

                QJsonValue jsonValue;
                if (jsonArrayEntries.contains(destKey)) {
                    QJsonArray array = pluginInfo[destKey].toArray();
//                     qDebug() << "      it is also an array entry";
                    QStringList values;
                    if (desktopListEntries.contains(destKey)) {
                        values = value.split(",", QString::SkipEmptyParts);
                    } else {
                        values << value;
                    }

                    for (auto v: values) {
                        if (!array.contains(v)) {
                            array.append(v);
                        }
                    }

                    if (array.isEmpty()) {
                        continue;
                    }

                    jsonValue = array;
                } else if (value == "false") {
                    jsonValue = false;
                } else if (value == "true") {
                    jsonValue = true;
                } else {
                    jsonValue = value;
                }

                pluginInfo.insert(destKey, jsonValue);
            } else if (contactKeyDict.contains(key)) {
//                 qDebug() << key << "is a contact entry";
                const QString destKey = contactKeyDict.value(key);
                if (destKey.isEmpty()) {
                    continue;
                }

                QJsonValue jsonValue;
                if (value == "false") {
                    jsonValue = false;
                } else if (value == "true") {
                    jsonValue = true;
                } else {
                    jsonValue = value;
                }
                contacts.insert(destKey, jsonValue);
            } else if (key.startsWith("X-")) {
//                 qDebug() << "got an X- key" << key;
                QStringList jsonObjNames = key.right(key.length() - 2).split('-', QString::SkipEmptyParts);
                if (jsonObjNames.isEmpty()) {
                    continue;
                }

                const QString destKey = jsonObjNames.takeLast();
//                 qDebug() << destKey << "..." << jsonObjNames;
                if (jsonObjNames.isEmpty()) {
                    // we don't know where to put this ... :/
                    qWarning() << "Found a key we don't know what to do with:" << key;
                    continue;
                }

                QVector<QJsonObject> objects;
                for (auto name: jsonObjNames) {
                    QJsonObject obj = topObj[name].toObject();
                    objects.append(obj);
                }
                objects.last().insert(destKey, value);
                for (int i = objects.size() - 1; i > 0; --i) {
                    QJsonObject obj = objects[i];
                    objects[i - 1].insert(jsonObjNames[i], objects[i]);
                }

//                 qDebug() << "inserting" << objects[0] << "as" << jsonObjNames[0];
                topObj.insert(jsonObjNames[0], objects[0]);
            } else {
                qWarning() << "Found a key we don't know what to do with:" << key;
            }
        }
    }

    //
    // Re-insert the modified results
    //
    // qDebug() << descriptions;
    pluginInfo.insert("Description", descriptions);
    pluginInfo.insert("Contacts", contacts);
    topObj.insert("PluginInfo", pluginInfo);
    json.setObject(topObj);

    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "Could not open json file for writing:" << jsonFilePath;
        return -1;
    }

    jsonFile.write(json.toJson());
    jsonFile.close();
    return 0;
}