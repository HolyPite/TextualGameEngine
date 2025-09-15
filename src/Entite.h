#ifndef ENTITE_H
#define ENTITE_H

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <optional>
#include <algorithm>
#include <unordered_map>
#include "Competence.h"
#include "ui.h"

class Entite {
protected:
    std::string nom;
    bool isPlayer{false};
    int PV;
    int PVmax;
    int PM;
    int PMmax;
    int defenseBase;
    int defenseBoost;
    int atkBase{5};
    int spdBase{10};
    bool protectionUtilisee;
    int gold = 0;
    // Effets rudimentaires
    enum class EffectType { None, DOT, BUFF_DEF };
    struct EffectDef { EffectType type; std::string label; int value; int duration; };
    struct ActiveEffect { EffectType type; std::string label; int value; int remaining; };
    std::vector<ActiveEffect> activeEffects;
    std::unordered_map<std::string, EffectDef> itemEffects;

    std::vector<std::pair<std::string, int>> inventaireArmes;
    std::vector<std::pair<std::string, int>> inventaireArmures;
    std::vector<Competence> competences;
    std::vector<std::pair<std::string, int>> equipements;


public:
    // Effets exposes pour l'exterieur
    enum class ItemEffectType { None, DOT, BUFF_DEF };
    Entite(const std::string& nom, int pv, int pm, int def)
        : nom(nom), PV(pv), PVmax(pv), PM(pm), PMmax(pm), defenseBase(def), defenseBoost(0),
          protectionUtilisee(false) {
        // Initialisation des emplacements d'équipement
        equipements.resize(2, {"Aucun", 0}); // Index 0: Arme, Index 1: Armure
    }

    virtual ~Entite() {}

    const std::string& getNom() const { return nom; }
    void setIsPlayer(bool v) { isPlayer = v; }
    std::string displayName() const { return ui::color(nom, isPlayer ? ui::GREEN : ui::MAGENTA); }
    int getPV() const { return PV; }
    int getPVmax() const { return PVmax; }
    int getAtkBase() const { return atkBase; }
    int getSpdBase() const { return spdBase; }
    void setAtkBase(int v) { atkBase = std::max(0, v); }
    void setSpdBase(int v) { spdBase = std::max(0, v); }
    

    virtual void defendre(int degats) {
        int buffDef = 0;
        for (const auto& e : activeEffects) if (e.type == EffectType::BUFF_DEF) buffDef += e.value;
        int passive = 0; {
            auto itEff = itemEffects.find(equipements[1].first);
            if (itEff != itemEffects.end() && itEff->second.type == EffectType::BUFF_DEF && itEff->second.duration == 0) passive = itEff->second.value;
        }
        int defenseTotale = defenseBase + equipements[1].second + defenseBoost + passive + buffDef;
        int degatsReduits = std::max(degats - defenseTotale, 0);
        PV -= degatsReduits;
        if (PV < 0) PV = 0;
        {
            std::ostringstream oss; oss << displayName() << " subit " << degatsReduits << " dégâts après réduction par la défense.\n";
            ui::typewrite_words(oss.str(), 28, 110);
        }
    }

    virtual void afficherStats(bool colorName = false, const char* colorCode = ui::YELLOW) const {
        std::string shownName = colorName ? ui::color(nom, colorCode) : (isPlayer ? ui::color(nom, ui::GREEN) : ui::color(nom, ui::MAGENTA));
        std::ostringstream oss;
        oss << "\n~~~~~~~~~ Stats : " << shownName << " ~~~~~~~~~\n"
            << "PV: " << PV << "/" << PVmax << "\n"
            << "Mana: " << PM << "/" << PMmax << "\n"
            << "Défense: " << defenseBase << "\n"
            << "ATK: " << atkBase << "\n"
            << "SPD: " << spdBase << "\n"
            << "Or: " << gold << "\n";
        if (protectionUtilisee) {
            oss << "Boost de défense: " << defenseBoost << "\n";
        }
        oss << "Arme: " << equipements[0].first << " (+ " << equipements[0].second << ")\n"
            << "Armure: " << equipements[1].first << " (+ " << equipements[1].second << ")\n\n";
        ui::typewrite_words(oss.str(), 28, 110);
    }

    

    // Or / Economie
    void ajouterOr(int delta) {
        gold += delta;
        if (gold < 0) gold = 0;
        if (delta > 0) {
            ui::typewrite_words("+" + std::to_string(delta) + " or obtenu.\n", 28, 110);
        } else if (delta < 0) {
            ui::typewrite_words(std::to_string(delta) + " or depense.\n", 28, 110);
        }
    }
    int getOr() const { return gold; }

// Competences

