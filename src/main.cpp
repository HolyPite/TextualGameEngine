#include <iostream>
#include <memory>
#include <locale>
#include <filesystem>
#include <vector>
#include <string>
#ifdef _WIN32
#include <windows.h>
#endif
#include "utils.h"
#include "Histoire.h"
#include "Entite.h"
#include "ui.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
#ifdef _WIN32
    // Set Windows console to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    try { std::locale::global(std::locale("")); } catch (...) {}
    ui::init();

    std::vector<std::string> fichiersClasse;

    // Resolve data directory relative to executable
    std::filesystem::path exePath;
    try { exePath = std::filesystem::absolute(argv[0]); } catch (...) { exePath = std::filesystem::current_path() / "jeu"; }
    std::filesystem::path baseDir = exePath.parent_path();
    std::filesystem::path dataDir = baseDir / "data";
    if (!std::filesystem::exists(dataDir)) {
        // fallback to CWD/data
        if (std::filesystem::exists("data")) dataDir = std::filesystem::path("data");
    }
    std::filesystem::path classesDir = dataDir / "class";

    // Recherche des fichiers dans le dossier
    try {
        for (const auto& entry : fs::directory_iterator(classesDir)) {
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
        std::cerr << "Aucune classe disponible dans le dossier " << classesDir.string() << ".\n";
        return 1;
    }

    // Affichage des options
    std::cout << "Bienvenue dans le jeu RPG ! Choisissez votre classe :\n";
    for (size_t i = 0; i < fichiersClasse.size(); ++i) {
        std::cout << i + 1 << ". " << fichiersClasse[i].substr(0, fichiersClasse[i].find_last_of('.')) << "\n";
    }

    int choixClasse;
    std::unique_ptr<Entite> hero;

    // Boucle pour demander une saisie valide
    while (true) {
        std::cout << "\nVotre choix : ";
        if (std::cin >> choixClasse) { // Vérifie que l'entrée est un entier
            if (choixClasse > 0 && choixClasse <= static_cast<int>(fichiersClasse.size())) {
                break; // Sort de la boucle si la saisie est valide
            } else {
                std::cout << "Choix invalide. Veuillez choisir un nombre entre 1 et " << fichiersClasse.size() << ".\n";
            }
        } else {
            std::cout << "Entrée invalide. Veuillez entrer un nombre entier.\n";
            std::cin.clear(); // Efface le flag d'erreur
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n'); // Ignore la mauvaise saisie
        }
    }

    // Charge la classe sélectionnée
    try {
        hero = chargerClasse((classesDir / fichiersClasse[choixClasse - 1]).string());
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement de la classe : " << e.what() << "\n";
        return 1;
    }

    // Optional start scene via CLI: --start=N
    int startId = 0;
    for (int i=1;i<argc;i++) {
        std::string arg = argv[i];
        const std::string pre = "--start=";
        if (arg.rfind(pre,0)==0) {
            try { startId = std::stoi(arg.substr(pre.size())); } catch (...) {}
        }
    }

    Histoire histoire(std::move(hero), dataDir, startId);
    histoire.jouer();

    return 0;
}
