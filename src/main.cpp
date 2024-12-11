#include <iostream>
#include <memory>
#include <locale>
#include <filesystem>
#include <vector>
#include "utils.h"
#include "Histoire.h"
#include "Entite.h"

namespace fs = std::filesystem;

int main() {
    std::setlocale(LC_ALL, "fr_FR.UTF-8");

    std::vector<std::string> fichiersClasse;
    const std::string dossierClasses = "data/class/";

    // Recherche des fichiers dans le dossier
    try {
        for (const auto& entry : fs::directory_iterator(dossierClasses)) {
            if (entry.is_regular_file() && entry.path().extension() == ".txt") {
                fichiersClasse.push_back(entry.path().filename().string());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors de la lecture du dossier des classes : " << e.what() << "\n";
        return 1;
    }

    // Vérifie si des fichiers de classe existent
    if (fichiersClasse.empty()) {
        std::cerr << "Aucune classe disponible dans le dossier " << dossierClasses << ".\n";
        return 1;
    }

    // Affichage des options
    std::cout << "Bienvenue dans le jeu RPG ! Choisissez votre classe :\n";
    for (size_t i = 0; i < fichiersClasse.size(); ++i) {
        std::cout << i + 1 << ". " << fichiersClasse[i].substr(0, fichiersClasse[i].find_last_of('.')) << "\n";
    }

    int choixClasse;
    std::cout << "\nVotre Choix:";
    std::cin >> choixClasse;

    std::unique_ptr<Entite> hero;

    try {
        if (choixClasse > 0 && choixClasse <= static_cast<int>(fichiersClasse.size())) {
            hero = chargerClasse(dossierClasses + fichiersClasse[choixClasse - 1]);
        } else {
            std::cout << "Choix invalide, Guerrier par défaut.\n";
            hero = chargerClasse(dossierClasses + "Guerrier.txt");
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur : " << e.what() << "\n";
        return 1;
    }

    Histoire histoire(std::move(hero));
    histoire.jouer();

    return 0;
}
