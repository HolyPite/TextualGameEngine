#include "Competence.h"
#include <iostream>

void Competence::afficher() const {
    std::cout << nom << " (" << type << "): \n";
    if (type == "Attaque") {
        std::cout << "    Dégât: ";
    } else if (type == "Protection") {
        std::cout << "    Bouclier: ";
    } else if (type == "Soin") {
        std::cout << "    Soin: ";
    }
    std::cout << (typeValeur == '+' ? "+" : "") << valeur << (typeValeur == '%' ? "%" : "") << "\n";
    std::cout << "    Mana: " << coutMana << "\n";
}
