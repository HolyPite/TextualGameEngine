#include "Competence.h"
#include "GameUI.h"
#include <sstream>

namespace {
const char* skillTypeToLabel(SkillType t) {
    switch (t) {
        case SkillType::Attaque: return "Attaque";
        case SkillType::Soin: return "Soin";
        case SkillType::Protection: return "Protection";
    }
    return "";
}
}

void Competence::afficher(GameUI& ui) const {
    std::ostringstream oss;
    oss << nom << " (" << skillTypeToLabel(type) << "): \n";
    switch (type) {
        case SkillType::Attaque: oss << "    Degat: "; break;
        case SkillType::Protection: oss << "    Bouclier: "; break;
        case SkillType::Soin: oss << "    Soin: "; break;
    }
    if (getValueType() == ValueType::Flat) {
        oss << "+" << valeur;
    } else {
        oss << valeur << "%";
    }
    oss << "\n";
    oss << "    Mana: " << coutMana << "\n";
    ui.typewriteWords(oss.str(), 28, 110);
}
