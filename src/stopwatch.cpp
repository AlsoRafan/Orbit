#include "../include/stopwatch.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <atomic>
#include <iomanip>
#include <sstream>
#include <string>
#include <conio.h>
#include <windows.h>
using namespace std;

enum class State
{
    stopped,
    running,
    paused
};

struct Lap
{
    int index;
    long long split_ms;
    long long total_ms;
};

class Stopwatch
{
private:
    State cur_state{State::stopped};
    long long saved_ms{0};
    chrono::steady_clock::time_point start_tp;
    vector<Lap> lap_list;

    long long live_ms() const
    {
        return chrono::duration_cast<chrono::milliseconds>(
                   chrono::steady_clock::now() - start_tp)
            .count();
    }

public:
    void start()
    {
        saved_ms = 0;
        lap_list.clear();
        start_tp = chrono::steady_clock::now();
        cur_state = State::running;
    }
    void pause()
    {
        if (cur_state != State::running)
            return;
        saved_ms += live_ms();
        cur_state = State::paused;
    }
    void resume()
    {
        if (cur_state != State::paused)
            return;
        start_tp = chrono::steady_clock::now();
        cur_state = State::running;
    }
    bool lap()
    {
        if (cur_state != State::running)
            return false;
        long long total = saved_ms + live_ms();
        long long prev = lap_list.empty() ? 0 : lap_list.back().total_ms;
        lap_list.push_back({(int)lap_list.size() + 1, total - prev, total});
        return true;
    }
    void reset()
    {
        cur_state = State::stopped;
        saved_ms = 0;
        lap_list.clear();
    }

    State state() const { return cur_state; }
    long long elapsed() const { return saved_ms + (cur_state == State::running ? live_ms() : 0LL); }
    const vector<Lap> &laps() const { return lap_list; }
    string format_current() const { return format_time(elapsed()); }

    static string format_time(long long ms)
    {
        ostringstream s;
        s << setfill('0')
          << setw(2) << ms / 3600000 << ":"
          << setw(2) << ms % 3600000 / 60000 << ":"
          << setw(2) << ms % 60000 / 1000 << "."
          << setw(3) << ms % 1000;
        return s.str();
    }
    static const char *state_label(State s)
    {
        switch (s)
        {
        case State::running:
            return "[running]";
        case State::paused:
            return "[paused] ";
        default:
            return "[stopped]";
        }
    }
};

class App
{
public:
    void run()
    {
        cout << "\n"
             << "  [s] Start   [p] Pause   [r] Resume\n"
             << "  [l] Lap     [x] Reset   [q] Quit\n\n"
             << "  00:00:00.000" << flush;

        thread disp(&App::display_loop, this);
        while (!quit)
            handle(_getch());
        disp.join();

        cout << "\n\n  Final time : " << sw.format_current() << "\n";
        if (!sw.laps().empty())
        {
            cout << "  Laps:\n";
            for (const auto &l : sw.laps())
                cout << "    Lap " << setfill('0') << setw(2) << l.index
                     << "  total: " << Stopwatch::format_time(l.total_ms)
                     << "  split: " << Stopwatch::format_time(l.split_ms) << "\n";
        }
        cout << "\n";
    }

private:
    Stopwatch sw;
    atomic<bool> quit{false};
    atomic<bool> laps_dirty{false};

    void display_loop()
    {
        int shown_laps = 0;
        while (!quit)
        {
            int cur_laps = (int)sw.laps().size();
            if (laps_dirty.exchange(false) && cur_laps > shown_laps)
            {
                for (int i = shown_laps; i < cur_laps; ++i)
                {
                    const Lap &l = sw.laps()[i];
                    cout << "\n  Lap " << setfill('0') << setw(2) << l.index
                         << "  total: " << Stopwatch::format_time(l.total_ms)
                         << "  split: " << Stopwatch::format_time(l.split_ms);
                }
                cout << "\n";
                shown_laps = cur_laps;
            }
            cout << "\r\033[2K  " << sw.format_current() << "  "
                 << Stopwatch::state_label(sw.state()) << flush;
            this_thread::sleep_for(chrono::milliseconds(10));
        }
    }

    void handle(char c)
    {
        switch (c | 32)
        {
        case 's':
            sw.start();
            laps_dirty = false;
            break;
        case 'p':
            sw.pause();
            break;
        case 'r':
            sw.resume();
            break;
        case 'l':
            if (sw.lap())
                laps_dirty = true;
            break;
        case 'x':
            sw.reset();
            laps_dirty = false;
            break;
        case 'q':
            quit = true;
            break;
        default:
            break;
        }
    }
};

void stopwatch()
{
    App app;
    app.run();
}