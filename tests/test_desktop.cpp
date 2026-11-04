#include "backend.hpp"
#include "desktop.hpp"
#include "portable.hpp"
#include <QCoreApplication>
#include <QDir>
#include <QElapsedTimer>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QScopeGuard>
#include <QSemaphore>
#include <QTemporaryDir>
#include <QTest>
#include <QThread>
#include <csignal>

namespace {
void put(const QString& path, const QByteArray& bytes) {
    QDir().mkpath(QFileInfo(path).absolutePath());
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size()) qFatal("desktop fixture write failed");
}
QJsonObject read(const QString& path) {
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) return {};
    return QJsonDocument::fromJson(file.readAll()).object();
}
struct Environment {
    QProcessEnvironment original = QProcessEnvironment::systemEnvironment();
    QStringList changed;
    void set(const QString& name, const QString& value) {
        changed.append(name);
        qputenv(name.toUtf8(), value.toUtf8());
    }
    ~Environment() {
        for (const auto& key : changed) {
            if (original.contains(key)) qputenv(key.toUtf8(), original.value(key).toUtf8());
            else qunsetenv(key.toUtf8());
        }
    }
};
struct Fixture {
    QTemporaryDir temporary;
    Environment environment;
    QString root = temporary.path() + "/bundle", report = temporary.path() + "/report.json";
    Fixture() {
        put(root + "/usr/bin/tools/.transcribe-bundle", "1\n");
        environment.set("TRANSCRIBE_PORTABLE_ROOT", root);
        environment.set("TRANSCRIBE_PORTABLE_SNAPSHOT_VERSION", "1");
        environment.set("TEST_DESKTOP_REPORT", report);
        for (const auto* variable : portable::environmentVariables) {
            const auto name = QString::fromLatin1(variable);
            environment.set("TRANSCRIBE_PORTABLE_ENV_" + name + "_SET", "x");
            environment.set("TRANSCRIBE_PORTABLE_ENV_" + name + "_VALUE", "original " + name);
        }
    }
    QUrl uri(const QString& name) const { return QUrl::fromLocalFile(temporary.path() + '/' + name); }
    void release() const { put(report + ".release", "ready"); }
};
int opener(const QString& uri) {
    const auto report = qEnvironmentVariable("TEST_DESKTOP_REPORT");
    const auto environment = QProcessEnvironment::systemEnvironment();
    QJsonObject variables;
    for (const auto& key : environment.keys())
        if (key.startsWith("TRANSCRIBE_PORTABLE_") || key.startsWith("APPIMAGE") || key == "APPDIR") variables[key] = environment.value(key);
    for (const auto* key : portable::environmentVariables)
        if (environment.contains(key)) variables[QString::fromLatin1(key)] = environment.value(key);
    put(report, QJsonDocument(QJsonObject{{"pid", QCoreApplication::applicationPid()}, {"uri", uri}, {"environment", variables}}).toJson());
    if (uri.contains("long")) {
        QElapsedTimer deadline;
        deadline.start();
        while (!QFile::exists(report + ".release") && deadline.elapsed() < 15000) QThread::msleep(5);
    }
    put(report + ".done", "done");
    if (uri.contains("crash")) std::raise(SIGTERM);
    return uri.contains("fail") ? 3 : 0;
}
} // namespace
class DesktopTests : public QObject {
    Q_OBJECT
private slots:
    void init() {
        QTest::failOnWarning(QRegularExpression("QProcess: Destroyed while process.*"));
    }
    void environmentRestoresAbsenceEmptyAndValues() {
        Fixture fixture;
        for (const auto& value : {QString{}, QString("Путь 😀\n$(literal)")}) {
            for (const bool present : {false, true}) {
                auto input = QProcessEnvironment::systemEnvironment();
                input.insert("APPIMAGE", "/bundle.AppImage");
                for (const auto* variable : portable::environmentVariables) {
                    const auto name = QString::fromLatin1(variable);
                    input.insert(name, "bundle");
                    input.insert("TRANSCRIBE_PORTABLE_ENV_" + name + "_SET", present ? "x" : "");
                    input.insert("TRANSCRIBE_PORTABLE_ENV_" + name + "_VALUE", value);
                }
                const auto before = input;
                const auto restored = portable::desktopEnvironment(input);
                QCOMPARE(input, before);
                for (const auto* variable : portable::environmentVariables) {
                    QCOMPARE(restored.contains(variable), present);
                    if (present) QCOMPARE(restored.value(variable), value);
                }
                for (const auto& key : restored.keys()) QVERIFY(!key.startsWith("TRANSCRIBE_PORTABLE_"));
                QVERIFY(!restored.contains("APPIMAGE"));
            }
        }
        auto invalid = QProcessEnvironment::systemEnvironment();
        invalid.remove("TRANSCRIBE_PORTABLE_ENV_PATH_SET");
        QVERIFY_THROWS_EXCEPTION(std::runtime_error, portable::desktopEnvironment(invalid));
    }
    void exitsAndFailures_data() {
        QTest::addColumn<QString>("name");
        QTest::addColumn<bool>("error");
        QTest::newRow("success") << QString("Файл 😀 $(literal)") << false;
        QTest::newRow("missing-handler") << QString("fail") << true;
        QTest::newRow("crash") << QString("crash") << true;
    }
    void exitsAndFailures() {
        QFETCH(QString, name);
        QFETCH(bool, error);
        Fixture fixture;
        DesktopOpener desktop(TEST_DESKTOP_HELPER, QCoreApplication::applicationFilePath());
        int completed = 0;
        UiText message;
        const auto before = QProcessEnvironment::systemEnvironment();
        desktop.open(0, fixture.uri(name), [&](const UiText& result) { message = result; ++completed; });
        QTRY_COMPARE(completed, 1);
        QCOMPARE(!message.isEmpty(), error);
        const auto result = read(fixture.report);
        QCOMPARE(result.value("uri").toString(), fixture.uri(name).toString(QUrl::FullyEncoded));
        const auto variables = result.value("environment").toObject();
        for (const auto* key : portable::environmentVariables) QCOMPARE(variables.value(key).toString(), "original " + QString::fromLatin1(key));
        for (const auto& key : variables.keys()) QVERIFY(!key.startsWith("TRANSCRIBE_PORTABLE_"));
        QCOMPARE(QProcessEnvironment::systemEnvironment(), before);
    }
    void missingLauncherAndHelper() {
        Fixture fixture;
        for (const bool helperMissing : {false, true}) {
            DesktopOpener desktop(helperMissing ? "/missing-helper" : TEST_DESKTOP_HELPER, "/missing-opener");
            int completed = 0;
            desktop.open(0, fixture.uri("target"), [&](const UiText& message) { QVERIFY(!message.isEmpty()); ++completed; });
            QTRY_COMPARE(completed, 1);
        }
    }
    void repeatedRequestsAndCloseKeepHandlerAlive() {
        Fixture fixture;
        int completed = 0;
        {
            DesktopOpener desktop(TEST_DESKTOP_HELPER, QCoreApplication::applicationFilePath());
            const auto done = [&](const UiText&) { ++completed; };
            desktop.open(0, fixture.uri("long"), done);
            QTRY_VERIFY(!read(fixture.report).isEmpty());
            const auto pid = read(fixture.report).value("pid").toInteger();
            for (int i = 0; i < 10; ++i) desktop.open(0, fixture.uri("long"), done);
            QTest::qWait(100);
            QCOMPARE(read(fixture.report).value("pid").toInteger(), pid);
            QCOMPARE(completed, 0);
        }
        const auto pid = read(fixture.report).value("pid").toInteger();
        QVERIFY(::kill(static_cast<pid_t>(pid), 0) == 0);
        fixture.release();
        QTRY_VERIFY(QFile::exists(fixture.report + ".done"));
        QCOMPARE(completed, 0);
    }
    void taskErrorsAreScopedAcrossRestart() {
        Fixture fixture;
        fixture.environment.set("TRANSCRIBE_TEST_MODE", "cancel");
        const auto input = fixture.temporary.path() + "/input.wav";
        put(input, "fixture");
        const auto engine = fixture.root + "/whisper.cpp/build/bin/whisper-cli";
        put(engine, "fixture");
        QFile::setPermissions(engine, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        put(fixture.root + "/models/ggml-medium-q5_0.bin", "fixture");
        put(fixture.root + "/models/ggml-silero-v6.2.0.bin", "fixture");
        Backend backend({TEST_GUI_HELPER, fixture.temporary.path() + "/settings.ini", fixture.root, {}, TEST_DESKTOP_HELPER, QCoreApplication::applicationFilePath()});
        const QVariantMap options{{"output", fixture.temporary.path() + "/long-fail-results"}};
        backend.start(input, options);
        QTRY_VERIFY(!backend.taskResultDirectory().isEmpty());
        backend.openTaskResult();
        QTRY_VERIFY(!read(fixture.report).isEmpty());
        const auto pid = read(fixture.report).value("pid").toInteger();
        backend.cancel();
        QTRY_VERIFY(!backend.busy());
        backend.start(input, options);
        QTRY_VERIFY(backend.busy());
        QVERIFY(::kill(static_cast<pid_t>(pid), 0) == 0);
        fixture.release();
        QTRY_VERIFY(QFile::exists(fixture.report + ".done"));
        QTest::qWait(100);
        QVERIFY(backend.taskOpenError().isEmpty());
        backend.cancel();
        QTRY_VERIFY(!backend.busy());
        const auto error = backend.error();
        backend.openTaskResult();
        QTRY_VERIFY(!backend.taskOpenError().isEmpty());
        QCOMPARE(backend.error(), error);
        QCOMPARE(backend.state(), TaskState::interrupted);
    }
    void errorsSurvivePreviewAndDoNotChangeNewSelection() {
        Fixture fixture;
        const auto directory = fixture.temporary.path() + "/long-fail";
        put(directory + "/transcripts/transcript.txt", "text");
        auto release = std::make_shared<QSemaphore>();
        Backend::Readers readers;
        readers.preview = [release](const QString&, const Backend::Cancel& cancel) {
            while (!release->tryAcquire(1, 10) && !cancel->load()) {}
            return QString("preview");
        };
        Backend backend({"missing", fixture.temporary.path() + "/settings.ini", fixture.root, {}, TEST_DESKTOP_HELPER, QCoreApplication::applicationFilePath()}, readers);
        backend.viewResult(directory);
        backend.openResult("txt");
        QTRY_VERIFY(!read(fixture.report).isEmpty());
        fixture.release();
        QTRY_VERIFY(!backend.resultOpenError().isEmpty());
        release->release();
        QTRY_COMPARE(backend.transcript(), QString("preview"));
        QVERIFY(!backend.resultOpenError().isEmpty());
        QVERIFY(backend.previewError().isEmpty());
        QVERIFY(backend.error().isEmpty());
        backend.openTaskResult();
        QVERIFY(!backend.taskOpenError().isEmpty());
        QVERIFY(backend.error().isEmpty());
        QFile::remove(fixture.report + ".release");
        QFile::remove(fixture.report);
        backend.openResult("txt");
        QTRY_VERIFY(!read(fixture.report).isEmpty());
        const auto pid = read(fixture.report).value("pid").toInteger();
        const auto other = fixture.temporary.path() + "/other";
        put(other + "/transcripts/transcript.txt", "other");
        backend.viewResult(other);
        QVERIFY(backend.resultOpenError().isEmpty());
        QVERIFY(::kill(static_cast<pid_t>(pid), 0) == 0);
        fixture.release();
        release->release();
        QTRY_COMPARE(backend.transcript(), QString("preview"));
        QTest::qWait(100);
        QVERIFY(backend.resultOpenError().isEmpty());
    }
};
int main(int argc, char** argv) {
    QCoreApplication app(argc, argv);
    if (app.arguments().size() == 2 && app.arguments()[1].startsWith("file:")) return opener(app.arguments()[1]);
    DesktopTests tests;
    return QTest::qExec(&tests, argc, argv);
}
#include "test_desktop.moc"
