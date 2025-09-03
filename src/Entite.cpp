#include "Entite.h"
#include <iostream>
#include <limits>

Entite::Entite(const std::string& nom, int pv, int pm, int def)
    : nom(nom), PV(pv), PVmax(pv), PM(pm), PMmax(pm), defenseBase(def), defenseBoost(0),
      protectionUtilisee(false) {
    // Index 0: Arme, Index 1: Armure
    equipements.resize(2, Objet(ItemType::None, "Aucun", 0));
}

void Entite::defendre(int degats) {
    int defenseTotale = defenseBase + equipements[1].getValeur() + defenseBoost;
    int degatsReduits = std::max(degats - defenseTotale, 0);
    PV -= degatsReduits;
    if (PV < 0) PV = 0;
    std::cout << nom << " subit " << degatsReduits << " dégâts après réduction par la défense.\n";
}

void Entite::afficherStats() const {
    std::cout << "\n~~~~~~~~~ Stats : " << nom << " ~~~~~~~~~\n"
              << "PV: " << PV << "/" << PVmax << "\n"
              << "Mana: " << PM << "/" << PMmax << "\n"
              << "Défense: " << defenseBase << "\n";
    if (protectionUtilisee) {
        std::cout << "Boost de défense: " << defenseBoost << "\n";
    }
    afficherEquipements();
}

void Entite::changerDefenceBase(int valeur) {
    defenseBase = valeur;
}

void Entite::ajouterCompetence(const Competence& competence) {
    competences.push_back(competence);
}

