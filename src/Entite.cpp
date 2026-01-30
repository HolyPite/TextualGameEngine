#include "Entite.h"
#include <algorithm>
#include <optional>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <memory>
namespace {
Entite::EffectType readEffectType(int raw) {
    switch (raw) {
        case 0: return Entite::EffectType::None;
        case 1: return Entite::EffectType::DOT;
        case 2: return Entite::EffectType::BUFF_DEF;
        default: throw std::runtime_error("Invalid effect type value");
    }
}

SkillType readSkillType(int raw) {
    switch (raw) {
        case 0: return SkillType::Attaque;
        case 1: return SkillType::Soin;
        case 2: return SkillType::Protection;
        default: throw std::runtime_error("Invalid skill type value");
    }
}

ValueType readValueType(int raw) {
    switch (raw) {
        case 0: return ValueType::Flat;
        case 1: return ValueType::Percent;
        default: throw std::runtime_error("Invalid value type value");
    }
}
}



Entite::Entite(const std::string& nom_, int pv, int pm, int def)
    : nom(nom_), PV(pv), PVmax(pv), PM(pm), PMmax(pm), defenseBase(def), defenseBoost(0),
      protectionUtilisee(false) {
    equipements.resize(2, {"Aucun", 0});
}

Entite::~Entite() = default;

void Entite::serialize(std::ostream& out) const {
    out << "ENTITE\n";
    out << std::quoted(nom) << ' '
        << (isPlayer ? 1 : 0) << ' '
        << PV << ' ' << PVmax << ' ' << PM << ' ' << PMmax << ' '
        << defenseBase << ' ' << defenseBoost << ' '
        << atkBase << ' ' << spdBase << ' '
        << (protectionUtilisee ? 1 : 0) << ' '
        << gold << '\n';

    out << activeEffects.size() << '\n';
    for (const auto& eff : activeEffects) {
        out << static_cast<int>(eff.type) << ' ' << std::quoted(eff.label) << ' ' << eff.value << ' ' << eff.remaining << '\n';
    }

    out << itemEffects.size() << '\n';
    for (const auto& kv : itemEffects) {
        out << std::quoted(kv.first) << ' '
            << static_cast<int>(kv.second.type) << ' '
            << std::quoted(kv.second.label) << ' '
            << kv.second.value << ' ' << kv.second.duration << '\n';
    }

    auto writePairs = [&](const std::vector<std::pair<std::string, int>>& vec) {
        out << vec.size() << '\n';
        for (const auto& entry : vec) {
            out << std::quoted(entry.first) << ' ' << entry.second << '\n';
        }
    };

    writePairs(inventaireArmes);
    writePairs(inventaireArmures);
    writePairs(equipements);

    out << competences.size() << '\n';
    for (const auto& comp : competences) {
        out << std::quoted(comp.getNom()) << ' '
            << static_cast<int>(comp.getType()) << ' '
            << static_cast<int>(comp.getValueType()) << ' '
            << comp.getValeur() << ' ' << comp.getCoutMana() << '\n';
    }

    out << "ENDENTITE\n";
}

std::unique_ptr<Entite> Entite::deserialize(std::istream& in, GameUI* ui) {
    std::string header;
    if (!(in >> header) || header != "ENTITE") {
        throw std::runtime_error("Invalid entity header");
    }

    std::string nomLu;
    int isPlayerFlag = 0;
    int pv = 0, pvMaxLu = 0, pm = 0, pmMaxLu = 0;
    int defBaseLu = 0, defBoostLu = 0, atkBaseLu = 0, spdBaseLu = 0;
    int protectionFlag = 0;
    int goldLu = 0;

    if (!(in >> std::quoted(nomLu) >> isPlayerFlag >> pv >> pvMaxLu >> pm >> pmMaxLu
          >> defBaseLu >> defBoostLu >> atkBaseLu >> spdBaseLu >> protectionFlag >> goldLu)) {
        throw std::runtime_error("Invalid entity core data");
    }

    auto ent = std::unique_ptr<Entite>(new Entite(nomLu, pvMaxLu, pmMaxLu, defBaseLu));
    ent->PV = pv;
    ent->PVmax = pvMaxLu;
    ent->PM = pm;
    ent->PMmax = pmMaxLu;
    ent->defenseBase = defBaseLu;
    ent->defenseBoost = defBoostLu;
    ent->atkBase = atkBaseLu;
    ent->spdBase = spdBaseLu;
    ent->protectionUtilisee = (protectionFlag != 0);
    ent->gold = goldLu;
    ent->isPlayer = (isPlayerFlag != 0);
    ent->uiIface = ui;

    std::size_t activeCount = 0;
    if (!(in >> activeCount)) {
        throw std::runtime_error("Invalid entity active effect count");
    }
    ent->activeEffects.clear();
    ent->activeEffects.reserve(activeCount);
    for (std::size_t i = 0; i < activeCount; ++i) {
        int typeRaw = 0;
        std::string label;
        int value = 0, remaining = 0;
        if (!(in >> typeRaw >> std::quoted(label) >> value >> remaining)) {
            throw std::runtime_error("Invalid entity active effect entry");
        }
        ent->activeEffects.push_back({readEffectType(typeRaw), label, value, remaining});
    }

    std::size_t effectCount = 0;
    if (!(in >> effectCount)) {
        throw std::runtime_error("Invalid entity item effect count");
    }
    ent->itemEffects.clear();
    for (std::size_t i = 0; i < effectCount; ++i) {
        std::string key;
        int typeRaw = 0;
        std::string label;
        int value = 0, duration = 0;
        if (!(in >> std::quoted(key) >> typeRaw >> std::quoted(label) >> value >> duration)) {
            throw std::runtime_error("Invalid entity item effect entry");
        }
        ent->itemEffects[key] = {readEffectType(typeRaw), label, value, duration};
    }

    auto readPairs = [&](std::vector<std::pair<std::string, int>>& vec) {
        std::size_t count = 0;
        if (!(in >> count)) {
            throw std::runtime_error("Invalid entity inventory count");
        }
        vec.clear();
        vec.reserve(count);
        for (std::size_t i = 0; i < count; ++i) {
            std::string name;
            int value = 0;
            if (!(in >> std::quoted(name) >> value)) {
                throw std::runtime_error("Invalid entity inventory entry");
            }
            vec.emplace_back(std::move(name), value);
        }
    };

    readPairs(ent->inventaireArmes);
    readPairs(ent->inventaireArmures);
    readPairs(ent->equipements);
    if (ent->equipements.size() < 2) {
        ent->equipements.resize(2, {"Aucun", 0});
    }

    std::size_t compCount = 0;
    if (!(in >> compCount)) {
        throw std::runtime_error("Invalid entity competence count");
    }
    ent->competences.clear();
    ent->competences.reserve(compCount);
    for (std::size_t i = 0; i < compCount; ++i) {
        std::string nomComp;
        int typeRaw = 0, valTypeRaw = 0, valeur = 0, mana = 0;
        if (!(in >> std::quoted(nomComp) >> typeRaw >> valTypeRaw >> valeur >> mana)) {
            throw std::runtime_error("Invalid entity competence entry");
        }
        ent->competences.emplace_back(nomComp, readSkillType(typeRaw), readValueType(valTypeRaw), valeur, mana);
    }

    std::string footer;
    if (!(in >> footer) || footer != "ENDENTITE") {
        throw std::runtime_error("Invalid entity footer");
    }

    return ent;
}

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
