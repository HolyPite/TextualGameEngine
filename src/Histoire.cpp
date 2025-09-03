#include "Histoire.h"
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>
#include "ui.h"
#include <unordered_set>
#include <fstream>
#include <filesystem>
#include <regex>

void Histoire::jouer() {
    // Init a temp cache for removals/additions (debug only)
    try {
        auto tmpdir = std::filesystem::temp_directory_path();
        cachePath = tmpdir / "jeu_cpp_runtime.cache";
        // Truncate any existing
        std::ofstream(cachePath).close();
    } catch (...) {
        // ignore errors; closures remain in-memory only
    }
    while (!sceneActuelle.empty()) {
        chargerScene();
    }

    ui::pause("Appuyez sur une touche pour fermer le jeu...");
    // Cleanup temp cache file
    if (!cachePath.empty()) {
        std::error_code ec;
        std::filesystem::remove(cachePath, ec);
    }
    std::cout << "Fin du jeu." << std::endl;
}

void Histoire::afficherDescription(std::string& description) {
    if (!description.empty()) {
        std::cout << "\n\n******************************\n\n" << description << std::endl;
        description.clear(); // Efface la description après affichage
    }
}


void Histoire::chargerScene() {
    std::ifstream fichier(sceneActuelle);
    if (!fichier.is_open()) {
        std::cerr << "Impossible de charger la scène : " << sceneActuelle << std::endl;
        sceneActuelle = "";
        return;
    }

    std::string ligne;
    std::string description;
    std::vector<std::pair<int, std::string>> chemins;

    while (std::getline(fichier, ligne)) {
        if (ligne.empty()) continue;

        if (ligne.find("*COMBAT*") != std::string::npos) {
            std::string nom;
            int pv,def,attaque;
            std::getline(fichier, nom);
            fichier >> pv >> def >> attaque;
            fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            afficherDescription(description);
            // Skip combat if closed
            int sid = extractSceneId(sceneActuelle);
            if (!isRemoved(sid, "COMBAT", nom))
                gererCombat(nom, pv, attaque,def);
        } else if (ligne.find("*ARME*") != std::string::npos || ligne.find("*ARMURE*") != std::string::npos) {
            ItemType type = ligne.find("*ARME*") != std::string::npos ? ItemType::Weapon : ItemType::Armor;
            std::string nom;
            int valeur;
            std::getline(fichier, nom);
            fichier >> valeur;
            fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            afficherDescription(description);
            // Skip loot if closed
            int sid = extractSceneId(sceneActuelle);
            std::string t = (type == ItemType::Weapon ? "ARME" : "ARMURE");
            if (!isRemoved(sid, t, nom))
                gererEquipement(type, nom, valeur);
        } else if (ligne.find("*PATH*") != std::string::npos) {
            afficherDescription(description);
            // Lecture robuste des chemins jusqu'à ligne vide ou prochaine balise '*'
            std::string l;
            int sid = extractSceneId(sceneActuelle);
            while (std::getline(fichier, l)) {
                if (l.empty()) break;
                if (!l.empty() && l[0] == '*') break;
                std::istringstream iss(l);
                int id; std::string rest;
                if (iss >> id) {
                    std::getline(iss, rest);
                    if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);
                    if (rest.empty()) rest = "Continuer";
                    if (!isRemoved(sid, "PATH", std::to_string(id)))
                        chemins.emplace_back(id, rest);
                }
            }
            // Apply ADDed combats/items before presenting choices
            auto itAddCI = additions.find(sid);
            if (itAddCI != additions.end()) {
                for (const auto& c : itAddCI->second.combats) {
                    if (!isRemoved(sid, "COMBAT", c.nom))
                        gererCombat(c.nom, c.pv, c.atk, c.def);
                }
                for (const auto& it : itAddCI->second.items) {
                    std::string t = (it.type == ItemType::Weapon ? "ARME" : "ARMURE");
                    if (!isRemoved(sid, t, it.nom))
                        gererEquipement(it.type, it.nom, it.valeur);
                }
            }
            // Append ADDed paths for this scene
            auto itAdd = additions.find(sid);
            if (itAdd != additions.end()) {
                for (const auto& p : itAdd->second.paths) {
                    if (!isRemoved(sid, "PATH", std::to_string(p.first)))
                        chemins.emplace_back(p);
                }
            }

            for (size_t i = 0; i < chemins.size(); ++i) {
                std::cout << i + 1 << ". " << chemins[i].second << std::endl;
            }

            while (true) {
                std::string input = ui::prompt("\nChoisissez une option('I' pour les stats): ");

                if (input == "I" || input == "i") {
                    hero->afficherStats();
                    std::cout << "\nOptions disponibles :\n";
                    std::cout << "1. Afficher l'inventaire\n";
                    std::cout << "2. Retour\n";

                    std::string choix;
                    std::cout << "Votre choix:";
                    std::cin >> choix;

                    if (choix == "1") {
                        hero->equiperObjet();
                    }
                } else {
                    try {
                        int choix = std::stoi(input);
                        if (choix > 0 && choix <= static_cast<int>(chemins.size())) {
                            sceneActuelle = "data/scenes/" + std::to_string(chemins[choix - 1].first) + ".txt";
                            return;
                        } else {
                            std::cout << "Choix invalide. Réessayez.\n";
                        }
                    } catch (std::invalid_argument&) {
                        std::cout << "Entrée invalide. Réessayez.\n";
                    }
                }
            }
        } else if (ligne.find("*REMOVE*") != std::string::npos) {
            // Parse removal lines until empty or next block
            appliquerRemoveDirective(fichier);
        } else if (ligne.find("*ADD*") != std::string::npos) {
            // Parse add lines until empty or next block
            appliquerAddDirective(fichier);
        } else if (ligne.find("*GO*") != std::string::npos) {
            afficherDescription(description);
            std::cout << "C'est la fin de votre aventure...\n";
            sceneActuelle = "";
            return;
        } else if (ligne.find("*VICTOIRE*") != std::string::npos) {
            afficherDescription(description);
            std::cout << "Felicitations, vous avez terminé l'aventure avec succès !\n";
            sceneActuelle = "";
            return;
        } else {
            description += ligne + "\n";
        }
    }
    afficherDescription(description);

    // If no PATH encountered, but we have ADDed PATHs, offer them
    int sid = extractSceneId(sceneActuelle);
    auto itAdd = additions.find(sid);
    if (!chemins.size() && itAdd != additions.end() && !itAdd->second.paths.empty()) {
        for (const auto& p : itAdd->second.paths) {
            if (!isRemoved(sid, "PATH", std::to_string(p.first)))
                chemins.emplace_back(p);
        }
        if (!chemins.empty()) {
            for (size_t i = 0; i < chemins.size(); ++i) {
                std::cout << i + 1 << ". " << chemins[i].second << std::endl;
            }
            while (true) {
                std::string input = ui::prompt("\nChoisissez une option('I' pour les stats): ");
                if (input == "I" || input == "i") {
                    hero->afficherStats();
                    std::cout << "\nOptions disponibles :\n";
                    std::cout << "1. Afficher l'inventaire\n";
                    std::cout << "2. Retour\n";
                    std::string choix;
                    std::cout << "Votre choix:";
                    std::cin >> choix;
                    if (choix == "1") hero->equiperObjet();
                } else {
                    try {
                        int choix = std::stoi(input);
                        if (choix > 0 && choix <= static_cast<int>(chemins.size())) {
                            sceneActuelle = "data/scenes/" + std::to_string(chemins[choix - 1].first) + ".txt";
                            return;
                        } else {
                            std::cout << "Choix invalide. Réessayez.\n";
                        }
                    } catch (...) {
                        std::cout << "Entrée invalide. Réessayez.\n";
                    }
                }
            }
        }
    }
}

