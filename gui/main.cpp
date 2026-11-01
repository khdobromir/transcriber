#include "backend.hpp"
#ifdef Q_OS_LINUX
#include "portable_fonts.hpp"
#endif
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QtQml/qqmlextensionplugin.h>
#include <iostream>

// NOLINTNEXTLINE(bugprone-throwing-static-initialization): Qt requires this generated registration before QML engine startup.
Q_IMPORT_QML_PLUGIN(TranscribePlugin)
int main(int argc, char** argv) {
    try {
#ifdef Q_OS_LINUX
        initializePortableFonts();
#endif
        QQuickStyle::setStyle("Material");
        QGuiApplication app(argc, argv);
        app.setApplicationVersion(TRANSCRIBE_VERSION);
        app.setWindowIcon(QIcon(":/icons/icon.png"));
        app.setApplicationName("Transcribe");
        app.setOrganizationName("Transcribe");
        app.setDesktopFileName("transcribe-gui");
#ifdef Q_OS_LINUX
        writePortableFontDiagnostics();
#endif
        Backend backend;
        backend.initializeLanguage();
        QQmlApplicationEngine engine;
        QObject::connect(backend.language(), &LanguageController::changed, &engine, &QQmlEngine::retranslate);
        engine.setInitialProperties({{"backend", QVariant::fromValue(&backend)}});
        QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, [] { QCoreApplication::exit(1); }, Qt::QueuedConnection);
        engine.loadFromModule("Transcribe.App", "Main");
        return app.exec();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
