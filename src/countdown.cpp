#include "../include/countdown.h"

void Timer::set_time() {
    int choice, t;
    std::cout << "\nChoose time unit:\n"
              << "  1. Seconds\n"
              << "  2. Minutes\n"
              << "  3. Hours\n"
              << "Enter choice: ";
    std::cin >> choice;
    std::cout << "Enter time: ";
    std::cin >> t;

    switch (choice) {
        case 1: time_sec = t;        break;
        case 2: time_sec = t * 60;   break;
        case 3: time_sec = t * 3600; break;
        default:
            std::cout << "Invalid choice.\n";
            time_sec = 0;
    }
}

void CountDown::display() const {
    int h = time_sec / 3600;
    int m = time_sec % 3600 / 60;
    int s = time_sec % 60;
    std::cout << std::setfill('0')
              << std::setw(2) << h << ":"
              << std::setw(2) << m << ":"
              << std::setw(2) << s << "\r" << std::flush;
}

void CountDown::start() {
    while (time_sec > 0) {
        display();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        time_sec--;
    }
    std::cout << "\nTime's up!\n";
}

int countdown() {
    CountDown c;
    c.set_time();
    c.start();
    return 0;
}