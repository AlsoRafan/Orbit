#include "../include/alarm.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <ctime>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <climits>
using namespace std;

struct alarm_info {
    string time;
    string type;
    int day   = 0;
    int month = 0;
    bool is_true = false;
};
vector<alarm_info> all_alarms;

class file_save {
public:
    class file_open_err {};

    virtual void save_data(string alarm_data) {
        ofstream file("data/alarm.txt", ios::app);
        if (!file.is_open()) throw file_open_err{};
        file << alarm_data << endl;
        file.close();
    }
};

class data_fetch_from_file {
public:
    class file_read_err {};

    void fetch_data_from_file() {
        ifstream file("data/alarm.txt");
        if (!file.is_open()) throw file_read_err{};
        string line;
        while (getline(file, line)) cout << line << endl;
    }
};

class time_input : virtual public file_save {
private:
    string lower_case_string;
protected:
    string hours, mins, type;
public:
    class no_known_format_found_err {};
    class invalid_time_value_err    {};

    void string_convert_to_lowercase(string alarm_input) {
        transform(alarm_input.begin(), alarm_input.end(), alarm_input.begin(), ::tolower);
        lower_case_string = alarm_input;
    }

    void final_time(size_t colon_index, size_t type_index) {
        hours = lower_case_string.substr(0, colon_index);
        mins  = lower_case_string.substr(colon_index + 1, 2);
        type  = lower_case_string.substr(type_index, 2);
    }

    void input_type_find() {
        if (lower_case_string.find(":") == string::npos)
            throw no_known_format_found_err{};

        size_t colon_index = lower_case_string.find(":");
        string raw_h = lower_case_string.substr(0, colon_index);
        string raw_m = lower_case_string.substr(colon_index + 1, 2);

        for (char c : raw_h) if (!isdigit((unsigned char)c)) throw no_known_format_found_err{};
        for (char c : raw_m) if (!isdigit((unsigned char)c)) throw no_known_format_found_err{};

        int h = stoi(raw_h);
        int m = stoi(raw_m);

        if (h < 0 || h > 23) throw invalid_time_value_err{};
        if (m < 0 || m > 59) throw invalid_time_value_err{};

        if (lower_case_string.find("am") != string::npos)
            final_time(colon_index, lower_case_string.find("am"));
        else if (lower_case_string.find("pm") != string::npos)
            final_time(colon_index, lower_case_string.find("pm"));
        else {
            hours = raw_h;
            mins  = raw_m;
            type  = "stand";
        }
    }
};
class alarm_data : public time_input, virtual public file_save {
protected:
    int second, minute, hour, day, month, year;
    int alarm_ring_day, alarm_ring_month;
    string alarm_time;

public:
    class invalid_day_err   {};
    class invalid_month_err {};

    void get_curr_time() {
        time_t time_now = time(0);
        tm* t = localtime(&time_now);
        second = t->tm_sec;
        minute = t->tm_min;
        hour   = t->tm_hour;
        day    = t->tm_mday;
        month  = t->tm_mon + 1;
        year   = t->tm_year + 1900;
    }

    void get_alarm_info_from_user() {
        cout << "Enter time (e.g. 4:35 am  or  4:35 pm): ";
        getline(cin, alarm_time);
        string_convert_to_lowercase(alarm_time);
        input_type_find();

        cout << "Day: ";
        if (!(cin >> alarm_ring_day)) {
            cin.clear();
            cin.ignore(1000, '\n');
            throw invalid_day_err{};
        }
        if (alarm_ring_day < 1 || alarm_ring_day > 31) throw invalid_day_err{};

        cout << "Month: ";
        if (!(cin >> alarm_ring_month)) {
            cin.clear();
            cin.ignore(1000, '\n');
            throw invalid_month_err{};
        }
        if (alarm_ring_month < 1 || alarm_ring_month > 12) throw invalid_month_err{};
    }

    void save_data_into_file() {
        string data = alarm_time +
            " Date = " + to_string(alarm_ring_day) +
            " Month = " + to_string(alarm_ring_month);
        file_save::save_data(data);
    }

    bool is_valid_line(const string& line) {
        for (char c : line) if (!isspace((unsigned char)c)) return true;
        return false;
    }

