#pragma once
#include <QProcessEnvironment>
#include <QString>
#include <array>

namespace portable {
inline constexpr std::array environmentVariables{
    "LD_LIBRARY_PATH", "QT_PLUGIN_PATH", "QML_IMPORT_PATH", "QML2_IMPORT_PATH",
    "FONTCONFIG_FILE", "QT_QPA_FONTDIR", "TRANSCRIBE_CA_BUNDLE", "QT_QPA_PLATFORMTHEME",
    "QT_QUICK_BACKEND", "QT_QPA_PLATFORM_PLUGIN_PATH", "PATH"};
QString root(const QProcessEnvironment& environment = QProcessEnvironment::systemEnvironment());
QProcessEnvironment desktopEnvironment(const QProcessEnvironment& environment);
} // namespace portable
