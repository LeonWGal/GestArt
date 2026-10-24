#pragma once
#include <cstdint>
#include <algorithm>

namespace gestart {
enum class State { Idle, Loading, Running, Paused, Review, Break, Completed };
enum class Action { None, NextImage, Restart };
struct SessionSettings {
    std::int64_t durationMs = 30'000;
    int imageLimit = 10;
    bool breakEnabled = false;
    std::int64_t breakMs = 300'000;
};

// Times are injected monotonic milliseconds. No UI, filesystem, or timer dependency.
class Session {
public:
    void begin(SessionSettings settings);
    void requestImage();
    void presented(std::int64_t now);
    Action advance(std::int64_t now, bool count = true);
    Action tick(std::int64_t now);
    void togglePause(std::int64_t now);
    void pauseForSettings(std::int64_t now);
    void resumeFromSettings(std::int64_t now);
    void pauseForNavigation(std::int64_t now);
    void resumeFromNavigation(std::int64_t now);
    void enterReview(std::int64_t now);
    void leaveReview(std::int64_t now);
    void stop();
    void pauseAfterSleep(std::int64_t beforeSleep);
    Action skipBreak();
    [[nodiscard]] std::int64_t remaining(std::int64_t now) const;
    [[nodiscard]] std::int64_t activeElapsed(std::int64_t now) const;
    [[nodiscard]] bool paused()const{return state_==State::Paused || (state_==State::Break && manualPause_);}
    [[nodiscard]] State state() const { return state_; }
    [[nodiscard]] int completed() const { return completed_; }
    [[nodiscard]] int ordinal() const { return completed_ + (state_ == State::Completed || state_ == State::Break ? 0 : 1); }
    [[nodiscard]] const SessionSettings& settings() const { return settings_; }
    [[nodiscard]] bool settingsOpen() const { return settingsOpen_; }
private:
    void resumeClock(std::int64_t now);
    void freezeActiveTime(std::int64_t now);
    SessionSettings settings_;
    State state_ = State::Idle;
    State beforeReview_ = State::Idle;
    State beforeSettings_ = State::Idle;
    State beforeNavigation_ = State::Idle;
    std::int64_t deadline_ = 0;
    std::int64_t remaining_ = 0;
    std::int64_t activeElapsed_ = 0;
    std::int64_t activeStarted_ = -1;
    int completed_ = 0;
    bool manualPause_ = false;
    bool settingsOpen_ = false;
    bool navigationOpen_ = false;
};
}