    void ajouterCompetence(const Competence& competence) {
        competences.push_back(competence);
    }

    

    void afficherCompetences() const {
        for (size_t i = 0; i < competences.size(); ++i) {
            std::cout << i + 1 << ". ";
            competences[i].afficher();
        }
    }

//Equipement

    void ajouterArme(const std::string& nom, int valeur) {
        inventaireArmes.emplace_back(nom, valeur);
        std::cout << "Arme ajoutée : " << nom << " (+ " << valeur << ").\n";
    }

    void ajouterArmure(const std::string& nom, int valeur) {
        inventaireArmures.emplace_back(nom, valeur);
        std::cout << "Armure ajoutée : " << nom << " (+ " << valeur << ").\n";
    }

#if 0 // removed unused function
    void enleverArme(const std::string& nom) {
        auto it = std::find_if(inventaireArmes.begin(), inventaireArmes.end(),
            [&nom](const std::pair<std::string, int>& arme) {
                return arme.first == nom;
            });
        
        if (it != inventaireArmes.end()) {
            inventaireArmes.erase(it);
            std::cout << "Arme " << nom << " a été retirée de l'inventaire.\n";
        } else {
            std::cout << "Arme " << nom << " non trouvée dans l'inventaire.\n";
        }
    }
#endif

#if 0 // removed unused function
    void enleverArmure(const std::string& nom) {
        auto it = std::find_if(inventaireArmures.begin(), inventaireArmures.end(),
            [&nom](const std::pair<std::string, int>& armure) {
                return armure.first == nom;
            });
        
        if (it != inventaireArmures.end()) {
            inventaireArmures.erase(it);
            std::cout << "Armure " << nom << " a été retirée de l'inventaire.\n";
        } else {
            std::cout << "Armure " << nom << " non trouvée dans l'inventaire.\n";
        }
    }
#endif

    void equiperArme(const std::string& nom, int valeur) {
        equipements[0] = {nom, valeur};
        std::cout << "Arme équipée : " << nom << " (+ " << valeur << ").\n";
    }

    void equiperArmure(const std::string& nom, int valeur) {
        equipements[1] = {nom, valeur};
        std::cout << "Armure équipée : " << nom << " (+ " << valeur << ").\n";
    }

    void afficherInventaire() const {
        std::cout << "\n--- Inventaire ---\n";
        std::cout << "Armes :\n";
        for (size_t i = 0; i < inventaireArmes.size(); ++i) {
            std::cout << i + 1 << ". " << inventaireArmes[i].first << " (+ " << inventaireArmes[i].second << ")\n";
        }
        std::cout << "Armures :\n";
        for (size_t i = 0; i < inventaireArmures.size(); ++i) {
            std::cout << i + 1 << ". " << inventaireArmures[i].first << " (+ " << inventaireArmures[i].second << ")\n";
        }
        std::cout << "\n";
    }

    void equiperObjet() {
        afficherInventaire();
        std::cout << "Choisissez une catégorie :\n1. Armes\n2. Armures\n";
        int categorie;
        std::cin >> categorie;

        if (categorie == 1 && !inventaireArmes.empty()) {
            std::cout << "Choisissez une arme à équiper : ";
            int index;
            std::cin >> index;
            if (index > 0 && index <= static_cast<int>(inventaireArmes.size())) {
                equiperArme(inventaireArmes[index - 1].first, inventaireArmes[index - 1].second);
                std::cout << "Vous avez équipé l'arme " << inventaireArmes[index - 1].first << ".\n";
            } else {
                std::cout << "Choix invalide.\n";
            }
        } else if (categorie == 2 && !inventaireArmures.empty()) {
            std::cout << "Choisissez une armure à équiper : ";
            int index;
            std::cin >> index;
            if (index > 0 && index <= static_cast<int>(inventaireArmures.size())) {
                equiperArmure(inventaireArmures[index - 1].first, inventaireArmures[index - 1].second);
                std::cout << "Vous avez équipé l'armure " << inventaireArmures[index - 1].first << ".\n";
            } else {
                std::cout << "Choix invalide.\n";
            }
        } else {
            std::cout << "Choix invalide ou inventaire vide.\n";
        }
    }

// Action

