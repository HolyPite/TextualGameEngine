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

    std::string arme;
    int valeurArme;
    std::string armure;
    int valeurArmure;
    std::vector<Competence> competences;

public:
    Entite(const std::string& nom, int pv, int pm,int def)
        : nom(nom), PV(pv), PVmax(pv), PM(pm), PMmax(pm), defenseBase(def), defenseBoost(0),
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
        std::cout << "\n" // Ligne vide pour l'espace
                << "~~~~~~~~~ Stats : "<< nom <<" ~~~~~~~~~" << "\n"
                << "PV: " << PV << "/" << PVmax << "\n";
        if (!(PMmax == 0)){
            std::cout << "Mana: " << PM << "/" << PMmax << "\n";
        }
        if (!(defenseBase == 0)){
            std::cout << "Defence: " << defenseBase << "\n";
        }
        if (protectionUtilisee){
            std::cout << "Boost de defence: " << defenseBoost << "\n";
        }
        if (!(valeurArme == 0)){
            std::cout << "Arme: " << arme << " (+ " << valeurArme << ")\n";
        }
        if (!(valeurArmure == 0)){
            std::cout << "Armure: " << armure << " (+ " << valeurArmure << ")\n";
        }
        std::cout <<"\n\n";
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

    void ajouterCompetence(const Competence& competence) {
        competences.push_back(competence);
    }

    void afficherCompetences() const {
        for (size_t i = 0; i < competences.size(); ++i) {
            std::cout << i + 1 << ". ";
            competences[i].afficher();
        }
    }

    void attaquer(Entite& cible, char typeValeur, int valeur) {
        int degats = (typeValeur == '%') ? cible.PVmax * valeur / 100 : valeur;
        degats += valeurArme;
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
