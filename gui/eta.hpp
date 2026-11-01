#pragma once
#include "text.hpp"
#include <chrono>
#include <optional>

class EtaPresentation {
public:
    using Clock = std::chrono::steady_clock;
    void observe(double fraction, std::optional<double> seconds, Clock::time_point now);
    bool refresh(Clock::time_point now);
    void reset() { *this = EtaPresentation{}; }
    QString text() const { return text_; }

private:
    double fraction_ = -1;
    std::optional<double> seconds_;
    Clock::time_point lastAdvance_{}, observed_{};
    std::optional<Clock::time_point> rendered_, resumed_;
    UiText text_;
};
