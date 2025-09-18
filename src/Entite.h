#ifndef ENTITE_H
#define ENTITE_H

#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include <utility>
#include "Competence.h"
#include "GameUI.h"
#include "ui.h"

class Entite {
protected:
    std::string nom;
    bool isPlayer{false};
    GameUI* uiIface{nullptr};
    int PV;
    int PVmax;
    int PM;
    int PMmax;
    int defenseBase;
    int defenseBoost;
    int atkBase{5};
    int spdBase{10};
    bool protectionUtilisee;
    int gold{0};

    enum class EffectType { None, DOT, BUFF_DEF };
    struct EffectDef {
        EffectType type{EffectType::None};
        std::string label;
        int value{0};
        int duration{0};
    };
    struct ActiveEffect {
        EffectType type{EffectType::None};
        std::string label;
        int value{0};
        int remaining{0};
    };

    std::vector<ActiveEffect> activeEffects;
    std::unordered_map<std::string, EffectDef> itemEffects;
    std::vector<std::pair<std::string, int>> inventaireArmes;
    std::vector<std::pair<std::string, int>> inventaireArmures;
    std::vector<Competence> competences;
    std::vector<std::pair<std::string, int>> equipements;

public:
    enum class ItemEffectType { None, DOT, BUFF_DEF };

    Entite(const std::string& nom, int pv, int pm, int def);
    virtual ~Entite();

    void setUI(GameUI* ui);
    GameUI& out() const;

    const std::string& getNom() const { return nom; }
    void setIsPlayer(bool v) { isPlayer = v; }
    std::string displayName() const { return ui::color(nom, isPlayer ? ui::GREEN : ui::MAGENTA); }
    int getPV() const { return PV; }
    int getPVmax() const { return PVmax; }
    int getAtkBase() const { return atkBase; }
    int getSpdBase() const { return spdBase; }
    void setAtkBase(int v);
    void setSpdBase(int v);

    virtual void defendre(int degats);
    virtual void afficherStats(bool colorName = false, const char* colorCode = ui::YELLOW) const;

    void ajouterOr(int delta);
    int getOr() const { return gold; }

    void ajouterCompetence(const Competence& competence);
    void afficherCompetences() const;

    void ajouterArme(const std::string& nom, int valeur);
    void ajouterArmure(const std::string& nom, int valeur);
    void equiperArme(const std::string& nom, int valeur);
    void equiperArmure(const std::string& nom, int valeur);
    void afficherInventaire() const;
    void equiperObjet();

    void attaquer(Entite& cible, char typeValeur, int valeur);
    void soigner(char typeValeur, int valeur);
    void proteger(char typeValeur, int valeur);
    void reinitialiserProtection();

    bool utiliserCompetence(int index, Entite& cible);
    size_t getNombreCompetences() const { return competences.size(); }
    bool estVivant() const { return PV > 0; }

    void registerItemEffect(const std::string& itemName, const std::string& effName, int value, int duration);
    void registerItemEffect(const std::string& itemName, const std::string& effName, const std::string& displayName, int value, int duration);
    void registerItemEffect(const std::string& itemName, ItemEffectType eff, int value, int duration);
    void registerItemEffect(const std::string& itemName, ItemEffectType eff, const std::string& label, int value, int duration);
    void tickEffects();
};

#endif // ENTITE_H
