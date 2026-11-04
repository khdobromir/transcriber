#include "portable_fonts.hpp"
#include "portable.hpp"
#include <QFileInfo>
#include <QCryptographicHash>
#include <QFile>
#include <QFont>
#include <QGuiApplication>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QRawFont>
#include <QSaveFile>
#include <fontconfig/fontconfig.h>
#include <dlfcn.h>
#include <memory>
#include <stdexcept>

namespace {
bool portableInitialized = false;
QString loadedFontconfig() {
    Dl_info info{};
    const auto* symbol = dlsym(RTLD_DEFAULT, "FcGetVersion");
    if (!symbol || !dladdr(symbol, &info) || !info.dli_fname) return {};
    return QFileInfo(QString::fromLocal8Bit(info.dli_fname)).canonicalFilePath();
}
} // namespace
bool initializePortableFonts() {
    const auto root = portable::root();
    if (root.isEmpty()) return false;
    const auto library = loadedFontconfig();
    if (!library.startsWith(root + "/usr/lib/"))
        throw std::runtime_error("Portable Fontconfig initialization failed: library is outside the bundle");
    const auto path = root + "/usr/share/transcribe/fonts.conf";
    const auto canonical = QFileInfo(path).canonicalFilePath();
    if (!canonical.startsWith(root + "/usr/share/transcribe/"))
        throw std::runtime_error("Portable Fontconfig initialization failed: missing or invalid private config");
    std::unique_ptr<FcConfig, decltype(&FcConfigDestroy)> config(FcConfigCreate(), &FcConfigDestroy);
    if (!config) throw std::runtime_error("Portable Fontconfig initialization failed: cannot create config");
    FcConfigSetSysRoot(config.get(), nullptr);
    const auto filename = path.toUtf8();
    // Fontconfig 2.15 leaks the resolved filename for an XML relative prefix.
    // Register the relocatable bundled fonts directly instead.
    const auto fonts = (root + "/usr/share/fonts").toUtf8();
    if (!FcConfigParseAndLoad(config.get(), reinterpret_cast<const FcChar8*>(filename.constData()), FcTrue) ||
        !FcConfigAppFontAddDir(config.get(), reinterpret_cast<const FcChar8*>(fonts.constData())) ||
        !FcConfigSetCurrent(config.get()))
        throw std::runtime_error("Portable Fontconfig initialization failed: cannot load private config");
    portableInitialized = true;
    return true;
}
void writePortableFontDiagnostics() {
    const auto output = qEnvironmentVariable("TRANSCRIBE_PORTABLE_DIAGNOSTICS");
    if (output.isEmpty()) return;
    const auto root = portable::root();
    if (root.isEmpty() || !portableInitialized) throw std::runtime_error("Portable font diagnostics require portable initialization");
    const auto library = loadedFontconfig();
    const auto font = QRawFont::fromFont(QGuiApplication::font());
    bool cyrillic = font.isValid();
    for (char32_t character = U'А'; character <= U'я'; ++character)
        cyrillic = cyrillic && font.supportsCharacter(static_cast<uint>(character));
    cyrillic = cyrillic && font.supportsCharacter(uint{0x0401}) && font.supportsCharacter(uint{0x0451});
    const bool bundled = library.startsWith(root + "/usr/lib/");
    QFile loaded(library);
    if (!loaded.open(QIODevice::ReadOnly)) throw std::runtime_error("Cannot inspect loaded Fontconfig");
    QCryptographicHash hash(QCryptographicHash::Sha256);
    if (!hash.addData(&loaded)) throw std::runtime_error("Cannot hash loaded Fontconfig");
    const auto digest = QString::fromLatin1(hash.result().toHex());
    QFile manifest(root + "/package-manifest.json");
    bool manifestMatch = false;
    if (manifest.open(QIODevice::ReadOnly)) {
        const auto files = QJsonDocument::fromJson(manifest.readAll()).object().value("files").toArray();
        for (const auto& value : files) {
            const auto entry = value.toObject();
            if (QFileInfo(root + '/' + entry.value("path").toString()).canonicalFilePath() == library &&
                entry.value("sha256").toString() == digest) manifestMatch = true;
        }
    }
    const QJsonObject report{{"portable_fontconfig", true}, {"bundled_fontconfig", bundled}, {"fontconfig_library", library}, {"fontconfig_version", FcGetVersion()}, {"bundle_root", root}, {"fontconfig_sha256", digest}, {"fontconfig_manifest_match", manifestMatch}, {"config", root + "/usr/share/transcribe/fonts.conf"}, {"cyrillic", cyrillic}};
    QSaveFile file(output);
    const auto bytes = QJsonDocument(report).toJson(QJsonDocument::Compact) + '\n';
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
        throw std::runtime_error("Cannot save portable font diagnostics");
    if (!bundled || !cyrillic || !manifestMatch) throw std::runtime_error("Portable Fontconfig diagnostics failed");
}
