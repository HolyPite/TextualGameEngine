#include "utils.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <iostream> // Pour affichage de débogage
#include <algorithm>
#include <cctype>
#include <limits>

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

    std::string ligne, nomClasse;
    int pv = 0, pm = 0, armure = 0;
    std::vector<Competence> competences;
    bool nomClasseDefini = false; // Indique si nomClasse a été défini

    while (std::getline(fichier, ligne)) {
        ligne = trim(ligne); // Nettoyer la ligne

        if (!nomClasseDefini && !ligne.empty()) {
            // Définir nomClasse comme la première ligne non vide
            nomClasse = ligne;
            nomClasseDefini = true;
            continue; // Passer à la ligne suivante après avoir défini nomClasse
        }
        if (ligne == "PV") {
            fichier >> pv;
            fichier.ignore();
        } else if (ligne == "PM") {
            fichier >> pm;
            fichier.ignore();
        } else if (ligne == "ARM") {
            fichier >> armure;
            fichier.ignore();
        } else if (ligne == "COMP") {
            while (std::getline(fichier, ligne) && !ligne.empty()) {
                std::string nomCompetence = trim(ligne);
                std::string type;
                char typeValeur;
                int valeur, coutMana;

                // Lire les détails de la compétence
                std::getline(fichier, type);
                fichier >> typeValeur >> valeur >> coutMana;
                fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

                // Mapper type string -> enum SkillType
                std::string t = trim(type);
                SkillType st = SkillType::Attack;
                if (t == "Attaque") st = SkillType::Attack;
                else if (t == "Soin") st = SkillType::Heal;
                else if (t == "Protection") st = SkillType::Protection;
                else {
                    // défaut raisonnable
                    st = SkillType::Attack;
                }

                competences.emplace_back(nomCompetence, st, typeValeur, valeur, coutMana);
            }
        }
    }

    // Créer l'entité avec les données lues
    auto classe = std::make_unique<Entite>(nomClasse.empty() ? "Inconnu" : nomClasse, pv, pm, armure);

    // Ajouter les compétences à l'entité
    for (const auto& competence : competences) {
        classe->ajouterCompetence(competence);
    }

    return classe;
}
