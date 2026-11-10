#include "process.hpp"
#include <csignal>
#include <thread>
#include <utility>

namespace transcribe {
namespace {
#ifdef _WIN32
constexpr int terminate_signal = 15, kill_signal = 9;
#else
constexpr int terminate_signal = SIGTERM, kill_signal = SIGKILL;
#endif
} // namespace
void stop_all(std::vector<std::unique_ptr<Process>>& processes, int signal) {
    for (auto& process : processes)
        if (process && !process->done()) process->request_stop(signal);
    std::vector<bool> usable(processes.size(), true);
    bool waiting;
    do {
        waiting = false;
        for (size_t i = 0; i < processes.size(); ++i)
            if (auto& process = processes[i]; process && !process->done()) {
                // Preserve callbacks (and already received text) when possible.
                try {
                    process->tick(usable[i]);
                } catch (...) {
                    usable[i] = false;
                    process->request_stop(kill_signal);
                }
                waiting = true;
            }
        if (waiting) std::this_thread::sleep_for(std::chrono::milliseconds(10));
    } while (waiting);
}
void run(const std::vector<std::string>& args, const std::filesystem::path& log,
         Output output, Output errors, std::chrono::seconds timeout) {
    std::vector<std::unique_ptr<Process>> processes;
    processes.push_back(std::make_unique<Process>(args, log, std::move(output), std::move(errors)));
    const auto start = std::chrono::steady_clock::now();
    try {
        while (!processes[0]->done()) {
            check_cancelled();
            if (timeout.count() && std::chrono::steady_clock::now() - start >= timeout)
                throw ProcessError(transcribe::Message{"Превышено время ожидания " + args[0], "Timed out waiting for " + args[0]}, 1);
            processes[0]->tick();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        check_cancelled();
        processes[0]->require_success();
    } catch (...) {
        stop_all(processes, cancellation_signal() ? cancellation_signal() : terminate_signal);
        throw;
    }
}
} // namespace transcribe
