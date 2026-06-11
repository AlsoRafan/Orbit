#include <iostream>
#include "include/stopwatch.h"
#include "include/countdown.h"

using namespace std;

int main() {
    int choice;

    cout << "\n"
 
              << "  1. Stopwatch\n"
              << "  2. Countdown\n"
              << "  Enter choice: ";

    cin >> choice;

    switch (choice) {
        case 1: stopwatch(); break;
        case 2: countdown(); break;
        default: std::cout << "  Invalid choice.\n"; break;
    }

    return 0;
}