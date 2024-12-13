#ifndef COMPETENCE_H
#define COMPETENCE_H

#include <string>

class Competence {
private:
    std::string nom;
    std::string type; // "Attaque", "Soin", "Protection"
    char typeValeur;  // '+' ou '%'
    int valeur;
    int coutMana;

public:
    Competence(const std::string& nom, const std::string& type, char typeValeur, int valeur, int coutMana)
        : nom(nom), type(type), typeValeur(typeValeur), valeur(valeur), coutMana(coutMana) {}

    const std::string& getNom() const { return nom; }
    const std::string& getType() const { return type; }
    char getTypeValeur() const { return typeValeur; }
    int getValeur() const { return valeur; }
    int getCoutMana() const { return coutMana; }

    void afficher() const;
};

#endif // COMPETENCE_H
