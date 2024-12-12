#include "utils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <iostream> // Pour affichage de débogage
#include <algorithm>
#include <cctype>

// Supprime les espaces en début et en fin de chaîne
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

std::unique_ptr<Entite> chargerClasse(const std::string& cheminFichier) {
    std::ifstream fichier(cheminFichier);
    if (!fichier.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + cheminFichier);
    }

    std::string ligne, nom;
    int pv = 0, pm = 0, armure = 0;
    std::vector<std::tuple<std::string, std::string, char, int, int>> competences;

    while (std::getline(fichier, ligne)) {
        // Lecture des caractéristiques principales
        if (ligne.find("PV") != std::string::npos) {
            fichier >> pv;
            fichier.ignore();
        } else if (ligne.find("PM") != std::string::npos) {
            fichier >> pm;
            fichier.ignore();
        } else if (ligne.find("ARM") != std::string::npos) {
            fichier >> armure;
            fichier.ignore();
        }
        // Lecture des compétences
        else if (ligne.find("COMP") != std::string::npos) {
            while (std::getline(fichier, ligne) && !ligne.empty()) {
                std::string nomCompetence = trim(ligne); // Nettoie les espaces superflus
                std::getline(fichier, ligne);
                std::string type = trim(ligne);          // Nettoie les espaces superflus
                char typeValeur;
                int valeur, coutMana;

                fichier >> typeValeur >> valeur >> coutMana;
                fichier.ignore(); // Ignore le saut de ligne restant

                competences.emplace_back(nomCompetence, type, typeValeur, valeur, coutMana);
            }
        }
    }

    // Création de l'entité et ajout des compétences
    auto classe = std::make_unique<Entite>("Guerrier", pv, pm, armure);

    for (const auto& comp : competences) {
        classe->ajouterCompetence(
            std::get<0>(comp), // Nom
            std::get<1>(comp), // Type
            std::get<2>(comp), // TypeValeur
            std::get<3>(comp), // Valeur
            std::get<4>(comp)  // Consommation de mana
        );
    }

    // Débogage : Affichage des données lues
    //std::cout << "Classe chargée : " << "Guerrier" << "\n";
    //std::cout << "PV : " << pv << ", PM : " << pm << ", Armure : " << armure << "\n";
    //for (const auto& comp : competences) {
    //    std::cout << "Compétence : " << std::get<0>(comp) << ", Type : " << std::get<1>(comp)
    //              << ", TypeValeur : " << std::get<2>(comp) << ", Valeur : " << std::get<3>(comp)
    //              << ", Mana : " << std::get<4>(comp) << "\n";
    //}

    return classe;
}
