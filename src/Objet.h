#ifndef OBJET_H
#define OBJET_H

#include <string>
#include <iostream>

enum class ItemType { None, Weapon, Armor };

inline const char* toString(ItemType t) {
    switch (t) {
        case ItemType::Weapon: return "arme";
        case ItemType::Armor: return "armure";
        default: return "aucun";
    }
}

class Objet {
private:
    ItemType type;
    std::string nom;
    int valeur;

public:
    Objet(ItemType type, const std::string& nom, int valeur)
        : type(type), nom(nom), valeur(valeur) {}

    ItemType getType() const { return type; }
    const std::string& getNom() const { return nom; }
    int getValeur() const { return valeur; }

    void afficher() const {
        std::cout << nom << "(+" << valeur << ")" << std::endl;
    }
};

#endif // OBJET_H
