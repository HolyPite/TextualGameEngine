#include "GameUI.h"
#include "ui.h"
#include <iostream>
#include <limits>

namespace {
ConsoleGameUI g_defaultUI;
GameUI* g_activeUI = &g_defaultUI;
}

GameUI& activeGameUI() {
    return *g_activeUI;
}

void setActiveGameUI(GameUI* ui) {
    g_activeUI = ui ? ui : &g_defaultUI;
}

void ConsoleGameUI::clear() {
    ui::clear();
}

void ConsoleGameUI::hr(const char* pattern, const char* colorCode) {
    ui::hr(pattern, colorCode);
}

void ConsoleGameUI::typewriteWords(const std::string& text, int spaceMs, int newlineMs) {
    ui::typewrite_words(text, spaceMs, newlineMs);
}

void ConsoleGameUI::typewriteJitter(const std::string& text, int baseMs, int jitterMs) {
    ui::typewrite_jitter(text, baseMs, jitterMs);
}

void ConsoleGameUI::print(const std::string& text) {
    std::cout << text;
}

void ConsoleGameUI::printLine(const std::string& text) {
    std::cout << text << '\n';
}

void ConsoleGameUI::flush() {
    std::cout << std::flush;
}

bool ConsoleGameUI::readToken(std::string& out) {
    if (!(std::cin >> out)) {
        return false;
    }
    return true;
}

bool ConsoleGameUI::readInt(int& out) {
    if (!(std::cin >> out)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return false;
    }
    return true;
}

bool ConsoleGameUI::readLine(std::string& out) {
    if (!std::getline(std::cin, out)) {
        return false;
    }
    return true;
}

void ConsoleGameUI::discardLine() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}
