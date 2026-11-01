#pragma once
#include "text.hpp"
#include <QObject>
#include <QProcess>
#include <QUrl>
#include <functional>
#include <map>
#include <memory>

class DesktopOpener : public QObject {
public:
    using Completion = std::function<void(const UiText&)>;
    explicit DesktopOpener(QString helper, QString launcher = "/usr/bin/xdg-open");
    ~DesktopOpener() override;
    void open(int channel, const QUrl& url, Completion completion);
    void reset(int channel);

private:
    struct Job {
        QProcess process;
        Completion completion;
        bool done = false;
        quint64 serial = 0;
    };
    using Key = std::pair<int, QString>;
    std::map<Key, std::unique_ptr<Job>> jobs_;
    QString helper_, launcher_;
    quint64 serial_ = 0;
    void finish(const Key& key, Job* job, const UiText& error);
};
