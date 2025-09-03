#ifndef ENTITE_H
#define ENTITE_H

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <optional>
#include <algorithm>
#include "Competence.h"

class Entite {
protected:
    std::string nom;
    int PV;
    int PVmax;
    int PM;
    int PMmax;
    int defenseBase;
    int defenseBoost;
    bool protectionUtilisee;

    std::vector<std::pair<std::string, int>> inventaireArmes;
    std::vector<std::pair<std::string, int>> inventaireArmures;
    std::vector<Competence> competences;
    std::vector<std::pair<std::string, int>> equipements;


public:
    Entite(const std::string& nom, int pv, int pm, int def)
        : nom(nom), PV(pv), PVmax(pv), PM(pm), PMmax(pm), defenseBase(def), defenseBoost(0),
          protectionUtilisee(false) {
        // Initialisation des emplacements d'équipement
        equipements.resize(2, {"Aucun", 0}); // Index 0: Arme, Index 1: Armure
    }

    virtual ~Entite() {}

    const std::string& getNom() const { return nom; }

    virtual void defendre(int degats) {
        int defenseTotale = defenseBase + equipements[1].second + defenseBoost;
        int degatsReduits = std::max(degats - defenseTotale, 0);
        PV -= degatsReduits;
        if (PV < 0) PV = 0;
        std::cout << nom << " subit " << degatsReduits << " dégâts après réduction par la défense.\n";
    }

    virtual void afficherStats() const {
        std::cout << "\n~~~~~~~~~ Stats : " << nom << " ~~~~~~~~~\n"
                  << "PV: " << PV << "/" << PVmax << "\n"
                  << "Mana: " << PM << "/" << PMmax << "\n"
                  << "Défense: " << defenseBase << "\n";
        if (protectionUtilisee) {
            std::cout << "Boost de défense: " << defenseBoost << "\n";
        }
        std::cout << "Arme: " << equipements[0].first << " (+ " << equipements[0].second << ")\n"
                  << "Armure: " << equipements[1].first << " (+ " << equipements[1].second << ")\n\n";
    }

    void changerDefenceBase(int valeur) {
        defenseBase = valeur;
    }

// Competences

    void ajouterCompetence(const Competence& competence) {
        competences.push_back(competence);
    }

    void enleverCompetence(const std::string& nom) {
        auto it = std::find_if(competences.begin(), competences.end(),
            [&nom](const Competence& competence) {
                return competence.getNom() == nom;
            });
        
        if (it != competences.end()) {
            competences.erase(it);
            std::cout << "Compétence " << nom << " a été retirée.\n";
        } else {
            std::cout << "Compétence " << nom << " non trouvée.\n";
        }
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
        std::cout << nom << " inflige " << degats << " dégâts à " << cible.getNom() << " !\n";
        cible.defendre(degats);
    }

    void soigner(char typeValeur, int valeur) {
        int soin = (typeValeur == '%') ? PVmax * valeur / 100 : valeur;
        PV = std::min(PV + soin, PVmax);
        std::cout << nom << " restaure " << soin << " PV !\n";
    }

    void proteger(char typeValeur, int valeur) {
        if (!protectionUtilisee) {
            if (typeValeur == '%') {
                defenseBoost = (defenseBase + equipements[1].second) * valeur / 100;
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

        const Competence& comp = competences[index];
        if (PM < comp.getCoutMana()) {
            std::cout << "Pas assez de mana pour utiliser " << comp.getNom() << " !\n";
            return false;
        }

        PM -= comp.getCoutMana();

        if (comp.getType() == "Attaque") {
            attaquer(cible, comp.getTypeValeur(), comp.getValeur());
        } else if (comp.getType() == "Soin") {
            soigner(comp.getTypeValeur(), comp.getValeur());
        } else if (comp.getType() == "Protection") {
            proteger(comp.getTypeValeur(), comp.getValeur());
        }
        return true;
    }

    size_t getNombreCompetences() const { return competences.size(); }
    bool estVivant() const { return PV > 0; }
};

#endif // ENTITE_H
