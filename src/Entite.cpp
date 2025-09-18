#include "Entite.h"
#include <algorithm>
#include <optional>
#include <sstream>

Entite::Entite(const std::string& nom_, int pv, int pm, int def)
    : nom(nom_), PV(pv), PVmax(pv), PM(pm), PMmax(pm), defenseBase(def), defenseBoost(0),
      protectionUtilisee(false) {
    equipements.resize(2, {"Aucun", 0});
}

Entite::~Entite() = default;

void Entite::setUI(GameUI* ui) {
    uiIface = ui;
}

GameUI& Entite::out() const {
    return uiIface ? *uiIface : activeGameUI();
}

void Entite::setAtkBase(int v) {
    atkBase = std::max(0, v);
}

void Entite::setSpdBase(int v) {
    spdBase = std::max(0, v);
}

void Entite::defendre(int degats) {
    int buffDef = 0;
    for (const auto& effect : activeEffects) {
        if (effect.type == EffectType::BUFF_DEF) {
            buffDef += effect.value;
        }
    }
    int passive = 0;
    auto itPassive = itemEffects.find(equipements[1].first);
    if (itPassive != itemEffects.end() && itPassive->second.type == EffectType::BUFF_DEF && itPassive->second.duration == 0) {
        passive = itPassive->second.value;
    }
    int defenseTotale = defenseBase + equipements[1].second + defenseBoost + passive + buffDef;
    int degatsReduits = std::max(degats - defenseTotale, 0);
    PV -= degatsReduits;
    if (PV < 0) PV = 0;
    std::ostringstream oss;
    oss << displayName() << " subit " << degatsReduits << " degats apres reduction par la defense.\n";
    out().typewriteWords(oss.str(), 28, 110);
}

void Entite::afficherStats(bool colorName, const char* colorCode) const {
    std::string shownName = colorName ? ui::color(nom, colorCode)
                                      : (isPlayer ? ui::color(nom, ui::GREEN) : ui::color(nom, ui::MAGENTA));
    std::ostringstream oss;
    oss << "\n~~~~~~~~~ Stats : " << shownName << " ~~~~~~~~~\n"
        << "PV: " << PV << "/" << PVmax << "\n"
        << "Mana: " << PM << "/" << PMmax << "\n"
        << "Defense: " << defenseBase << "\n"
        << "ATK: " << atkBase << "\n"
        << "SPD: " << spdBase << "\n"
        << "Or: " << gold << "\n";
    if (protectionUtilisee) {
        oss << "Boost de defense: " << defenseBoost << "\n";
    }
    oss << "Arme: " << equipements[0].first << " (+ " << equipements[0].second << ")\n"
        << "Armure: " << equipements[1].first << " (+ " << equipements[1].second << ")\n\n";
    out().typewriteWords(oss.str(), 28, 110);
}

void Entite::ajouterOr(int delta) {
    gold += delta;
    if (gold < 0) gold = 0;
    if (delta > 0) {
        out().typewriteWords("+" + std::to_string(delta) + " or obtenu.\n", 28, 110);
    } else if (delta < 0) {
        out().typewriteWords(std::to_string(delta) + " or depense.\n", 28, 110);
    }
}

void Entite::ajouterCompetence(const Competence& competence) {
    competences.push_back(competence);
}

void Entite::afficherCompetences() const {
    GameUI& io = const_cast<Entite&>(*this).out();
    for (size_t i = 0; i < competences.size(); ++i) {
        io.print(std::to_string(i + 1) + ". ");
        competences[i].afficher(io);
    }
    if (!competences.empty()) {
        io.print("\n");
    }
}

void Entite::ajouterArme(const std::string& nomItem, int valeur) {
    inventaireArmes.emplace_back(nomItem, valeur);
    out().printLine("Arme ajoutee : " + nomItem + " (+ " + std::to_string(valeur) + ")");
}

void Entite::ajouterArmure(const std::string& nomItem, int valeur) {
    inventaireArmures.emplace_back(nomItem, valeur);
    out().printLine("Armure ajoutee : " + nomItem + " (+ " + std::to_string(valeur) + ")");
}

