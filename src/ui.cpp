#include "ui.h"
#include <cstdlib>
#include <iostream>
#include <thread>
#include <chrono>
#include <random>

#ifdef _WIN32
#  include <windows.h>
#endif

namespace ui {

static bool g_use_color = false;
static int  g_type_ms = 0; // 0 = disabled
static int  g_type_space_ms = -1; // -1 = unset
static int  g_type_nl_ms = -1;    // -1 = unset
static int  g_type_jitter_ms = -1; // -1 = unset

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
    // Ensure UTF-8 codepage for proper accents rendering
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    g_use_color = true;
#else
    g_use_color = true;
#endif
    return g_use_color;
}

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

static int detectColumns() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        int w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        if (w > 0) return w;
    }
#endif
    const char* envCols = std::getenv("COLUMNS");
    if (envCols) {
        try { int v = std::stoi(envCols); if (v > 20 && v < 1000) return v; } catch(...) {}
    }
    return 60;
}

int columns() { return detectColumns(); }

void hr(const char* pattern, const char* colorCode) {
    int w = columns();
    std::string unit = pattern ? pattern : "-";
    if (unit.empty()) unit = "-";
    std::string line; line.reserve(w * (int)unit.size());
    for (int i=0;i<w;i++) line += unit;
    if (g_use_color && colorCode) std::cout << colorCode << line << RESET << "\n";
    else std::cout << line << "\n";
}

void typewrite_words(const std::string& text, int space_ms, int newline_ms) {
    if (space_ms < 0) {
        if (g_type_space_ms < 0) {
            const char* env = std::getenv("TYPE_SPACE_MS");
            if (env) { try { g_type_space_ms = std::max(0, std::stoi(env)); } catch(...) { g_type_space_ms = 8; } }
            else g_type_space_ms = 8;
        }
        space_ms = g_type_space_ms;
    }
    if (newline_ms < 0) {
        if (g_type_nl_ms < 0) {
            const char* env = std::getenv("TYPE_NL_MS");
            if (env) { try { g_type_nl_ms = std::max(0, std::stoi(env)); } catch(...) { g_type_nl_ms = 22; } }
            else g_type_nl_ms = 22;
        }
        newline_ms = g_type_nl_ms;
    }
    for (char c : text) {
        std::cout << c << std::flush;
        if (c == ' ') { if (space_ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(space_ms)); }
        else if (c == '\n') { if (newline_ms > 0) std::this_thread::sleep_for(std::chrono::milliseconds(newline_ms)); }
    }
}

void typewrite_jitter(const std::string& text, int base_ms, int jitter_ms) {
    if (base_ms < 0) {
        if (g_type_ms == 0) {
            const char* env = std::getenv("TYPE_MS");
            if (env) { try { g_type_ms = std::max(0, std::stoi(env)); } catch (...) { g_type_ms = 0; } }
        }
        if (g_type_ms <= 0) base_ms = 12; else base_ms = g_type_ms;
    }
    if (jitter_ms < 0) {
        if (g_type_jitter_ms < 0) {
            const char* env = std::getenv("TYPE_JITTER_MS");
            if (env) { try { g_type_jitter_ms = std::max(0, std::stoi(env)); } catch(...) { g_type_jitter_ms = 6; } }
            else g_type_jitter_ms = 6;
        }
        jitter_ms = g_type_jitter_ms;
    }
    if (base_ms <= 0) { std::cout << text; return; }
    static thread_local std::mt19937 rng{ std::random_device{}() };
    std::uniform_int_distribution<int> dist(-jitter_ms, jitter_ms);
    for (char c : text) {
        std::cout << c << std::flush;
        int d = base_ms + dist(rng);
        if (d < 0) d = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(d));
    }
}

std::string bar(int current, int maximum, int width, char fill, char empty, const char* colorFill) {
    if (maximum <= 0) maximum = 1;
    if (current < 0) current = 0;
    if (current > maximum) current = maximum;
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
