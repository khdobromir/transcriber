#include "portable.hpp"
#include <QFileInfo>
#include <stdexcept>

namespace portable {
QString root(const QProcessEnvironment& environment) {
#ifdef Q_OS_LINUX
    if (!environment.contains("TRANSCRIBE_PORTABLE_ROOT")) return {};
    const auto path = QFileInfo(environment.value("TRANSCRIBE_PORTABLE_ROOT")).canonicalFilePath();
    if (path.isEmpty() || !QFileInfo::exists(path + "/usr/bin/tools/.transcribe-bundle"))
        throw std::runtime_error("Invalid portable application root");
    return path;
#else
    Q_UNUSED(environment);
    return {};
#endif
}
QProcessEnvironment desktopEnvironment(const QProcessEnvironment& environment) {
    auto restored = environment;
    if (root(environment).isEmpty()) return restored;
    if (environment.value("TRANSCRIBE_PORTABLE_SNAPSHOT_VERSION") != "1")
        throw std::runtime_error("Missing portable desktop environment snapshot");
    for (const auto* variable : environmentVariables) {
        const auto name = QString::fromLatin1(variable);
        const auto prefix = "TRANSCRIBE_PORTABLE_ENV_" + name;
        if (!environment.contains(prefix + "_SET") || !environment.contains(prefix + "_VALUE"))
            throw std::runtime_error("Incomplete portable desktop environment snapshot");
        const auto state = environment.value(prefix + "_SET");
        if (state == "x") restored.insert(name, environment.value(prefix + "_VALUE"));
        else if (state.isEmpty()) restored.remove(name);
        else throw std::runtime_error("Invalid portable desktop environment snapshot");
    }
    for (const auto& key : restored.keys())
        if (key.startsWith("TRANSCRIBE_PORTABLE_") || key == "APPDIR" || key == "APPIMAGE" ||
            key == "APPIMAGE_EXTRACT_AND_RUN" || key == "ARGV0" || key == "OWD") restored.remove(key);
    return restored;
}
} // namespace portable
