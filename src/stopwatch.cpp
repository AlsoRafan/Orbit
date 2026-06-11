
#include "../include/stopwatch.h"
using namespace std;

#ifndef _WIN32
termios Terminal::saved_term;
#endif

void Terminal::init() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD  m = 0; GetConsoleMode(h, &m);
    SetConsoleMode(h, m | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
#endif
}

void Terminal::clear_line() { cout << "\r\033[2K"; }

char Terminal::get_key() {
#ifdef _WIN32
    return static_cast<char>(_getch());
#else
    char c = 0; read(STDIN_FILENO, &c, 1); return c;
#endif
}

void Terminal::enable_raw() {
#ifndef _WIN32
    tcgetattr(STDIN_FILENO, &saved_term);
    termios raw = saved_term;
    raw.c_lflag &= ~(ICANON | ECHO);
    raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
#endif
}

void Terminal::disable_raw() {
#ifndef _WIN32
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &saved_term);
#endif
}

long long Stopwatch::live_ms() const {
    return chrono::duration_cast<chrono::milliseconds>(
        chrono::steady_clock::now() - start_tp).count();
}

void Stopwatch::start() {
    lock_guard<mutex> lock(mtx);
    saved_ms = 0; lap_list.clear();
    start_tp  = chrono::steady_clock::now();
    cur_state = State::running;
}

void Stopwatch::pause() {
    lock_guard<mutex> lock(mtx);
    if (cur_state != State::running) return;
    saved_ms += live_ms();
    cur_state = State::paused;
}

void Stopwatch::resume() {
    lock_guard<mutex> lock(mtx);
    if (cur_state != State::paused) return;
    start_tp  = chrono::steady_clock::now();
    cur_state = State::running;
}

bool Stopwatch::lap() {
    lock_guard<mutex> lock(mtx);
    if (cur_state != State::running) return false;
    long long total = saved_ms + live_ms();
    long long prev  = lap_list.empty() ? 0 : lap_list.back().total_ms;
    lap_list.push_back({ (int)lap_list.size() + 1, total - prev, total });
    return true;
}

void Stopwatch::reset() {
    lock_guard<mutex> lock(mtx);
    cur_state = State::stopped; saved_ms = 0; lap_list.clear();
}

State     Stopwatch::state()   const { lock_guard<mutex> lock(mtx); return cur_state; }
long long Stopwatch::elapsed() const {
    lock_guard<mutex> lock(mtx);
    return saved_ms + (cur_state == State::running ? live_ms() : 0LL);
}
const vector<Lap>& Stopwatch::laps() const { return lap_list; }

string Stopwatch::format_time(long long ms) {
    ostringstream s;
    s << setfill('0')
      << setw(2) << ms / 3600000         << ":"
      << setw(2) << ms % 3600000 / 60000 << ":"
      << setw(2) << ms % 60000   / 1000  << "."
      << setw(3) << ms % 1000;
    return s.str();
}

const char* Stopwatch::state_label(State s) {
    switch (s) {
        case State::running: return "[running]";
        case State::paused:  return "[paused] ";
        default:             return "[stopped]";
    }
}

string Stopwatch::format_current() const { return format_time(elapsed()); }

void App::display_loop() {
    int shown_laps = 0;
    while (!quit) {
        int cur_laps = (int)sw.laps().size();
        if (laps_dirty.exchange(false) && cur_laps > shown_laps) {
            for (int i = shown_laps; i < cur_laps; ++i) {
                const Lap& l = sw.laps()[i];
                cout << "\n  Lap " << setfill('0') << setw(2) << l.index
                     << "  total: " << Stopwatch::format_time(l.total_ms)
                     << "  split: " << Stopwatch::format_time(l.split_ms);
            }
            cout << "\n";
            shown_laps = cur_laps;
        }
        Terminal::clear_line();
        cout << "  " << sw.format_current() << "  "
             << Stopwatch::state_label(sw.state()) << flush;
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void App::handle(char c) {
    switch (c | 32) {
        case 's': sw.start();                   laps_dirty = false; break;
        case 'p': sw.pause();                                       break;
        case 'r': sw.resume();                                      break;
        case 'l': if (sw.lap()) laps_dirty = true;                  break;
        case 'x': sw.reset();                   laps_dirty = false; break;
        case 'q': quit = true;                                      break;
        default:  break;
    }
}

void App::run() {
    Terminal::init();
    Terminal::enable_raw();

    cout << "\n"
         << "  [s] Start   [p] Pause   [r] Resume\n"
         << "  [l] Lap     [x] Reset   [q] Quit\n\n"
         << "  00:00:00.000 [stopped]" << flush;

    thread disp(&App::display_loop, this);
    while (!quit) handle(Terminal::get_key());
    disp.join();

    Terminal::disable_raw();

    cout << "\n\n  Final time : " << sw.format_current() << "\n";
    if (!sw.laps().empty()) {
        cout << "  Laps:\n";
        for (const auto& l : sw.laps())
            cout << "    Lap " << setfill('0') << setw(2) << l.index
                 << "  total: " << Stopwatch::format_time(l.total_ms)
                 << "  split: " << Stopwatch::format_time(l.split_ms) << "\n";
    }
    cout << "\n";
}

void stopwatch() {
    App app;
    app.run();
}