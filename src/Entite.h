#ifndef ENTITE_H
#define ENTITE_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Competence.h"
#include "Objet.h"

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

    std::vector<Objet> inventaire;
    std::vector<Objet> equipements;
    std::vector<Competence> competences;


public:
    Entite(const std::string& nom, int pv, int pm, int def);
    virtual ~Entite() {}

    const std::string& getNom() const { return nom; }

    virtual void defendre(int degats);
    virtual void afficherStats() const;

    void changerDefenceBase(int valeur);

    // Compétences
    void ajouterCompetence(const Competence& competence);
    void enleverCompetence(const std::string& nom);
    void afficherCompetences() const;

    // Équipement / Inventaire
    void ajouterObjet(const Objet& objet);
    void enleverObjet(const std::string& nom, ItemType type);
    void equiperObjet();
    void afficherEquipements() const;

    // Actions
    void attaquer(Entite& cible, char typeValeur, int valeur);
    void soigner(char typeValeur, int valeur);
    void proteger(char typeValeur, int valeur);
    void reinitialiserProtection();

    bool utiliserCompetence(int index, Entite& cible);

    size_t getNombreCompetences() const { return competences.size(); }
    bool estVivant() const { return PV > 0; }
};

#endif // ENTITE_H
