#include "desktop.hpp"
#include "portable.hpp"
#include <QDesktopServices>
#include <QTimer>

DesktopOpener::DesktopOpener(QString helper, QString launcher)
    : helper_(std::move(helper)), launcher_(std::move(launcher)) {}
DesktopOpener::~DesktopOpener() {
    reset(0);
    reset(1);
}
void DesktopOpener::reset(int channel) {
    for (auto it = jobs_.begin(); it != jobs_.end();) {
        if (it->first.first != channel) {
            ++it;
            continue;
        }
        it->second->process.disconnect(this);
        // Only stop the monitor; xdg-open may have exec'd a long-lived editor.
        it->second->process.kill();
        it = jobs_.erase(it);
    }
}
void DesktopOpener::finish(const Key& key, Job* job, const UiText& error) {
    if (job->done) return;
    job->done = true;
    const auto serial = job->serial;
    QTimer::singleShot(0, this, [this, key, serial, error] {
        const auto found = jobs_.find(key);
        if (found == jobs_.end() || found->second->serial != serial) return;
        const auto completion = found->second->completion;
        found->second->process.disconnect(this);
        found->second->process.kill();
        jobs_.erase(found);
        completion(error);
    });
}
void DesktopOpener::open(int channel, const QUrl& url, Completion completion) {
    const auto failure = UiText::translated("DesktopOpener", QT_TRANSLATE_NOOP("DesktopOpener", "Не удалось открыть %1")).arg(url.toLocalFile());
    bool native = false;
    try {
        native = portable::root().isEmpty();
        // Validate before starting a helper; the helper repeats this check.
        if (!native) (void)portable::desktopEnvironment(QProcessEnvironment::systemEnvironment());
    } catch (const std::exception&) {
        completion(failure);
        return;
    }
    if (native) {
        completion(QDesktopServices::openUrl(url) ? UiText{} : failure);
        return;
    }
    const Key key{channel, url.toString(QUrl::FullyEncoded)};
    const auto found = jobs_.find(key);
    if (found != jobs_.end() && !found->second->done) {
        found->second->completion = std::move(completion);
        return;
    }
    if (found != jobs_.end()) {
        found->second->process.disconnect(this);
        found->second->process.kill();
        jobs_.erase(found);
    }
    auto owned = std::make_unique<Job>();
    auto* job = owned.get();
    job->completion = completion;
    job->serial = ++serial_;
    jobs_.emplace(key, std::move(owned));
    connect(&job->process, &QProcess::errorOccurred, this, [this, key, job, failure](QProcess::ProcessError) {
        finish(key, job, failure + UiText(": ") + UiText(job->process.errorString()));
    });
    connect(&job->process, &QProcess::finished, this, [this, key, job, failure](int code, QProcess::ExitStatus status) {
        if (code == 0 && status == QProcess::NormalExit) finish(key, job, {});
        else finish(key, job, failure + UiText(": ") + UiText::translated("DesktopOpener", QT_TRANSLATE_NOOP("DesktopOpener", "обработчик завершился с кодом %1")).arg(code));
    });
    job->process.setProgram(helper_);
    job->process.setArguments({launcher_, key.second});
    job->process.setProcessEnvironment(QProcessEnvironment::systemEnvironment());
    job->process.setStandardInputFile(QProcess::nullDevice());
    job->process.setStandardOutputFile(QProcess::nullDevice());
    job->process.setStandardErrorFile(QProcess::nullDevice());
    job->process.start();
}
