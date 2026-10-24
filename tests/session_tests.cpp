#include "domain/session.h"
#include "domain/countdown.h"
#include <iostream>
#include <stdexcept>
#include <string>
using namespace gestart;
#define CHECK(x) do { if (!(x)) throw std::runtime_error("Failed at line " + std::to_string(__LINE__) + ": " #x); } while(false)
int main() {
  try {
    Session s;
    s.begin({1000, 2, false, 0});
    CHECK(s.state() == State::Loading && s.completed() == 0);
    CHECK(s.tick(100000) == Action::None);
    s.presented(100000);
    CHECK(s.remaining(100500) == 500);
    CHECK(s.tick(103500) == Action::NextImage); // delayed callbacks use actual time
    CHECK(s.completed() == 1 && s.state() == State::Loading);
    s.presented(104000);
    s.togglePause(104200);
    s.pauseForSettings(104300);
    s.resumeFromSettings(999999);
    CHECK(s.state() == State::Paused && s.remaining(999999) == 800);
    s.togglePause(1000000);
    CHECK(s.remaining(1000100) == 700);
    s.enterReview(1000200);
    CHECK(s.state() == State::Review && s.completed() == 1);
    s.pauseForSettings(1000300);
    s.resumeFromSettings(1000400);
    CHECK(s.state() == State::Review);
    s.leaveReview(1000500);
    CHECK(s.remaining(1000600) == 500);
    s.tick(1002000);
    CHECK(s.state() == State::Completed && s.completed() == 2);
    s.begin({1000, 1, true, 2000});
    s.presented(0); s.tick(1000);
    CHECK(s.state() == State::Break && s.ordinal() == 1);
    s.pauseForSettings(1500); s.resumeFromSettings(4000);
    CHECK(s.remaining(4100) == 1400);
    CHECK(s.tick(5500) == Action::Restart);
    CHECK(s.state() == State::Loading && s.completed() == 0);
    s.begin({1000,1,true,2000});s.presented(0);s.tick(1000);
    s.togglePause(1300);CHECK(s.state()==State::Break && s.paused() && s.remaining(5000)==1700);
    CHECK(s.tick(5000)==Action::None);s.pauseForSettings(5000);s.resumeFromSettings(6000);
    s.pauseForNavigation(6000);s.resumeFromNavigation(7000);CHECK(s.remaining(9000)==1700);
    s.togglePause(10000);CHECK(!s.paused() && s.remaining(10100)==1600);
    CHECK(s.tick(11700)==Action::Restart);
    s.begin({0, 0, false, 0}); s.presented(0);
    CHECK(s.tick(1'000'000) == Action::None);
    CHECK(s.advance(1'000'000, false) == Action::NextImage && s.completed() == 0);
    s.presented(1'000'001);
    s.advance(1'000'002); CHECK(s.completed() == 1);
    s.begin({1000, 1, true, 0}); s.presented(0); s.tick(1000);
    CHECK(s.state() == State::Break && s.tick(99999) == Action::None);
    CHECK(s.skipBreak() == Action::Restart);
    s.begin({1000, 10, false, 0}); s.togglePause(0); s.presented(100);
    CHECK(s.state() == State::Paused);
    s.togglePause(200); s.pauseAfterSleep(300);
    CHECK(s.state() == State::Paused && s.remaining(99999) == 900);
    s.stop(); CHECK(s.state() == State::Idle && s.completed() == 0);
    s.begin({0, 0, false, 0}); s.presented(100);
    CHECK(s.activeElapsed(300) == 200);
    s.togglePause(300); CHECK(s.activeElapsed(10000) == 200);
    s.togglePause(10000); s.pauseForSettings(10100);
    CHECK(s.activeElapsed(20000) == 300);
    s.resumeFromSettings(20000); s.enterReview(20100);
    CHECK(s.activeElapsed(90000) == 400);
    s.leaveReview(90000); CHECK(s.activeElapsed(90100) == 500);
    s.advance(90100); s.presented(100000); CHECK(s.activeElapsed(100100) == 100);
    s.begin({1000, 2, false, 0}); s.presented(0);
    CHECK(s.activeElapsed(100000) == 1000); // a delayed callback cannot inflate practice time
    s.begin({1000,2,false,0});s.presented(0);s.pauseForNavigation(200);
    CHECK(s.tick(10000)==Action::None && s.activeElapsed(10000)==200);
    CHECK(s.advance(10000)==Action::None && s.completed()==0);
    s.pauseForSettings(10000);s.resumeFromSettings(11000);CHECK(s.state()==State::Paused);
    s.resumeFromNavigation(12000);CHECK(s.state()==State::Running && s.remaining(12100)==700);
    s.togglePause(12200);s.pauseForNavigation(12300);s.resumeFromNavigation(15000);
    CHECK(s.state()==State::Paused);
    s.begin({1000,2,false,0});s.stop();s.presented(10000);CHECK(s.state()==State::Idle && s.completed()==0);
    Countdown cues;
    s.begin({1000,1,true,2000});s.presented(0);s.tick(1000);s.enterReview(1500);
    CHECK(s.state()==State::Review && s.remaining(10000)==1500);
    CHECK(s.tick(10000)==Action::None);s.leaveReview(10000);
    CHECK(s.state()==State::Break && s.remaining(10100)==1400 && s.completed()==1);
    cues.reset(30000);CHECK(cues.poll(3001)==-1 && cues.nextDelay(3001)==1);
    CHECK(cues.poll(3000)==3 && cues.poll(3000)==-1);
    CHECK(cues.poll(2000)==2 && cues.poll(1000)==1 && cues.poll(0)==0 && cues.poll(0)==-1);
    cues.reset(4000);CHECK(cues.poll(2990)==3);
    s.begin({4000,1,false,0});s.presented(0);s.togglePause(1100);
    CHECK(cues.poll(s.remaining(20000))==-1);s.togglePause(20000);
    CHECK(cues.nextDelay(s.remaining(20000))==900);
    CHECK(cues.poll(s.remaining(20900))==2 && cues.poll(s.remaining(21900))==1 && cues.poll(s.remaining(22900))==0);
    cues.reset(30000);CHECK(cues.poll(500)==-1 && cues.poll(0)==0); // no stale warning burst
    cues.reset(1000);CHECK(cues.poll(1000)==1 && cues.poll(0)==0);
    cues.reset(2000);CHECK(cues.poll(2000)==2 && cues.poll(1000)==1 && cues.poll(0)==0);
    cues.reset(0);CHECK(cues.poll(0)==-1 && cues.nextDelay(0)==-1);
    cues.reset(4000);CHECK(cues.poll(3000)==3);cues.reset(4000);CHECK(cues.poll(3000)==3);
    std::cout << "Session and countdown rules: loading, monotonic time, pause, review, limits, break, sleep, 3-2-1-end cues passed.\n";
    return 0;
  } catch (const std::exception& e) { std::cerr << e.what() << '\n'; return 1; }
}
