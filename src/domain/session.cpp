#include "session.h"
namespace gestart {
void Session::begin(SessionSettings settings) {
    settings.durationMs = std::clamp<std::int64_t>(settings.durationMs, 0, 86'400'000);
    settings.imageLimit = std::clamp(settings.imageLimit, 0, 100'000);
    settings.breakMs = std::clamp<std::int64_t>(settings.breakMs, 0, 86'400'000);
    settings_ = settings;
    completed_ = 0;
    manualPause_ = false;
    state_ = State::Loading;
    remaining_ = settings_.durationMs;
    activeElapsed_ = 0;
    activeStarted_ = -1;
}
void Session::requestImage() {
    if (state_ == State::Review) state_ = beforeReview_;
    state_ = State::Loading;
    remaining_ = settings_.durationMs;
    activeElapsed_ = 0;
    activeStarted_ = -1;
}
void Session::presented(std::int64_t now) {
    if (state_ != State::Loading) return;
    remaining_ = settings_.durationMs;
    state_ = manualPause_ || settingsOpen_ || navigationOpen_ ? State::Paused : State::Running;
    resumeClock(now);
}
std::int64_t Session::remaining(std::int64_t now) const {
    if (state_ == State::Running && settings_.durationMs == 0) return 0;
    if ((state_ == State::Running || state_ == State::Break) && !settingsOpen_ && !navigationOpen_ && !manualPause_)
        return std::max<std::int64_t>(0, deadline_ - now);
    return remaining_;
}
std::int64_t Session::activeElapsed(std::int64_t now) const {
    const auto elapsed = activeElapsed_ + (activeStarted_ >= 0 ? std::max<std::int64_t>(0, now - activeStarted_) : 0);
    return settings_.durationMs > 0 ? std::min(elapsed, settings_.durationMs) : elapsed;
}
void Session::freezeActiveTime(std::int64_t now) {
    activeElapsed_ = activeElapsed(now);
    activeStarted_ = -1;
}
void Session::resumeClock(std::int64_t now) {
    deadline_ = now + remaining_;
    if (state_ == State::Running && !settingsOpen_ && !navigationOpen_) activeStarted_ = now;
}
Action Session::advance(std::int64_t now, bool count) {
    if(settingsOpen_ || navigationOpen_)return Action::None;
    if (state_ != State::Running && state_ != State::Paused) return Action::None;
    freezeActiveTime(now);
    if (count) ++completed_;
    if (settings_.imageLimit > 0 && completed_ >= settings_.imageLimit) {
        if (settings_.breakEnabled) {
            state_ = State::Break;
            remaining_ = settings_.breakMs;
            resumeClock(now);
        } else {
            state_ = State::Completed;
            remaining_ = 0;
        }
        return Action::None;
    }
    requestImage();
    return Action::NextImage;
}
Action Session::tick(std::int64_t now) {
    if (settingsOpen_ || navigationOpen_ || manualPause_) return Action::None;
    if (state_ == State::Running && settings_.durationMs > 0 && now >= deadline_)
        return advance(now);
    if (state_ == State::Break && settings_.breakMs > 0 && now >= deadline_)
        return skipBreak();
    return Action::None;
}
void Session::togglePause(std::int64_t now) {
    if (settingsOpen_ || navigationOpen_) return;
    if(state_==State::Break){
        if(!manualPause_){remaining_=remaining(now);manualPause_=true;}
        else{manualPause_=false;resumeClock(now);}
        return;
    }
    if (state_ == State::Running) {
        remaining_ = remaining(now);
        freezeActiveTime(now);
        manualPause_ = true;
        state_ = State::Paused;
    } else if (state_ == State::Paused) {
        manualPause_ = false;
        state_ = State::Running;
        resumeClock(now);
    } else if (state_ == State::Loading) manualPause_ = !manualPause_;
}
void Session::pauseForSettings(std::int64_t now) {
    if (settingsOpen_) return;
    remaining_ = remaining(now);
    freezeActiveTime(now);
    beforeSettings_ = state_;
    settingsOpen_ = true;
    if (state_ == State::Running) state_ = State::Paused;
}
void Session::resumeFromSettings(std::int64_t now) {
    if (!settingsOpen_) return;
    settingsOpen_ = false;
    if ((beforeSettings_ == State::Running || beforeSettings_ == State::Loading) && state_ == State::Paused && !manualPause_ && !navigationOpen_)
        state_ = State::Running;
    resumeClock(now);
}
void Session::pauseForNavigation(std::int64_t now) {
    if(navigationOpen_)return;
    remaining_=remaining(now);freezeActiveTime(now);beforeNavigation_=state_;navigationOpen_=true;
    if(state_==State::Running)state_=State::Paused;
}
void Session::resumeFromNavigation(std::int64_t now) {
    if(!navigationOpen_)return;
    navigationOpen_=false;
    if((beforeNavigation_==State::Running || beforeNavigation_==State::Loading) && state_==State::Paused && !manualPause_ && !settingsOpen_)state_=State::Running;
    resumeClock(now);
}
void Session::enterReview(std::int64_t now) {
    if (state_ != State::Running && state_ != State::Paused && state_ != State::Break) return;
    remaining_ = remaining(now);
    freezeActiveTime(now);
    beforeReview_ = state_;
    state_ = State::Review;
}
void Session::leaveReview(std::int64_t now) {
    if (state_ != State::Review) return;
    state_ = beforeReview_;
    resumeClock(now);
}
void Session::stop() {
    state_ = State::Idle;
    completed_ = 0;
    manualPause_ = false;
    remaining_ = 0;
    activeElapsed_ = 0;
    activeStarted_ = -1;
    navigationOpen_ = false;
}
void Session::pauseAfterSleep(std::int64_t beforeSleep) {
    if(state_==State::Break){remaining_=remaining(beforeSleep);manualPause_=true;return;}
    if (state_ == State::Running) {
        remaining_ = remaining(beforeSleep);
        freezeActiveTime(beforeSleep);
        manualPause_ = true;
        state_ = State::Paused;
    }
}
Action Session::skipBreak() {
    if (state_ != State::Break) return Action::None;
    begin(settings_);
    return Action::Restart;
}
}
