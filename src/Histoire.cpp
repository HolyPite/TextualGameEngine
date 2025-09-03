#include "Histoire.h"
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>
#include <cctype>
#include <limits>

void Histoire::jouer() {
    // Init a temp cache for removals (optional debug)
    try {
        auto tmpdir = std::filesystem::temp_directory_path();
        cachePath = tmpdir / "jeu_cpp_runtime.cache";
        std::ofstream(cachePath).close();
    } catch (...) {}

    while (!sceneActuelle.empty()) {
        chargerScene();
    }

    std::cout << "Appuyez sur une touche pour fermer le jeu..." << std::endl;
    std::cin.ignore();
    std::cin.get();
    if (!cachePath.empty()) {
        std::error_code ec; std::filesystem::remove(cachePath, ec);
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
    std::string prochaineScene;

    while (std::getline(fichier, ligne)) {
        if (ligne.empty()) continue;

        if (ligne.find("*COMBAT*") != std::string::npos) {
            std::string nom;
            int pv,def,attaque;
            std::getline(fichier, nom);
            fichier >> pv >> def >> attaque;
            fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            afficherDescription(description);
            // Skip combat if removed
            int sid = extractSceneId(sceneActuelle);
            if (!isRemoved(sid, "COMBAT", nom))
                gererCombat(nom, pv, attaque,def);
        } else if (ligne.find("*ARME*") != std::string::npos || ligne.find("*ARMURE*") != std::string::npos) {
            std::string type = ligne.find("*ARME*") != std::string::npos ? "arme" : "armure";
            std::string nom;
            int valeur;
            std::getline(fichier, nom);
            fichier >> valeur;
            fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            afficherDescription(description);
            int sid = extractSceneId(sceneActuelle);
            std::string t = (type == "arme" ? "ARME" : "ARMURE");
            if (!isRemoved(sid, t, nom))
                gererEquipement(type, nom, valeur);
        } else if (ligne.find("*PATH*") != std::string::npos) {
            afficherDescription(description);
            // Robust reading: stop at blank line or next block, without consuming it
            std::string l;
            int sid = extractSceneId(sceneActuelle);
            while (true) {
                std::streampos p_before = fichier.tellg();
                if (!std::getline(fichier, l)) break;
                if (l.empty()) break;
                if (!l.empty() && l[0] == '*') { fichier.seekg(p_before); break; }
                std::istringstream iss(l);
                int id; std::string rest;
                if (iss >> id) {
                    std::getline(iss, rest);
                    if (!rest.empty() && rest[0] == ' ') rest.erase(0,1);
                    if (rest.empty()) rest = "Continuer";
                    if (!isRemoved(sid, "PATH", std::to_string(id)))
                        chemins.emplace_back(id, rest);
                }
            }

            // Apply additions for this scene (combats/items before choices; extra paths appended)
            auto itAddCI = additions.find(sid);
            if (itAddCI != additions.end()) {
                for (const auto& c : itAddCI->second.combats) {
                    if (!isRemoved(sid, "COMBAT", c.nom))
                        gererCombat(c.nom, c.pv, c.atk, c.def);
                }
                for (const auto& it : itAddCI->second.items) {
                    std::string t = (it.type == "arme" ? "ARME" : "ARMURE");
                    if (!isRemoved(sid, t, it.nom))
                        gererEquipement(it.type, it.nom, it.valeur);
                }
                for (const auto& p : itAddCI->second.paths) {
                    if (!isRemoved(sid, "PATH", std::to_string(p.first)))
                        chemins.emplace_back(p);
                }
            }

            for (size_t i = 0; i < chemins.size(); ++i) {
                std::cout << i + 1 << ". " << chemins[i].second << std::endl;
            }

            while (true) {
                std::cout << "\nChoisissez une option('I' pour les stats): ";
                std::string input;
                std::cin >> input;

                if (input == "I" || input == "i") {
                    hero->afficherStats();
                    std::cout << "\nOptions disponibles :\n";
                    std::cout << "1. Retour au choix précédent\n";
                    std::cout << "2. Afficher l'inventaire\n";

                    std::string choix;
                    std::cin >> choix;

                    if (choix == "2") {
                        hero->equiperObjet();
                    }
                } else {
                    try {
                        int choix = std::stoi(input);
                        if (choix > 0 && choix <= static_cast<int>(chemins.size())) {
                            prochaineScene = "data/scenes/" + std::to_string(chemins[choix - 1].first) + ".txt";
                            break;
                        } else {
                            std::cout << "Choix invalide. Réessayez.\n";
                        }
                    } catch (std::invalid_argument&) {
                        std::cout << "Entrée invalide. Réessayez.\n";
                    }
                }
            }
            // Après choix, continuer à lire le fichier pour appliquer d'éventuels *REMOVE* / *ADD*
        } else if (ligne.find("*REMOVE*") != std::string::npos) {
            appliquerRemoveDirective(fichier);
        } else if (ligne.find("*ADD*") != std::string::npos) {
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
    if (!prochaineScene.empty()) {
        sceneActuelle = prochaineScene;
        return;
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

            if (choix > 0 && static_cast<size_t>(choix) <= hero->getNombreCompetences() && hero->utiliserCompetence(choix - 1, monstre)) {
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
        monstre.attaquer(*hero,'%',attaque);

        if (!hero->estVivant()) {
            std::cout << "Vous avez été vaincu... Fin du jeu.\n";
            sceneActuelle = "";
        }
    }

    hero->reinitialiserProtection();
}

void Histoire::gererEquipement(const std::string& type, const std::string& nom, int valeur) {
    if (type == "arme") {
        hero->ajouterArme(nom, valeur);
    } else if (type == "armure") {
        hero->ajouterArmure(nom, valeur);
    }
}

// ======= Directives REMOVE/ADD =======
bool Histoire::isRemoved(int sceneId, const std::string& type, const std::string& param) const {
    std::string key = std::to_string(sceneId) + ":" + type + ":" + param;
    return removals.find(key) != removals.end();
}

int Histoire::extractSceneId(const std::string& path) {
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
        if (ofs.is_open()) ofs << "REMOVE " << key << "\n";
    }
}

void Histoire::appliquerRemoveDirective(std::ifstream& fichier) {
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
        int sid = (sceneTok == "this") ? extractSceneId(sceneActuelle) : std::stoi(sceneTok);
        if (type == "PATH" || type == "COMBAT" || type == "ARME" || type == "ARMURE") {
            addRemoval(sid, type, param);
        }
    }
}

static inline std::string trim_copy(const std::string& s){ size_t a=0,b=s.size(); while(a<b && std::isspace((unsigned char)s[a])) ++a; while(b>a && std::isspace((unsigned char)s[b-1])) --b; return s.substr(a,b-a);} 

void Histoire::appliquerAddDirective(std::ifstream& fichier) {
    std::streampos pos;
    std::string l;
    while (true) {
        pos = fichier.tellg();
        if (!std::getline(fichier, l)) break;
        l = trim_copy(l);
        if (l.empty()) break;
        if (!l.empty() && l[0] == '*') { fichier.seekg(pos); break; }

        std::istringstream iss(l);
        std::string sceneTok, type;
        if (!(iss >> sceneTok >> type)) continue;
        for (auto& c : type) c = (char)std::toupper((unsigned char)c);
        int sid = (sceneTok == "this") ? extractSceneId(sceneActuelle) : std::stoi(sceneTok);

        // Read next non-empty line as header
        std::streampos pos2 = fichier.tellg();
        std::string header;
        while (std::getline(fichier, header)) { header = trim_copy(header); if (!header.empty()) break; }
        if (header.empty() || header[0] != '*') { fichier.seekg(pos2); continue; }

        if (type == "PATH" && header.find("*PATH*") != std::string::npos) {
            while (true) {
                std::streampos p3 = fichier.tellg();
                std::string line;
                if (!std::getline(fichier, line)) break;
                line = trim_copy(line);
                if (line.empty()) break;
                if (!line.empty() && line[0] == '*') { fichier.seekg(p3); break; }
                std::istringstream iss2(line);
                int id; std::string rest;
                if (iss2 >> id) {
                    std::getline(iss2, rest);
                    if (!rest.empty() && rest[0] == ' ') rest.erase(0,1);
                    if (rest.empty()) rest = "Continuer";
                    additions[sid].paths.emplace_back(id, rest);
                }
            }
        } else if (type == "COMBAT" && header.find("*COMBAT*") != std::string::npos) {
            std::string nom; int pv=0, def=0, atk=0;
            std::getline(fichier, nom);
            fichier >> pv; fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            fichier >> def; fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            fichier >> atk; fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            additions[sid].combats.push_back({trim_copy(nom), pv, def, atk});
        } else if ((type == "ARME" && header.find("*ARME*") != std::string::npos) || (type == "ARMURE" && header.find("*ARMURE*") != std::string::npos)) {
            std::string nom; int val=0;
            std::getline(fichier, nom);
            fichier >> val; fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::string t = (type == "ARME") ? std::string("arme") : std::string("armure");
            additions[sid].items.push_back({t, trim_copy(nom), val});
        } else {
            // Unknown or mismatched header; rewind to pos2 to not lose blocks
            fichier.seekg(pos2);
        }
    }
}

