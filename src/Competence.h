#ifndef COMPETENCE_H
#define COMPETENCE_H

#include <string>

class GameUI;

enum class SkillType { Attaque, Soin, Protection };
enum class ValueType { Flat, Percent };

class Competence {
private:
    std::string nom;
    SkillType type;      // Attaque, Soin, Protection
    ValueType valType;   // Flat(+) ou Percent(%)
    int valeur;
    int coutMana;

public:
    Competence(const std::string& nom, SkillType type, ValueType valType, int valeur, int coutMana)
        : nom(nom), type(type), valType(valType), valeur(valeur), coutMana(coutMana) {}

    const std::string& getNom() const { return nom; }
    SkillType getType() const { return type; }
    ValueType getValueType() const { return valType; }
    int getValeur() const { return valeur; }
    int getCoutMana() const { return coutMana; }

    void afficher(GameUI& ui) const;
};

#endif // COMPETENCE_H