    void attaquer(Entite& cible, char typeValeur, int valeur) {
        int degats = (typeValeur == '%') ? cible.PVmax * valeur / 100 : valeur;
        degats += equipements[0].second; // Bonus d'attaque de l'arme
        {
            std::ostringstream oss; oss << displayName() << " inflige " << degats << " dégâts à " << cible.displayName() << " !\n";
            ui::typewrite_words(oss.str(), 28, 110);
        }
        cible.defendre(degats);
        // Effet d'arme: appliquer DOT si défini pour l'arme équipée
        auto __itEffW = itemEffects.find(equipements[0].first);
        if (__itEffW != itemEffects.end() && __itEffW->second.type == EffectType::DOT && __itEffW->second.value > 0 && __itEffW->second.duration > 0) {
            cible.activeEffects.push_back({EffectType::DOT, __itEffW->second.label, __itEffW->second.value, __itEffW->second.duration});
            if (!__itEffW->second.label.empty()) {
                std::ostringstream oss; oss << cible.displayName() << " est affecté par " << __itEffW->second.label
                          << " (" << __itEffW->second.value << ", " << __itEffW->second.duration << " tours).\n";
                ui::typewrite_words(oss.str(), 28, 110);
            }
        }
    }

    void soigner(char typeValeur, int valeur) {
        int soin = (typeValeur == '%') ? PVmax * valeur / 100 : valeur;
        PV = std::min(PV + soin, PVmax);
        {
            std::ostringstream oss; oss << displayName() << " restaure " << soin << " PV !\n";
            ui::typewrite_words(oss.str(), 28, 110);
        }
    }

    void proteger(char typeValeur, int valeur) {
        if (!protectionUtilisee) {
            if (typeValeur == '%') {
                defenseBoost = (defenseBase + equipements[1].second) * valeur / 100;
            } else {
                defenseBoost = valeur;
            }
            protectionUtilisee = true;
            {
                std::ostringstream oss; oss << displayName() << " augmente sa défense de " << defenseBoost << " !\n";
                ui::typewrite_words(oss.str(), 28, 110);
            }
        } else {
            ui::typewrite_words("Protection déjà utilisée dans ce combat !\n", 28, 110);
        }
    }

    void reinitialiserProtection() {
        protectionUtilisee = false;
        defenseBoost = 0;             // Réinitialise le bonus de protection
    }

    bool utiliserCompetence(int index, Entite& cible) {
        if (index < 0 || index >= static_cast<int>(competences.size())) return false;

        const Competence& comp = competences[index];
        if (PM < comp.getCoutMana()) {
            ui::typewrite_words("Pas assez de mana pour utiliser " + comp.getNom() + " !\n", 28, 110);
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

    size_t getNombreCompetences() const { return competences.size(); }
    bool estVivant() const { return PV > 0; }

    // Effets: enregistrement et tick
    void registerItemEffect(const std::string& itemName, const std::string& effName, int value, int duration) {
        EffectType t = EffectType::None;
        if (effName == "DOT") t = EffectType::DOT;
        else if (effName == "BUFF_DEF") t = EffectType::BUFF_DEF;
        itemEffects[itemName] = {t, effName, value, duration};
    }
    void registerItemEffect(const std::string& itemName, const std::string& effName, const std::string& displayName, int value, int duration) {
        EffectType t = EffectType::None;
        if (effName == "DOT") t = EffectType::DOT;
        else if (effName == "BUFF_DEF") t = EffectType::BUFF_DEF;
        itemEffects[itemName] = {t, displayName, value, duration};
    }
    void registerItemEffect(const std::string& itemName, ItemEffectType eff, int value, int duration) {
        EffectType t = (eff == ItemEffectType::DOT ? EffectType::DOT : (eff == ItemEffectType::BUFF_DEF ? EffectType::BUFF_DEF : EffectType::None));
        itemEffects[itemName] = {t, std::string(), value, duration};
    }
    void registerItemEffect(const std::string& itemName, ItemEffectType eff, const std::string& label, int value, int duration) {
        EffectType t = (eff == ItemEffectType::DOT ? EffectType::DOT : (eff == ItemEffectType::BUFF_DEF ? EffectType::BUFF_DEF : EffectType::None));
        itemEffects[itemName] = {t, label, value, duration};
    }
    void tickEffects() {
        for (auto& e : activeEffects) {
            if (e.remaining > 0) {
                if (e.type == EffectType::DOT) {
                    PV -= e.value;
                    if (PV < 0) PV = 0;
                    {
                        std::ostringstream oss; oss << displayName() << " subit " << e.value << " dégâts" << (e.label.empty()?"":(" ("+e.label+")")) << ".\n";
                        ui::typewrite_words(oss.str(), 28, 110);
                    }
                }
                e.remaining -= 1;
            }
        }
        activeEffects.erase(std::remove_if(activeEffects.begin(), activeEffects.end(), [](const ActiveEffect& e){ return e.remaining <= 0; }), activeEffects.end());
    }
};

#endif // ENTITE_H
