#include "Competence.h"
#include <iostream>

static const char* skillTypeToLabel(SkillType t) {
    switch (t) {
        case SkillType::Attaque: return "Attaque";
        case SkillType::Soin: return "Soin";
        case SkillType::Protection: return "Protection";
    }
    return "";
}

void Competence::afficher() const {
    std::cout << nom << " (" << skillTypeToLabel(type) << "): \n";
    switch (type) {
        case SkillType::Attaque: std::cout << "    Degat: "; break;
        case SkillType::Protection: std::cout << "    Bouclier: "; break;
        case SkillType::Soin: std::cout << "    Soin: "; break;
    }
    if (getValueType() == ValueType::Flat) std::cout << "+" << valeur;
    else std::cout << valeur << "%";
    std::cout << "\n";
    std::cout << "    Mana: " << coutMana << "\n";
}

