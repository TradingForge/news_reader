#pragma once

#include <bits/stdc++.h>

class Stopwatch final {

private:

    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;

    Stopwatch() {
        _start = Clock::now();
        _stopped = false;
    }

    bool _stopped;
    TimePoint _start;
    TimePoint _stop;

public:

    static Stopwatch Start() {
        return {};
    }

    void Stop() {
        assert(!_stopped);
        _stop = Clock::now();
        _stopped = true;
    }

    long long Elapsed() {
        assert(_stopped);
        return std::chrono::duration_cast<std::chrono::microseconds>(_stop - _start).count();
    }

};
