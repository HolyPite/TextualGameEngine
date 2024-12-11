#ifndef ENTITE_H
#define ENTITE_H

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <optional>
#include <algorithm>

class Entite {
protected:
    std::string nom;
    int PV;
    int PVmax;
    int PM;
    int PMmax;
    int attaque;
    int defenseBase;
    int defenseBoost;
    bool protectionUtilisee;

    std::string arme;
    int valeurArme;
    std::string armure;
    int valeurArmure;
    std::vector<std::tuple<std::string, std::string, char, int, int>> competences;

public:
    Entite(const std::string& nom, int pv, int pm,int atk)
        : nom(nom), PV(pv), PVmax(pv), PM(pm), PMmax(pm), attaque(atk), defenseBase(0), defenseBoost(0),
          protectionUtilisee(false), arme("Aucune"), valeurArme(0), armure("Aucune"), valeurArmure(0) {}

    virtual ~Entite() {}

    const std::string& getNom() const { return nom; }

    virtual void defendre(int degats) {
        int defenseTotale = defenseBase + valeurArmure + defenseBoost;
        //std::cout << degats << defenseTotale << defenseBase << defenseBoost << valeurArmure;
        int degatsReduits = std::max(degats - defenseTotale, 0);
        PV -= degatsReduits;
        if (PV < 0) PV = 0;
        std::cout << nom << " subit " << degatsReduits << " dégâts après réduction par la défense.\n";
    }

    virtual void afficherStats() const {
        std::string enluminure = "~~~~~~~~~~~ Stats ~~~~~~~~~~~";

        std::cout << "\n" // Ligne vide pour l'espace
                << enluminure << "\n"
                << "Nom: " << nom << "\n"
                << "PV: " << PV << "/" << PVmax << "\n"
                << "Mana: " << PM << "/" << PMmax << "\n"
                << "Defence: " << defenseBase << "\n";
        if (protectionUtilisee){
            std::cout << "Boost de defence: " << defenseBoost << "\n";
        }
        std::cout << "Arme: " << arme << " (+ " << valeurArme << ")\n"
                << "Armure: " << armure << " (+ " << valeurArmure << ")\n"
                << enluminure << "\n\n";
    }

    void changerArme(const std::string& nomArme, int valeur) {
        arme = nomArme;
        valeurArme = valeur;
    }

    void changerDefenceBase(int valeur) {
        defenseBase = valeur;
    }
    
    void changerArmure(const std::string& nomArmure, int valeur) {
        armure = nomArmure;
        valeurArmure = valeur;
    }

    void ajouterCompetence(const std::string& nom, const std::string& type, char typeValeur, int valeur, int coutMana) {
        competences.emplace_back(nom, type, typeValeur, valeur, coutMana);
    }

    void afficherCompetences() const {
        for (size_t i = 0; i < competences.size(); ++i) {
            const auto& [nom, type, typeValeur, valeur, coutMana] = competences[i];
            std::cout << i + 1 << ". " << nom << " (" << type << "):\n";
            if (type == "Attaque") {
                std::cout << "    Dégât: " << (typeValeur == '+' ? "+" : "") << valeur << (typeValeur == '%' ? "%" : "") << "\n";
            } else if (type == "Protection") {
                std::cout << "    Bouclier: " << (typeValeur == '+' ? "+" : "") << valeur << (typeValeur == '%' ? "%" : "") << "\n";
            } else if (type == "Soin") {
                std::cout << "    Soin: " << (typeValeur == '+' ? "+" : "") << valeur << (typeValeur == '%' ? "%" : "") << "\n";
            }
            std::cout << "    Mana: " << coutMana << "\n";
        }
    }

    void attaquer(Entite& cible, char typeValeur, int valeur) {
        int degats = (typeValeur == '%') ? cible.PVmax * valeur / 100 : valeur;
        degats += valeurArme;
        std::cout << nom << " inflige " << degats << " dégâts à " << cible.getNom() << " !\n";
        cible.defendre(degats);
    }

    void soigner(char typeValeur, int valeur) {
        int soin = (typeValeur == '%') ? PV * valeur / 100 : valeur;
        PV = std::min(PV + soin, PVmax);
        std::cout << nom << " restaure " << soin << " PV !\n";
    }

    void proteger(char typeValeur, int valeur) {
        if (!protectionUtilisee) {
            if (typeValeur == '%') {
                defenseBoost = (defenseBase + valeurArmure) * valeur / 100;
            } else {
                defenseBoost = valeur;
            }
            protectionUtilisee = true;
            std::cout << nom << " augmente sa défense de " << defenseBoost << " !\n";
        } else {
            std::cout << "Protection déjà utilisée dans ce combat !\n";
        }
    }

    void reinitialiserProtection() {
        protectionUtilisee = false;
        defenseBoost = 0;             // Réinitialise le bonus de protection
    }

    bool utiliserCompetence(int index, Entite& cible) {
        if (index < 0 || index >= static_cast<int>(competences.size())) return false;

        auto& [nom, type, typeValeur, valeur, coutMana] = competences[index];
        if (PM < coutMana) {
            std::cout << "Pas assez de mana pour utiliser " << nom << " !\n";
            return false;
        } else if (type == "Protection" && protectionUtilisee) {
            std::cout << "Vous avez déjà utilisé une compétence de protection pour ce combat.\n";
            return false;
        }

        PM -= coutMana;

        if (type == "Attaque") {
            attaquer(cible, typeValeur, valeur);
        } else if (type == "Soin") {
            soigner(typeValeur, valeur);
        } else if (type == "Protection") {
            proteger(typeValeur, valeur);
        }
        return true;
    }

    size_t getNombreCompetences() const { return competences.size(); }
    bool estVivant() const { return PV > 0; }
};

#endif // ENTITE_H
