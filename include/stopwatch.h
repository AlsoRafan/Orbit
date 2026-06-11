#pragma once

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <mutex>
#include <iomanip>
#include <sstream>
#include <string>

#ifdef _WIN32
  #include <conio.h>
  #include <windows.h>
#else
  #include <termios.h>
  #include <unistd.h>
#endif



enum class State { stopped, running, paused };

struct Lap {
    int       index;
    long long split_ms;
    long long total_ms;
};


class Terminal {
public:
    static void     init();
    static void     clear_line();
    static char     get_key();
    static void     enable_raw();
    static void     disable_raw();
private:
#ifndef _WIN32
    static termios  saved_term;
#endif
};


class Stopwatch {
public:
    void        start();
    void        pause();
    void        resume();
    bool        lap();
    void        reset();

    State                      state()   const;
    long long                  elapsed() const;
    const std::vector<Lap>&    laps()    const;
    std::string                format_current() const;

    static std::string  format_time(long long ms);
    static const char*  state_label(State s);

private:
    mutable std::mutex  mtx;
    State               cur_state { State::stopped };
    long long           saved_ms  { 0 };
    std::chrono::steady_clock::time_point start_tp;
    std::vector<Lap>    lap_list;

    long long live_ms() const;
};

class App {
public:
    void run();
private:
    Stopwatch           sw;
    std::atomic<bool>   quit      { false };
    std::atomic<bool>   laps_dirty{ false };

    void display_loop();
    void handle(char c);
};

void stopwatch();