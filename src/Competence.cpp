#include "Competence.h"
#include <iostream>

const char* toString(SkillType type) {
    switch (type) {
        case SkillType::Attack: return "Attaque";
        case SkillType::Heal: return "Soin";
        case SkillType::Protection: return "Protection";
        default: return "Inconnu";
    }
}

void Competence::afficher() const {
    std::cout << nom << " (" << toString(type) << "): \n";
    switch (type) {
        case SkillType::Attack:
            std::cout << "    Dégât: ";
            break;
        case SkillType::Protection:
            std::cout << "    Bouclier: ";
            break;
        case SkillType::Heal:
            std::cout << "    Soin: ";
            break;
    }
    std::cout << (typeValeur == '+' ? "+" : "") << valeur << (typeValeur == '%' ? "%" : "") << "\n";
    std::cout << "    Mana: " << coutMana << "\n";
}
