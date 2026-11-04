#include "backend.hpp"
#include <QGuiApplication>
#include <QClipboard>
#include <QQmlApplicationEngine>
#include <QQuickItem>
#include <QQuickWindow>
#include <QQuickStyle>
#include <QTemporaryDir>
#include <QDir>
#include <QFile>
#include <QTest>
#include <QSignalSpy>
#include <QScopeGuard>
#include <QtQml/qqmlextensionplugin.h>
#include <cmath>

// NOLINTNEXTLINE(bugprone-throwing-static-initialization): Qt requires this generated registration before QML engine startup.
Q_IMPORT_QML_PLUGIN(TranscribePlugin)
class TranscriptViewTests : public QObject {
    Q_OBJECT
private slots:
    void streamingTranscriptPreservesReadingAndSelection() {
        QTemporaryDir temp;
        const auto root = temp.path() + "/data";
        const auto put = [](const QString& path, const QByteArray& bytes) {
            QDir().mkpath(QFileInfo(path).absolutePath());
            QFile file(path);
            if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size()) qFatal("transcript fixture failed");
            file.close();
            QFile::setPermissions(path, QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ExeOwner);
        };
        put(root + "/whisper.cpp/build/bin/whisper-cli"
#ifdef Q_OS_WIN
                   ".exe"
#endif
            ,
            "fixture");
        put(root + "/models/ggml-medium-q5_0.bin", "fixture");
        put(root + "/models/ggml-silero-v6.2.0.bin", "fixture");
        put(temp.path() + "/input.wav", "fixture");
        const auto previous = qgetenv("TRANSCRIBE_TEST_MODE");
        const auto restore = qScopeGuard([previous] {
            if (previous.isNull()) qunsetenv("TRANSCRIBE_TEST_MODE");
            else qputenv("TRANSCRIBE_TEST_MODE", previous);
        });
        qputenv("TRANSCRIBE_TEST_MODE", "cancel");
        Backend backend({TEST_GUI_HELPER, temp.path() + "/settings.ini", root});
        QQmlApplicationEngine engine;
        QStringList warnings;
        connect(&engine, &QQmlEngine::warnings, this, [&](const QList<QQmlError>& errors) {
            for (const auto& error : errors) warnings.append(error.toString());
        });
        engine.setInitialProperties({{"backend", QVariant::fromValue(&backend)}});
        engine.load(QUrl::fromLocalFile(GUI_QML_SOURCE));
        QVERIFY2(!engine.rootObjects().isEmpty(), qPrintable(warnings.join('\n')));
        auto* window = qobject_cast<QQuickWindow*>(engine.rootObjects().first());
        QVERIFY(window);
        auto* field = window->findChild<QQuickItem*>("transcriptText");
        auto* scroll = window->findChild<QQuickItem*>("transcriptScroll");
        QVERIFY(field && scroll);
        auto* viewport = scroll->property("contentItem").value<QObject*>();
        QVERIFY(viewport);
        backend.start(temp.path() + "/input.wav", {{"output", temp.path() + "/results"}});
        QTRY_VERIFY_WITH_TIMEOUT(backend.transcript().contains(QStringLiteral("частичный")), 3000);
        const auto file = backend.resultDirectory() + "/transcripts/transcript.txt";
        QString original;
        for (int i = 0; i < 200; ++i) original += QString("Строка %1: Лекция 😀\n").arg(i);
        put(file, original.toUtf8());
        QTRY_COMPARE(backend.transcript(), original);
        QTRY_COMPARE(field->property("text").toString(), original);
        QSignalSpy unchanged(&backend, &Backend::transcriptChanged);
        QTest::qWait(600);
        QCOMPARE(unchanged.count(), 0);
        QTRY_VERIFY(viewport->property("contentHeight").toDouble() > viewport->property("height").toDouble() + 200);
        viewport->setProperty("contentY", 100.0);
        const auto extended = original + "Новый фрагмент\n";
        put(file, extended.toUtf8());
        QTRY_COMPARE(backend.transcript(), extended);
        QCOMPARE(field->property("text").toString(), original);
        QVERIFY(std::abs(viewport->property("contentY").toDouble() - 100.0) < 1);
        QVERIFY(QMetaObject::invokeMethod(field, "select", Q_ARG(int, 0), Q_ARG(int, 6)));
        const auto selection = field->property("selectedText").toString();
        QVERIFY(!selection.isEmpty());
        QVERIFY(QMetaObject::invokeMethod(field, "copy"));
        QCOMPARE(QGuiApplication::clipboard()->text(), selection);
        const auto latest = extended + "Ещё один фрагмент\n";
        put(file, latest.toUtf8());
        QTRY_COMPARE(backend.transcript(), latest);
        QCOMPARE(field->property("selectedText").toString(), selection);
        QCOMPARE(field->property("text").toString(), original);
        auto* resume = window->findChild<QQuickItem*>("followTranscript");
        QVERIFY(resume);
        QTRY_VERIFY(resume->property("visible").toBool());
        QVERIFY(QMetaObject::invokeMethod(resume, "clicked"));
        QTRY_COMPARE(field->property("text").toString(), latest);
        QTRY_VERIFY(viewport->property("atYEnd").toBool());
        const auto following = latest + "Продолжение без лишнего переноса";
        put(file, following.toUtf8());
        QTRY_COMPARE(field->property("text").toString(), following);
        QTRY_VERIFY(viewport->property("atYEnd").toBool());
        viewport->setProperty("contentY", 100.0);
        const auto large = QString("Длинная строка лекции 😀 с текстом для чтения\n").repeated(14000);
        QVERIFY(large.toUtf8().size() > qsizetype{512} * 1024);
        put(file, large.toUtf8());
        QTRY_VERIFY(!backend.transcriptNotice().isEmpty());
        QTRY_VERIFY(backend.transcript().startsWith(QStringLiteral("Длинная")));
        QCOMPARE(field->property("text").toString(), following);
        QVERIFY(QMetaObject::invokeMethod(resume, "clicked"));
        QTRY_COMPARE(field->property("text").toString(), backend.transcript());
        QTRY_VERIFY(viewport->property("atYEnd").toBool());
        viewport->setProperty("contentY", 100.0);
        put(file, (large + "Последняя строка\n").toUtf8());
        QTRY_VERIFY(backend.transcript().endsWith(QStringLiteral("Последняя строка\n")));
        viewport->setProperty("contentY", viewport->property("contentHeight").toDouble() - viewport->property("height").toDouble());
        QTRY_COMPARE(field->property("text").toString(), backend.transcript());
        const auto history = temp.path() + "/history";
        put(history + "/transcripts/transcript.txt", "Другой результат\n");
        backend.viewResult(history);
        QTRY_COMPARE(field->property("text").toString(), QString("Другой результат\n"));
        QVERIFY(field->property("selectedText").toString().isEmpty());
        QVERIFY(!resume->property("visible").toBool());
        backend.cancel();
        QTRY_VERIFY_WITH_TIMEOUT(!backend.busy(), 3000);
        QVERIFY2(warnings.isEmpty(), qPrintable(warnings.join('\n')));
    }
};
int main(int argc, char** argv) {
    QQuickStyle::setStyle("Material");
    QGuiApplication app(argc, argv);
    TranscriptViewTests test;
    return QTest::qExec(&test, argc, argv);
}
#include "test_transcript_view.moc"
