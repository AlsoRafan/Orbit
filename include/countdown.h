#pragma once

#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

class Timer {
protected:
    int time_sec;
public:
    Timer(int t = 0) : time_sec(t) {}
    void        set_time();
    virtual void start() = 0;
    virtual ~Timer() = default;
};

class CountDown : public Timer {
public:
    CountDown(int t = 0) : Timer(t) {}
    void display() const;
    void start() override;
};

int countdown();