    alarm_info get_alarm_data_from_file(const string& line) {
        alarm_info alarm;
        istringstream ss(line);
        string token;

        ss >> alarm.time;
        ss >> token;

        if (token == "am" || token == "pm") {
            alarm.type = token;
            ss >> token;
        } else {
            alarm.type = "stand";
        }
        ss >> token;
        if (token == "=") ss >> alarm.day;
        else alarm.day = stoi(token);

        ss >> token;
        ss >> token;
        if (token == "=") ss >> alarm.month;
        else alarm.month = stoi(token);

        return alarm;
    }

    int to_abs_hour(const alarm_info& alarm) {
        size_t c = alarm.time.find(":");
        int h = stoi(alarm.time.substr(0, c));
        if (alarm.type == "pm" && h != 12) h += 12;
        else if (alarm.type == "am" && h == 12) h = 0;
        return h;
    }

    int to_abs_min(const alarm_info& alarm) {
        size_t c = alarm.time.find(":");
        return stoi(alarm.time.substr(c + 1, 2));
    }

    bool is_past_alarm(const alarm_info& alarm) {
        get_curr_time();
        if (alarm.month < month) return true;
        if (alarm.month == month && alarm.day < day) return true;
        if (alarm.month == month && alarm.day == day) {
            if (to_abs_hour(alarm) < hour) return true;
            if (to_abs_hour(alarm) == hour && to_abs_min(alarm) < minute) return true;
        }
        return false;
    }

    void delete_past_alarms_from_file() {
        ifstream file_in("data/alarm.txt");
        if (!file_in.is_open()) { cout << "error opening file" << endl; return; }

        vector<string> valid_lines;
        string line;
        while (getline(file_in, line)) {
            if (!is_valid_line(line) || line.find(":") == string::npos) continue;
            alarm_info alarm = get_alarm_data_from_file(line);
            if (alarm.time.empty() || alarm.month == 0) continue;
            if (!is_past_alarm(alarm)) valid_lines.push_back(line);
        }
        file_in.close();

        ofstream file_out("data/alarm.txt", ios::trunc);
        if (!file_out.is_open()) { cout << "error writing file" << endl; return; }
        for (auto& l : valid_lines) file_out << l << endl;
        file_out.close();
        cout << "Past alarms deleted." << endl;
    }

    void read_alarms_from_file() {
        all_alarms.clear();
        ifstream file("data/alarm.txt");
        if (!file.is_open()) { cout << "error opening file" << endl; return; }
        string line;
        while (getline(file, line)) {
            if (!is_valid_line(line) || line.find(":") == string::npos) continue;
            alarm_info alarm = get_alarm_data_from_file(line);
            if (alarm.time.empty() || alarm.month == 0) continue;
            all_alarms.push_back(alarm);
        }
        file.close();
        cout << "Loaded " << all_alarms.size() << " alarms!" << endl;
    }
};

class alarm_ring : public alarm_data {
public:
    bool is_alarm_time(alarm_info& alarm) {
        if (alarm.is_true) return false;
        get_curr_time();
        if (alarm.day != day || alarm.month != month) return false;
        return (hour == to_abs_hour(alarm) && minute == to_abs_min(alarm));
    }

    void show_ringing_screen(alarm_info& alarm) {
        system("cls");
        cout << "ALARM RINGING" << endl;
        cout << "Time:- " << alarm.time;
        if (alarm.type == "am" || alarm.type == "pm") cout << " " << alarm.type;
        cout << endl;
        cout << "Date:- " << alarm.day << "/" << alarm.month << endl;
        cout << "\n  Press ENTER to dismiss..." << endl;
    }

    void ring_alarm(alarm_info& alarm) {
        show_ringing_screen(alarm);
        bool stop = false;
        thread beep_thread([&stop]() {
            while (!stop) {
                cout << "\a" << flush;
                this_thread::sleep_for(chrono::seconds(1));
            }
        });
        string input;
        getline(cin, input);
        stop = true;
        beep_thread.join();
        alarm.is_true = true;
        system("cls");
        cout << "Alarm dismissed!\n";
    }

    bool any_pending() {
        for (auto& alarm : all_alarms)
            if (!alarm.is_true) return true;
        return false;
    }