void Histoire::gererCombat(const std::string& nomMonstre, int pv, int attaque, int defense) {
    Entite monstre(nomMonstre, pv, 0, defense);
    std::cout << "\nUn combat commence contre " << nomMonstre << " !\n";

    while (hero->estVivant() && monstre.estVivant()) {
        monstre.afficherStats();
        std::cout << "C'est votre tour !\n";
        hero->afficherStats();

        bool actionValide = false; // Permet de répéter jusqu'à une action valide
        while (!actionValide) {
            std::cout << "Choisissez une compétence :\n";
            hero->afficherCompetences();
            int choix;
            std::cout << "Compétence à utiliser:";
            std::cin >> choix;
            std::cout << "\n";

            if (choix > 0 && choix <= hero->getNombreCompetences() && hero->utiliserCompetence(choix - 1, monstre)) {
                actionValide = true; // Action valide effectuée, on sort de la boucle
            } else {
                std::cout << "Choix invalide, veuillez réessayer.\n";
            }
        }

        if (!monstre.estVivant()) {
            std::cout << "Vous avez vaincu " << nomMonstre << " !\n";
            break;
        }

        std::cout << nomMonstre << " attaque !\n";
        // Utiliser des dégâts fixes pour l'attaque du monstre
        monstre.attaquer(*hero,'+',attaque);

        if (!hero->estVivant()) {
            std::cout << "Vous avez été vaincu... Fin du jeu.\n";
            sceneActuelle = "";
        }
    }

    hero->reinitialiserProtection();
}

void Histoire::gererEquipement(ItemType type, const std::string& nom, int valeur) {
    Objet objet(type, nom, valeur);
    hero->ajouterObjet(objet);
    }

