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
#include "SceneWorld.h"
#include "ui.h"
#include "GameUI.h"

namespace fs = std::filesystem;

int main(int argc, char** argv) {
#ifdef _WIN32
    // Set Windows console to UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
#endif
    try { std::locale::global(std::locale("")); } catch (...) {}
    ui::init();

    ConsoleGameUI consoleUI;
    setActiveGameUI(&consoleUI);
    GameUI& io = activeGameUI();

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
    std::filesystem::path scenesDir = dataDir / "scenes";
    scene::SceneWorld world = scene::loadSceneWorld(scenesDir);
    for (const auto& warning : world.diagnostics().warnings) {
        std::cerr << "[Scene warning] " << warning << "\n";
    }
    for (const auto& err : world.diagnostics().errors) {
        std::cerr << "[Scene error] " << err << "\n";
    }
    if (!world.diagnostics().errors.empty() && world.scenes().empty()) {
        return 1;
    }

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
    io.printLine("Bienvenue dans le jeu RPG ! Choisissez votre classe :");
    for (size_t i = 0; i < fichiersClasse.size(); ++i) {
        io.printLine(std::to_string(i + 1) + ". " + fichiersClasse[i].substr(0, fichiersClasse[i].find_last_of('.')));
    }

    int choixClasse = 0;
    std::unique_ptr<Entite> hero;

    // Boucle pour demander une saisie valide
    while (true) {
        io.print("\nVotre choix : " );
        if (!io.readInt(choixClasse)) {
            io.discardLine();
            io.printLine("Entr�e invalide. Veuillez entrer un nombre entier.");
            continue;
        }
        io.discardLine();
        if (choixClasse > 0 && choixClasse <= static_cast<int>(fichiersClasse.size())) {
            break;
        } else {
            io.printLine("Choix invalide. Veuillez choisir un nombre valide.");
        }
    }

    // Charge la classe sélectionnée
    try {
        hero = chargerClasse((classesDir / fichiersClasse[choixClasse - 1]).string());
        if (hero) {
            hero->setIsPlayer(true);
            hero->setUI(&io);
        }
    } catch (const std::exception& e) {
        std::cerr << "Erreur lors du chargement de la classe : " << e.what() << "\n";
        return 1;
    }

    // Optional start scene via CLI: --start=KEY (file stem)
    std::string startKey = "start";
    for (int i=1;i<argc;i++) {
        std::string arg = argv[i];
        const std::string pre = "--start=";
        if (arg.rfind(pre,0)==0) { startKey = arg.substr(pre.size()); }
    }

    if (!world.getScene(startKey)) {
        std::cerr << "[Scene error] Scene de depart introuvable: " << startKey << "\n";
        return 1;
    }
    Histoire histoire(std::move(hero), world, dataDir, startKey);
    histoire.jouer();

    return 0;
}