void Entite::equiperArme(const std::string& nomItem, int valeur) {
    equipements[0] = {nomItem, valeur};
    out().printLine("Arme equipee : " + nomItem + " (+ " + std::to_string(valeur) + ")");
}

void Entite::equiperArmure(const std::string& nomItem, int valeur) {
    equipements[1] = {nomItem, valeur};
    out().printLine("Armure equipee : " + nomItem + " (+ " + std::to_string(valeur) + ")");
}

void Entite::afficherInventaire() const {
    std::ostringstream oss;
    oss << "\n--- Inventaire ---\n";
    oss << "Armes :\n";
    for (size_t i = 0; i < inventaireArmes.size(); ++i) {
        oss << i + 1 << ". " << inventaireArmes[i].first << " (+ " << inventaireArmes[i].second << ")\n";
    }
    oss << "Armures :\n";
    for (size_t i = 0; i < inventaireArmures.size(); ++i) {
        oss << i + 1 << ". " << inventaireArmures[i].first << " (+ " << inventaireArmures[i].second << ")\n";
    }
    oss << "\n";
    out().print(oss.str());
}

void Entite::equiperObjet() {
    GameUI& io = out();
    afficherInventaire();
    io.print("Choisissez une categorie :\n1. Armes\n2. Armures\n");
    int categorie = 0;
    if (!io.readInt(categorie)) {
        io.printLine("Entree invalide.");
        return;
    }

    auto choisirIndice = [&](const std::vector<std::pair<std::string, int>>& items, const std::string& prompt) -> std::optional<int> {
        if (items.empty()) {
            io.printLine("Inventaire vide.");
            return std::nullopt;
        }
        io.print(prompt);
        int idx = 0;
        if (!io.readInt(idx)) {
            io.printLine("Entree invalide.");
            return std::nullopt;
        }
        if (idx <= 0 || idx > static_cast<int>(items.size())) {
            io.printLine("Choix invalide.");
            return std::nullopt;
        }
        return idx - 1;
    };

    if (categorie == 1) {
        auto choix = choisirIndice(inventaireArmes, "Choisissez une arme a equiper : ");
        if (choix.has_value()) {
            equiperArme(inventaireArmes[*choix].first, inventaireArmes[*choix].second);
            io.printLine("Vous avez equipe l'arme " + inventaireArmes[*choix].first + ".");
        }
    } else if (categorie == 2) {
        auto choix = choisirIndice(inventaireArmures, "Choisissez une armure a equiper : ");
        if (choix.has_value()) {
            equiperArmure(inventaireArmures[*choix].first, inventaireArmures[*choix].second);
            io.printLine("Vous avez equipe l'armure " + inventaireArmures[*choix].first + ".");
        }
    } else {
        io.printLine("Choix invalide ou inventaire vide.");
    }
}

void Entite::attaquer(Entite& cible, char typeValeur, int valeur) {
    int degats = (typeValeur == '%') ? cible.PVmax * valeur / 100 : valeur;
    degats += equipements[0].second;
    std::ostringstream oss;
    oss << displayName() << " inflige " << degats << " degats a " << cible.displayName() << " !\n";
    out().typewriteWords(oss.str(), 28, 110);
    cible.defendre(degats);

    auto itEffet = itemEffects.find(equipements[0].first);
    if (itEffet != itemEffects.end() && itEffet->second.type == EffectType::DOT && itEffet->second.value > 0 && itEffet->second.duration > 0) {
        cible.activeEffects.push_back({EffectType::DOT, itEffet->second.label, itEffet->second.value, itEffet->second.duration});
        if (!itEffet->second.label.empty()) {
            std::ostringstream oss2;
            oss2 << cible.displayName() << " est affecte par " << itEffet->second.label
                 << " (" << itEffet->second.value << ", " << itEffet->second.duration << " tours).\n";
            out().typewriteWords(oss2.str(), 28, 110);
        }
    }
}

