#include "language.h"
#include "mainwindow.h"
#include "telemetry.h"

#include <QApplication>
#include <QObject>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setApplicationName("QMineSweeper");
    QApplication::setApplicationDisplayName("QMineSweeper");
    QApplication::setOrganizationName("Mavrikant");
    QApplication::setOrganizationDomain("mavrikant.com");
    QApplication::setApplicationVersion(QString::fromUtf8(QMS_VERSION));

    const QString release = QStringLiteral("qminesweeper@") + QString::fromUtf8(QMS_VERSION);
    Telemetry::initialize(release);
    QObject::connect(&app, &QApplication::aboutToQuit, [] { Telemetry::shutdown(); });

    // Install the chosen translator before constructing any UI so that every
    // tr() call in MainWindow picks up the right strings.
    const QString locale = Language::resolveStartupLocale();
    if (!Language::applyTranslator(&app, locale))
    {
        Language::applyTranslator(&app, QStringLiteral("en"));
    }

    MainWindow w;
    w.show();
    return app.exec();
}