bool Histoire::isRemoved(int sceneId, const std::string& type, const std::string& param) const {
    std::string key = std::to_string(sceneId) + ":" + type + ":" + param;
    return removals.find(key) != removals.end();
}

int Histoire::extractSceneId(const std::string& path) {
    // expects something like data/scenes/123.txt
    size_t slash = path.find_last_of("/\\");
    std::string fname = (slash == std::string::npos) ? path : path.substr(slash + 1);
    size_t dot = fname.find('.');
    std::string idstr = (dot == std::string::npos) ? fname : fname.substr(0, dot);
    try { return std::stoi(idstr); } catch (...) { return -1; }
}

void Histoire::addRemoval(int sceneId, const std::string& type, const std::string& param) {
    std::string key = std::to_string(sceneId) + ":" + type + ":" + param;
    removals.insert(key);
    if (!cachePath.empty()) {
        std::ofstream ofs(cachePath, std::ios::app);
        if (ofs.is_open()) {
            ofs << "REMOVE " << key << "\n";
        }
    }
}

void Histoire::appliquerRemoveDirective(std::ifstream& fichier) {
    // Lines format: <sceneId|this> <TYPE> <param>
    // TYPE in { PATH, COMBAT, ARME, ARMURE }
    std::streampos pos;
    std::string l;
    while (true) {
        pos = fichier.tellg();
        if (!std::getline(fichier, l)) break;
        if (l.empty()) break;
        if (!l.empty() && l[0] == '*') { fichier.seekg(pos); break; }
        std::istringstream iss(l);
        std::string sceneTok, type, param;
        if (!(iss >> sceneTok >> type)) continue;
        std::getline(iss, param);
        if (!param.empty() && param[0] == ' ') param.erase(0,1);
        int sid = 0;
        if (sceneTok == "this") sid = extractSceneId(sceneActuelle);
        else { try { sid = std::stoi(sceneTok); } catch (...) { sid = extractSceneId(sceneActuelle); } }
        if (type == "PATH") {
            // param is the target scene id
            addRemoval(sid, type, param);
        } else if (type == "COMBAT" || type == "ARME" || type == "ARMURE") {
            addRemoval(sid, type, param);
        }
    }
}

void Histoire::appliquerAddDirective(std::ifstream& fichier) {
    // Entry header lines: <sceneId|this> <TYPE>
    // Followed by a declaration block matching TYPE: *PATH* ... | *COMBAT* ... | *ARME*/*ARMURE* ...
    std::regex entryStart(R"(^\s*(this|\d+)\s+(PATH|COMBAT|ARME|ARMURE)\s*$)");
    std::streampos pos;
    std::string l;
    while (true) {
        pos = fichier.tellg();
        if (!std::getline(fichier, l)) break;
        if (l.empty()) break;
        if (!l.empty() && l[0] == '*') { fichier.seekg(pos); break; }
        std::smatch m;
        if (!std::regex_match(l, m, entryStart)) continue;
        std::string sceneTok = m[1];
        std::string type = m[2];
        int sid = (sceneTok == "this") ? extractSceneId(sceneActuelle) : std::stoi(sceneTok);

        // Read block header
        std::streampos pos2 = fichier.tellg();
        std::string header;
        if (!std::getline(fichier, header)) break;
        if (header.rfind("*", 0) != 0) { fichier.seekg(pos2); continue; }

        if (type == "PATH" && header.find("*PATH*") != std::string::npos) {
            // Read lines of id + text until blank or next block
            while (true) {
                std::streampos p3 = fichier.tellg();
                std::string line;
                if (!std::getline(fichier, line)) break;
                if (line.empty()) break;
                if (!line.empty() && line[0] == '*') { fichier.seekg(p3); break; }
                std::istringstream iss(line);
                int id; std::string rest;
                if (iss >> id) {
                    std::getline(iss, rest);
                    if (!rest.empty() && rest[0] == ' ') rest.erase(0,1);
                    if (rest.empty()) rest = "Continuer";
                    additions[sid].paths.emplace_back(id, rest);
                }
            }
        } else if (type == "COMBAT" && header.find("*COMBAT*") != std::string::npos) {
            std::string nom; int pv, def, atk;
            std::getline(fichier, nom);
            fichier >> pv; fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            fichier >> def; fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            fichier >> atk; fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            additions[sid].combats.push_back({nom, pv, def, atk});
        } else if ((type == "ARME" && header.find("*ARME*") != std::string::npos) || (type == "ARMURE" && header.find("*ARMURE*") != std::string::npos)) {
            std::string nom; int val;
            std::getline(fichier, nom);
            fichier >> val; fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            ItemType it = (type == "ARME") ? ItemType::Weapon : ItemType::Armor;
            additions[sid].items.push_back({it, nom, val});
        } else {
            // Unknown or mismatched block, skip
            continue;
        }
    }
}

