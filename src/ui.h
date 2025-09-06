#ifndef UI_H
#define UI_H

#include <string>

namespace ui {

// Initialize ANSI color support (Windows + others). Returns true if colors are enabled.
bool init();

// Returns true if colors are enabled.
bool use_color();

// Wrap text with ANSI color if enabled; otherwise returns text unchanged.
std::string color(const std::string& text, const char* ansiCode);

// Clear screen (if colors/ANSI enabled) else prints separators.
void clear();

// Typewriter printing (animated). If ms_per_char < 0, uses default from env (TYPE_MS).
void typewrite(const std::string& text, int ms_per_char = -1);

// ASCII progress bar helper. Colors only applied if enabled.
std::string bar(int current, int maximum, int width = 20,
                char fill = '#', char empty = '-', const char* colorFill = nullptr);

// Common ANSI codes
extern const char* RESET;
extern const char* BOLD;
extern const char* DIM;
extern const char* RED;
extern const char* GREEN;
extern const char* YELLOW;
extern const char* BLUE;
extern const char* MAGENTA;
extern const char* CYAN;
extern const char* WHITE;

}

#endif // UI_H
