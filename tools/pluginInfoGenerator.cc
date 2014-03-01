
#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

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

    opts.process(app);

    if (!opts.isSet(desktopFileOpt) || !opts.isSet(jsonFileOpt)) {
        opts.showHelp();
    }

    qDebug() << "We have .. " << opts.isSet(desktopFileOpt) << opts.value(desktopFileOpt);
    QFile desktopFile(opts.value(desktopFileOpt));
    if (!desktopFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open .desktop file:" << opts.value(desktopFileOpt);
        return -1;
    }

    QJsonObject translations;
    QByteArray line;
    bool inDesktopGroup = false;
    const char *desktopGroupName = "[Desktop Entry]";
    char buf[1024];
    QRegExp keyValueRe("(.*)=(.*)");
    QRegExp langRe("\\[(.*)\\]");
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
            if (inDesktopGroup) {
                break;
            } else if (qstrcmp(buf, desktopGroupName) == 0) {
                inDesktopGroup = true;
                continue;
            }
        } else if (inDesktopGroup) {
            if (keyValueRe.indexIn(buf) == -1) {
                continue;
            }

            QString key = keyValueRe.cap(1);
            const QString value = keyValueRe.cap(2).trimmed();
            QString lang;
            if (key == "Name") {
                lang = "c";
            } else if (key.startsWith("Name[")) {
                if (langRe.indexIn(buf) != -1) {
                    lang = langRe.cap(1);
                }
                key = "Name";
            } else if (key == "Comment") {
                lang = "c";
            } else if (key.startsWith("Comment[")) {
                if (langRe.indexIn(buf) != -1) {
                    lang = langRe.cap(1);
                }
                key = "Comment";
            }

            QJsonObject obj = translations[lang].toObject();
            obj.insert(key, value);
            translations[lang] = obj;
        }
    }

//     qDebug() << translations;
    QFile jsonFile(opts.value(jsonFileOpt));
    if (!jsonFile.open(QIODevice::ReadOnly)) {
        qCritical() << "Could not open json file:" << opts.value(jsonFileOpt);
        return -1;
    }

    const QByteArray data = jsonFile.readAll();
    jsonFile.close();

    QJsonDocument json = QJsonDocument::fromJson(data);
    QJsonObject pluginInfo = json.object()["PluginInfo"].toObject();
    pluginInfo.insert("Description", translations);
    QJsonObject topObj = json.object();
    topObj.insert("PluginInfo", pluginInfo);
    json.setObject(topObj);

    if (!jsonFile.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        qCritical() << "Could not open json file for writing:" << opts.value(jsonFileOpt);
        return -1;
    }

    jsonFile.write(json.toJson());
    jsonFile.close();
    return 0;
}