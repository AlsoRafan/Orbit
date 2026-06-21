#include <iostream>
#include "include/stopwatch.h"
#include "include/countdown.h"
#include "include/alarm.h"
using namespace std;

int main() {
    int choice;

    cout << "\n"
 
              << "  1. Stopwatch\n"
              << "  2. Countdown\n"
              << "  3. Alarm\n"
              << "  Enter choice: ";

    cin >> choice;

    switch (choice) {
        case 1: stopwatch(); break;
        case 2: countdown(); break;
        case 3: Alarm(); break;
        default: cout << "  Invalid choice.\n"; break;
    }

    return 0;
}