void Entite::enleverCompetence(const std::string& nom) {
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

void Entite::afficherCompetences() const {
    for (size_t i = 0; i < competences.size(); ++i) {
        std::cout << i + 1 << ". ";
        competences[i].afficher();
    }
}

void Entite::ajouterObjet(const Objet& objet) {
    inventaire.push_back(objet);
    std::cout << toString(objet.getType()) << " ajoutée : " << objet.getNom() << " (+ " << objet.getValeur() << ").\n";
}

void Entite::enleverObjet(const std::string& nom, ItemType type) {
    auto it = std::find_if(inventaire.begin(), inventaire.end(),
        [&nom, &type](const Objet& obj) {
            return obj.getNom() == nom && obj.getType() == type;
        });

    if (it != inventaire.end()) {
        std::cout << toString(it->getType()) << " " << it->getNom() << " a été retirée de l'inventaire.\n";
        inventaire.erase(it);
    } else {
        std::cout << toString(type) << " " << nom << " non trouvée dans l'inventaire.\n";
    }
}

void Entite::equiperObjet() {
    if (inventaire.empty()) {
        std::cout << "Votre inventaire est vide.\n";
        return;
    }

    // Types disponibles dans l'inventaire
    std::vector<ItemType> typesDisponibles;
    auto pushUnique = [&typesDisponibles](ItemType t) {
        if (t == ItemType::None) return;
        if (std::find(typesDisponibles.begin(), typesDisponibles.end(), t) == typesDisponibles.end())
            typesDisponibles.push_back(t);
    };
    for (const auto& objet : inventaire) pushUnique(objet.getType());

    if (typesDisponibles.empty()) {
        std::cout << "Aucun objet équipable.\n";
        return;
    }

    std::cout << "\nChoisissez un type d'objet à équiper :\n";
    for (size_t i = 0; i < typesDisponibles.size(); ++i) {
        std::cout << i + 1 << ". " << toString(typesDisponibles[i]) << "\n";
    }
    std::cout << typesDisponibles.size() + 1 << ". Retour";

    int choixType;
    std::cout << "\nVotre choix:";
    std::cin >> choixType;
    if (choixType == static_cast<int>(typesDisponibles.size()) + 1) {
        std::cout << "Action annulée.\n";
        return;
    } else if (choixType <= 0 || choixType > static_cast<int>(typesDisponibles.size())) {
        std::cout << "Choix invalide.\n";
        return;
    }

    ItemType typeChoisi = typesDisponibles[choixType - 1];

    // Filtrer les objets par type choisi
    std::vector<Objet> objetsFiltres;
    for (const auto& objet : inventaire) {
        if (objet.getType() == typeChoisi) {
            objetsFiltres.push_back(objet);
        }
    }

    std::cout << "\nActuellement équipé (" << toString(typeChoisi) << "): ";
    const Objet& actuel = (typeChoisi == ItemType::Weapon) ? equipements[0] : equipements[1];
    actuel.afficher();

    // Afficher les objets du type choisi
    std::cout << "\nObjets disponibles dans la catégorie " << toString(typeChoisi) << " :\n";
    for (size_t i = 0; i < objetsFiltres.size(); ++i) {
        std::cout << i + 1 << ". ";
        objetsFiltres[i].afficher();
    }

    std::cout << "Choisissez un objet à équiper (par numéro) : ";
    int choixObjet;
    std::cin >> choixObjet;

    if (choixObjet <= 0 || choixObjet > static_cast<int>(objetsFiltres.size())) {
        std::cout << "Choix invalide.\n";
        return;
    }

    // Équiper l'objet choisi
    const Objet& objetChoisi = objetsFiltres[choixObjet - 1];
    if (typeChoisi == ItemType::Weapon) {
        equipements[0] = objetChoisi;
    } else if (typeChoisi == ItemType::Armor) {
        equipements[1] = objetChoisi;
    } else {
        equipements.push_back(objetChoisi);
    }

    std::cout << toString(typeChoisi) << " équipée : " << objetChoisi.getNom() << " (+ " << objetChoisi.getValeur() << ").\n";
}

void Entite::afficherEquipements() const {
    std::cout << "    ~~~~ Équipements Actuels ~~~~\n";
    for (size_t i = 0; i < equipements.size(); ++i) {
        if (equipements[i].getType() != ItemType::None){
            std::cout << i + 1 << ". ";
            equipements[i].afficher();
        }
    }
    std::cout << "\n";
}

void Entite::attaquer(Entite& cible, char typeValeur, int valeur) {
    int degats = (typeValeur == '%') ? cible.PVmax * valeur / 100 : valeur;
    degats += equipements[0].getValeur(); // Bonus d'attaque de l'arme
    std::cout << nom << " inflige " << degats << " dégâts à " << cible.getNom() << " !\n";
    cible.defendre(degats);
}

void Entite::soigner(char typeValeur, int valeur) {
    int soin = (typeValeur == '%') ? PVmax * valeur / 100 : valeur;
    PV = std::min(PV + soin, PVmax);
    std::cout << nom << " restaure " << soin << " PV !\n";
}

void Entite::proteger(char typeValeur, int valeur) {
    if (!protectionUtilisee) {
        if (typeValeur == '%') {
            defenseBoost = (defenseBase + equipements[1].getValeur()) * valeur / 100;
        } else {
            defenseBoost = valeur;
        }
        protectionUtilisee = true;
        std::cout << nom << " augmente sa défense de " << defenseBoost << " !\n";
    } else {
        std::cout << "Protection déjà utilisée dans ce combat !\n";
    }
}

void Entite::reinitialiserProtection() {
    protectionUtilisee = false;
    defenseBoost = 0;
}

bool Entite::utiliserCompetence(int index, Entite& cible) {
    if (index < 0 || index >= static_cast<int>(competences.size())) return false;

    const Competence& comp = competences[index];
    if (PM < comp.getCoutMana()) {
        std::cout << "Pas assez de mana pour utiliser " << comp.getNom() << " !\n";
        return false;
    }

    PM -= comp.getCoutMana();

    switch (comp.getType()) {
        case SkillType::Attack:
            attaquer(cible, comp.getTypeValeur(), comp.getValeur());
            break;
        case SkillType::Heal:
            soigner(comp.getTypeValeur(), comp.getValeur());
            break;
        case SkillType::Protection:
            proteger(comp.getTypeValeur(), comp.getValeur());
            break;
    }
    return true;
}

