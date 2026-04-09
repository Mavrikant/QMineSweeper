#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("QMineSweeper");
    QApplication::setApplicationDisplayName("QMineSweeper");
    QApplication::setOrganizationName("Mavrikant");
    QApplication::setOrganizationDomain("mavrikant.com");
    QApplication::setApplicationVersion(QString::fromUtf8(QMS_VERSION));

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages)
    {
        const QString baseName = "QMineSweeper_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName))
        {
            QApplication::installTranslator(&translator);
            break;
        }
    }

    MainWindow w;
    w.show();
    return app.exec();
}
