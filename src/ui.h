#ifndef UI_H
#define UI_H

#include <string>

namespace ui {

// Initialize ANSI color support (Windows + others). Returns true if colors are enabled.
bool init();

// Wrap text with ANSI color if enabled; otherwise returns text unchanged.
std::string color(const std::string& text, const char* ansiCode);

// Clear screen (if colors/ANSI enabled) else prints separators.
void clear();

// Console width in columns (best effort). Fallback to 60.
int columns();

// Horizontal rule across full width. Pattern should be 1 printable glyph
// (supports UTF-8 like "─"). Colored if ANSI is enabled.
void hr(const char* pattern = "-", const char* colorCode = nullptr);

// ASCII progress bar helper. Colors only applied if enabled.
std::string bar(int current, int maximum, int width = 20,
                char fill = '#', char empty = '-', const char* colorFill = nullptr);

// Word-by-word typewriter: no delay within words; tiny pause on spaces/newlines.
// If delays are negative, reads env TYPE_SPACE_MS and TYPE_NL_MS; defaults to 8ms and 22ms.
void typewrite_words(const std::string& text, int space_ms = -1, int newline_ms = -1);

// Jittered typewriter: per-character delay with +-jitter to mimic irregular handwriting.
// If params are negative, reads env TYPE_MS (base) and TYPE_JITTER_MS; defaults base 12ms, jitter 6ms.
void typewrite_jitter(const std::string& text, int base_ms = -1, int jitter_ms = -1);

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
