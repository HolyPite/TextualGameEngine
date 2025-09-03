#include "ui.h"
#include <iostream>
#include <limits>

namespace ui {
    void print(const std::string& s) {
        std::cout << s;
    }

    std::string prompt(const std::string& message) {
        std::cout << message;
        std::string input;
        std::cin >> input;
        return input;
    }

    int promptInt(const std::string& message) {
        while (true) {
            std::cout << message;
            int v;
            if (std::cin >> v) return v;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Entrée invalide. Réessayez.\n";
        }
    }

    void pause(const std::string& message) {
        std::cout << message << std::endl;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cin.get();
    }
}