    alarm_info* find_next_alarm() {
        alarm_info* next = nullptr;
        int next_total = INT_MAX;
        get_curr_time();
        for (auto& alarm : all_alarms) {
            if (alarm.is_true) continue;
            int a_hour = to_abs_hour(alarm);
            int a_min  = to_abs_min(alarm);
            if (alarm.month < month) continue;
            if (alarm.month == month && alarm.day < day) continue;
            if (alarm.month == month && alarm.day == day) {
                if (a_hour < hour) continue;
                if (a_hour == hour && a_min <= minute) continue;
            }
            int total = alarm.month * 43200 + alarm.day * 1440 + a_hour * 60 + a_min;
            if (total < next_total) { next_total = total; next = &alarm; }
        }
        return next;
    }

    void start_checking() {
        if (all_alarms.empty()) { cout << "No alarms set.\n"; return; }
        while (any_pending()) {
            get_curr_time();
            system("cls");
            cout << "Time : " << (hour   < 10 ? "0" : "") << hour   << ":"
                              << (minute < 10 ? "0" : "") << minute << ":"
                              << (second < 10 ? "0" : "") << second << "\n";
            cout << "Date : " << day << "/" << month << "/" << year << "\n";

            alarm_info* next = find_next_alarm();
            if (next) {
                cout << "\nNext Alarm : " << next->time;
                if (next->type == "am" || next->type == "pm") cout << " " << next->type;
                cout << " | " << next->day << "/" << next->month << "\n";
            } else {
                cout << "\nNo upcoming alarms.\n";
            }

            cout << "\nAlarms:-\n";
            for (auto& alarm : all_alarms) {
                cout << alarm.time;
                if (alarm.type == "am" || alarm.type == "pm") cout << " " << alarm.type;
                cout << " | " << alarm.day << "/" << alarm.month;
                cout << (alarm.is_true ? " [DONE]" : " [waiting]") << "\n";
            }

            for (auto& alarm : all_alarms)
                if (is_alarm_time(alarm)) ring_alarm(alarm);

            this_thread::sleep_for(chrono::seconds(1));
        }

        system("cls");
        cout << "  NO ALARM!\n";
    }
};
void Alarm() {
    int choice;
    while (true) {
        cout << "1. Add Alarm\n2. Alarms\n > ";
        if (cin >> choice && (choice == 1 || choice == 2)) {
            cin.ignore();
            break;
        }
        cin.clear();
        cin.ignore(1000, '\n');
        cout << "[Error] Invalid choice. Enter 1 or 2.\n\n";
    }

    alarm_ring* a_r = new alarm_ring;

    if (choice == 1) {
        while (true) {
            try {
                a_r->get_alarm_info_from_user();
                cin.ignore();                   
                a_r->save_data_into_file();
                cout << "Alarm saved!\n";
                break;                            

            } catch (alarm_data::invalid_day_err) {
                cin.ignore(1000, '\n');
                cout << "[Error] Day must be a number between 1 and 31. Try again.\n\n";

            } catch (alarm_data::invalid_month_err) {
                cin.ignore(1000, '\n');
                cout << "[Error] Month must be a number between 1 and 12. Try again.\n\n";

            } catch (time_input::no_known_format_found_err) {
                cin.ignore(1000, '\n');
                cout << "[Error] Time format not recognised. Use  HH:MM am / HH:MM pm / HH:MM (24h). Try again.\n\n";

            } catch (time_input::invalid_time_value_err) {
                cin.ignore(1000, '\n');
                cout << "[Error] Hour must be 0-23 and minute 0-59. Try again.\n\n";

            } catch (file_save::file_open_err) {
                cout << "[Error] Could not open alarm file for saving. Check that data/ folder exists.\n";
                break;                          
            }
        }

        delete a_r;
        return;
    }


    a_r->delete_past_alarms_from_file();
    a_r->read_alarms_from_file();

    cout << "\nLoaded alarms:\n";
    try {
        data_fetch_from_file d_f;
        d_f.fetch_data_from_file();
    } catch (data_fetch_from_file::file_read_err) {
        cout << "[Error] Could not read alarm file.\n";
    }

    a_r->start_checking();
    delete a_r;
}