void Entite::soigner(char typeValeur, int valeur) {
    int soin = (typeValeur == '%') ? PVmax * valeur / 100 : valeur;
    PV = std::min(PV + soin, PVmax);
    std::ostringstream oss;
    oss << displayName() << " restaure " << soin << " PV !\n";
    out().typewriteWords(oss.str(), 28, 110);
}

void Entite::proteger(char typeValeur, int valeur) {
    if (!protectionUtilisee) {
        if (typeValeur == '%') {
            defenseBoost = (defenseBase + equipements[1].second) * valeur / 100;
        } else {
            defenseBoost = valeur;
        }
        protectionUtilisee = true;
        std::ostringstream oss;
        oss << displayName() << " augmente sa defense de " << defenseBoost << " !\n";
        out().typewriteWords(oss.str(), 28, 110);
    } else {
        out().typewriteWords("Protection deja utilisee dans ce combat !\n", 28, 110);
    }
}

void Entite::reinitialiserProtection() {
    protectionUtilisee = false;
    defenseBoost = 0;
}

bool Entite::utiliserCompetence(int index, Entite& cible) {
    if (index < 0 || index >= static_cast<int>(competences.size())) {
        return false;
    }
    const Competence& comp = competences[index];
    if (PM < comp.getCoutMana()) {
        out().typewriteWords("Pas assez de mana pour utiliser " + comp.getNom() + " !\n", 28, 110);
        return false;
    }
    PM -= comp.getCoutMana();
    char tv = (comp.getValueType() == ValueType::Percent ? '%' : '+');
    if (comp.getType() == SkillType::Attaque) {
        attaquer(cible, tv, comp.getValeur());
    } else if (comp.getType() == SkillType::Soin) {
        soigner(tv, comp.getValeur());
    } else if (comp.getType() == SkillType::Protection) {
        proteger(tv, comp.getValeur());
    }
    return true;
}

void Entite::registerItemEffect(const std::string& itemName, const std::string& effName, int value, int duration) {
    EffectType type = (effName == "DOT") ? EffectType::DOT : (effName == "BUFF_DEF" ? EffectType::BUFF_DEF : EffectType::None);
    itemEffects[itemName] = {type, effName, value, duration};
}

void Entite::registerItemEffect(const std::string& itemName, const std::string& effName, const std::string& displayName, int value, int duration) {
    EffectType type = (effName == "DOT") ? EffectType::DOT : (effName == "BUFF_DEF" ? EffectType::BUFF_DEF : EffectType::None);
    itemEffects[itemName] = {type, displayName, value, duration};
}

void Entite::registerItemEffect(const std::string& itemName, ItemEffectType eff, int value, int duration) {
    EffectType type = (eff == ItemEffectType::DOT) ? EffectType::DOT : (eff == ItemEffectType::BUFF_DEF ? EffectType::BUFF_DEF : EffectType::None);
    itemEffects[itemName] = {type, std::string(), value, duration};
}

void Entite::registerItemEffect(const std::string& itemName, ItemEffectType eff, const std::string& label, int value, int duration) {
    EffectType type = (eff == ItemEffectType::DOT) ? EffectType::DOT : (eff == ItemEffectType::BUFF_DEF ? EffectType::BUFF_DEF : EffectType::None);
    itemEffects[itemName] = {type, label, value, duration};
}

void Entite::tickEffects() {
    for (auto& effect : activeEffects) {
        if (effect.remaining > 0) {
            if (effect.type == EffectType::DOT) {
                PV -= effect.value;
                if (PV < 0) PV = 0;
                std::ostringstream oss;
                oss << displayName() << " subit " << effect.value;
                if (!effect.label.empty()) {
                    oss << " (" << effect.label << ")";
                }
                oss << ".\n";
                out().typewriteWords(oss.str(), 28, 110);
            }
            effect.remaining -= 1;
        }
    }
    activeEffects.erase(std::remove_if(activeEffects.begin(), activeEffects.end(), [](const ActiveEffect& effect) {
        return effect.remaining <= 0;
    }), activeEffects.end());
}
