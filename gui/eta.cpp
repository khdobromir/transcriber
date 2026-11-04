#include "eta.hpp"
#include <algorithm>
#include <cmath>

void EtaPresentation::observe(double fraction, std::optional<double> seconds, Clock::time_point now) {
    if (!std::isfinite(fraction) || fraction < 0 || fraction > 1) return;
    if (fraction_ < 0 || fraction > fraction_) {
        if (fraction_ >= 0 && now - lastAdvance_ >= std::chrono::seconds(30)) resumed_ = now;
        lastAdvance_ = now;
        fraction_ = fraction;
    }
    seconds_ = seconds;
    observed_ = now;
    refresh(now);
}

bool EtaPresentation::refresh(Clock::time_point now) {
    if (fraction_ < 0 || (rendered_ && now - *rendered_ < std::chrono::seconds(5))) return false;
    rendered_ = now;
    const auto before = text_.text();
    if (now - lastAdvance_ >= std::chrono::seconds(30)) {
        text_ = UiText::translated("Backend", QT_TRANSLATE_NOOP("Backend", "Уточняем оставшееся время…"));
    } else if (!seconds_ || !std::isfinite(*seconds_) || *seconds_ < 0 ||
               (resumed_ && observed_ - *resumed_ < std::chrono::seconds(5))) {
        text_ = UiText::translated("Backend", QT_TRANSLATE_NOOP("Backend", "Оценка времени рассчитывается…"));
    } else {
        const auto seconds = std::clamp(*seconds_, 0.0, 359999.0);
        if (seconds >= 60) {
            const auto minutes = static_cast<qint64>(std::ceil(seconds / 60));
            if (minutes >= 60)
                text_ = UiText::translated("Backend", QT_TRANSLATE_NOOP("Backend", "Осталось примерно %1 ч %2 мин")).arg(minutes / 60).arg(minutes % 60);
            else text_ = UiText::translated("Backend", QT_TRANSLATE_NOOP("Backend", "Осталось примерно %1 мин")).arg(minutes);
        } else {
            const auto rounded = std::max(5, static_cast<int>(std::ceil(seconds / 5)) * 5);
            text_ = UiText::translated("Backend", QT_TRANSLATE_NOOP("Backend", "Осталось примерно %1 с")).arg(rounded);
        }
    }
    return text_.text() != before;
}
