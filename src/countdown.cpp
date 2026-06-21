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
    Timer(int t = 0) : time_sec(t) {}
    void  set_time();
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
    cin >> choice;
    cout << "Enter time: ";
    cin >> t;

    switch (choice) {
        case 1: time_sec = t;        break;
        case 2: time_sec = t * 60;   break;
        case 3: time_sec = t * 3600; break;
        default:
            cout << "Invalid choice.\n";
            time_sec = 0;
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
    c.set_time();
    c.start();
    return 0;
}