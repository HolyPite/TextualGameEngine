#ifndef UI_H
#define UI_H

#include <string>

namespace ui {
    void print(const std::string& s);
    std::string prompt(const std::string& message);
    int promptInt(const std::string& message);
    void pause(const std::string& message = "Appuyez sur une touche pour continuer...");
}

#endif // UI_H
