#include "ui.h"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>

#ifdef _WIN32
#  include <windows.h>
#endif

namespace ui {

static bool g_use_color = false;
static int  g_type_ms = 0; // 0 = disabled

const char* RESET   = "\x1b[0m";
const char* BOLD    = "\x1b[1m";
const char* DIM     = "\x1b[2m";
const char* RED     = "\x1b[31m";
const char* GREEN   = "\x1b[32m";
const char* YELLOW  = "\x1b[33m";
const char* BLUE    = "\x1b[34m";
const char* MAGENTA = "\x1b[35m";
const char* CYAN    = "\x1b[36m";
const char* WHITE   = "\x1b[37m";

bool init() {
    const char* noColor = std::getenv("NO_COLOR");
    if (noColor) { g_use_color = false; return g_use_color; }
#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) { g_use_color = false; return g_use_color; }
    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode)) { g_use_color = false; return g_use_color; }
    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    if (!SetConsoleMode(hOut, dwMode)) { g_use_color = false; return g_use_color; }
    g_use_color = true;
#else
    g_use_color = true;
#endif
    return g_use_color;
}

bool use_color() { return g_use_color; }

std::string color(const std::string& text, const char* ansiCode) {
    if (!g_use_color) return text;
    return std::string(ansiCode) + text + RESET;
}

void clear() {
    const char* env = std::getenv("CLEAR_SCREEN");
    if (env && std::string(env) == "0") { return; }
    if (g_use_color) {
        std::cout << "\x1b[2J\x1b[H";
    } else {
        std::cout << "\n\n----------------------------------------\n\n";
    }
}

void typewrite(const std::string& text, int ms_per_char) {
    int delay = ms_per_char;
    if (delay < 0) {
        if (g_type_ms == 0) {
            const char* env = std::getenv("TYPE_MS");
            if (env) { try { g_type_ms = std::max(0, std::stoi(env)); } catch (...) { g_type_ms = 0; } }
        }
        delay = g_type_ms;
    }
    if (delay <= 0) { std::cout << text; return; }
    for (char c : text) {
        std::cout << c << std::flush;
        if (c == '\n') { std::this_thread::sleep_for(std::chrono::milliseconds(delay)); }
        else { std::this_thread::sleep_for(std::chrono::milliseconds(delay)); }
    }
}

std::string bar(int current, int maximum, int width, char fill, char empty, const char* colorFill) {
    if (maximum <= 0) maximum = 1;
    if (current < 0) current = 0; if (current > maximum) current = maximum;
    int filled = (int)((long long)current * width / maximum);
    std::string s;
    s.reserve(width + 2);
    s.push_back('[');
    for (int i = 0; i < width; ++i) {
        if (i < filled) s.push_back(fill); else s.push_back(empty);
    }
    s.push_back(']');
    if (g_use_color && colorFill) return std::string(colorFill) + s + RESET;
    return s;
}

} // namespace ui
