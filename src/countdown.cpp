#include "../include/countdown.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>
using namespace std;

class Timer {
protected:
    int time_sec;
public:
    class invalid_choice_err  {};
    class invalid_time_err    {};
    class non_numeric_err     {};

    Timer(int t = 0) : time_sec(t) {}
    void set_time();
    virtual void start() = 0;
    virtual ~Timer() = default;
};

class CountDown : public Timer {
public:
    CountDown(int t = 0) : Timer(t) {}
    void display() const;
    void start() override;
};

void Timer::set_time() {
    int choice, t;

    cout << "\nChoose time unit:\n"
         << "  1. Seconds\n"
         << "  2. Minutes\n"
         << "  3. Hours\n"
         << "Enter choice: ";

    if (!(cin >> choice)) {
        cin.clear();
        cin.ignore(1000, '\n');
        throw non_numeric_err{};
    }

    if (choice < 1 || choice > 3)
        throw invalid_choice_err{};

    cout << "Enter time: ";

    if (!(cin >> t)) {
        cin.clear();
        cin.ignore(1000, '\n');
        throw non_numeric_err{};
    }

    if (t <= 0) throw invalid_time_err{};

    switch (choice) {
        case 1: time_sec = t;        break;
        case 2: time_sec = t * 60;   break;
        case 3: time_sec = t * 3600; break;
    }
}

void CountDown::display() const {
    int h = time_sec / 3600;
    int m = time_sec % 3600 / 60;
    int s = time_sec % 60;
    cout << setfill('0')
         << setw(2) << h << ":"
         << setw(2) << m << ":"
         << setw(2) << s << "\r" << flush;
}

void CountDown::start() {
    while (time_sec > 0) {
        display();
        this_thread::sleep_for(chrono::seconds(1));
        time_sec--;
    }
    cout << "\nTime's up!\n";
}

int countdown() {
    CountDown c;

    while (true) {
        try {
            c.set_time();
            break;

        } catch (Timer::non_numeric_err) {
            cout << "[Error] Please enter numbers only. Try again.\n";

        } catch (Timer::invalid_choice_err) {
            cout << "[Error] Choice must be 1, 2, or 3. Try again.\n";

        } catch (Timer::invalid_time_err) {
            cout << "[Error] Time must be greater than 0. Try again.\n";
        }
    }

    c.start();
    return 0;
}
