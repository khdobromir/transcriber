#include "portable.hpp"
#include <QCoreApplication>
#include <QProcess>
#include <QUrl>
#include <QTimer>
#include <iostream>

int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    if (app.arguments().size() != 3 || !QUrl(app.arguments()[2]).isLocalFile()) return 64;
    try {
        QProcess process;
        process.setProcessEnvironment(portable::desktopEnvironment(QProcessEnvironment::systemEnvironment()));
        process.setUnixProcessParameters(QProcess::UnixProcessFlag::CreateNewSession |
                                         QProcess::UnixProcessFlag::DisconnectControllingTerminal);
        process.setStandardInputFile(QProcess::nullDevice());
        process.setStandardOutputFile(QProcess::nullDevice());
        process.setStandardErrorFile(QProcess::nullDevice());
        QObject::connect(&process, &QProcess::errorOccurred, &app, [&](QProcess::ProcessError error) {
            if (error == QProcess::FailedToStart) app.exit(126);
        });
        QObject::connect(&process, &QProcess::finished, &app, [&](int code, QProcess::ExitStatus status) {
            app.exit(status == QProcess::NormalExit ? code : 125);
        });
        QTimer::singleShot(0, &app, [&] { process.start(app.arguments()[1], {app.arguments()[2]}); });
        return app.exec();
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 126;
    }
}
