#include "Competence.h"
#include <iostream>
#include <sstream>
#include "ui.h"

static const char* skillTypeToLabel(SkillType t) {
    switch (t) {
        case SkillType::Attaque: return "Attaque";
        case SkillType::Soin: return "Soin";
        case SkillType::Protection: return "Protection";
    }
    return "";
}

void Competence::afficher() const {
    std::ostringstream oss;
    oss << nom << " (" << skillTypeToLabel(type) << "): \n";
    switch (type) {
        case SkillType::Attaque: oss << "    Degat: "; break;
        case SkillType::Protection: oss << "    Bouclier: "; break;
        case SkillType::Soin: oss << "    Soin: "; break;
    }
    if (getValueType() == ValueType::Flat) oss << "+" << valeur; else oss << valeur << "%";
    oss << "\n";
    oss << "    Mana: " << coutMana << "\n";
    ui::typewrite_words(oss.str(), 28, 110);
}
