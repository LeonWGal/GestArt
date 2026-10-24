#pragma once
#include <algorithm>
#include <array>
#include <cstdint>

namespace gestart {
// Every beat follows the session's remaining time, including after a pause.
// A stalled event loop skips obsolete warning beats instead of playing a burst.
class Countdown {
public:
    void reset(std::int64_t duration) {
        index_ = 0;
        while (index_ < beats_.size() && beats_[index_] * 1000 > duration) ++index_;
        if (duration <= 0) index_ = beats_.size();
    }
    int poll(std::int64_t remaining) {
        int beat = -1;
        while (index_ < beats_.size() && remaining <= beats_[index_] * 1000) {
            const int crossed = beats_[index_++];
            beat = crossed == 0 || crossed * 1000 - remaining <= 150 ? crossed : -1;
        }
        return beat;
    }
    std::int64_t nextDelay(std::int64_t remaining) const {
        return index_ < beats_.size() ? std::max<std::int64_t>(1, remaining - beats_[index_] * 1000) : -1;
    }
private:
    static constexpr std::array<int, 4> beats_{3, 2, 1, 0};
    std::size_t index_ = beats_.size();
};
}
