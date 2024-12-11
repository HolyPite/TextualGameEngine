#include "Histoire.h"
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>

void Histoire::jouer() {
    while (!sceneActuelle.empty()) {
        chargerScene();
    }

    std::cout << "Appuyez sur une touche pour fermer le jeu..." << std::endl;
    std::cin.ignore();
    std::cin.get();
    std::cout << "Fin du jeu." << std::endl;
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
            int pv, attaque;
            std::getline(fichier, nom);
            fichier >> pv >> attaque;
            fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << description << std::endl;
            gererCombat(nom, pv, attaque);
            description.clear();
        } else if (ligne.find("*ARME*") != std::string::npos || ligne.find("*ARMURE*") != std::string::npos) {
            std::string type = ligne.find("*ARME*") != std::string::npos ? "arme" : "armure";
            std::string nom;
            int valeur;
            std::getline(fichier, nom);
            fichier >> valeur;
            fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << description << std::endl;
            gererEquipement(type, nom, valeur);
            description.clear();
        } else if (ligne.find("*PATH*") != std::string::npos) {
            if (!description.empty()) {
                std::cout << description << std::endl;
                description.clear();
            }

            int id;
            std::string chemin;
            while (fichier >> id) {
                std::getline(fichier, chemin);
                chemin = chemin.empty() ? "Continuer" : chemin.substr(1);
                chemins.emplace_back(id, chemin);
            }

            for (size_t i = 0; i < chemins.size(); ++i) {
                std::cout << i + 1 << ". " << chemins[i].second << std::endl;
            }

            while (true) {
                std::cout << "Choisissez une option ou appuyez sur 'I' pour afficher les statistiques : ";
                std::string input;
                std::cin >> input;

                if (input == "I" || input == "i") {
                    hero->afficherStats();
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
        } else if (ligne.find("*GO*") != std::string::npos) {
            std::cout << description << std::endl;
            std::cout << "C'est la fin de votre aventure...\n";
            sceneActuelle = "";
            return;
        } else if (ligne.find("*VICTOIRE*") != std::string::npos) {
            std::cout << description << std::endl;
            std::cout << "Felicitations, vous avez terminé l'aventure avec succès !\n";
            sceneActuelle = "";
            return;
        } else {
            description += ligne + "\n";
        }
    }

    if (!description.empty()) {
        std::cout << description << std::endl;
    }
}

void Histoire::gererCombat(const std::string& nomMonstre, int pv, int attaque) {
    Entite monstre(nomMonstre, pv, 0, attaque);
    std::cout << "Un combat commence contre " << nomMonstre << " !\n";

    while (hero->estVivant() && monstre.estVivant()) {
        std::cout << "C'est votre tour !\n";
        hero->afficherStats();

        bool actionValide = false; // Permet de répéter jusqu'à une action valide
        while (!actionValide) {
            std::cout << "Choisissez une compétence :\n";
            hero->afficherCompetences();
            int choix;
            std::cin >> choix;

            if (choix > 0 && choix <= hero->getNombreCompetences()) {
                hero->utiliserCompetence(choix - 1, monstre);
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
}

void Histoire::gererEquipement(const std::string& type, const std::string& nom, int valeur) {
    std::cout << "Vous trouvez une " << type << " : " << nom << " (+ " << valeur << ").\n";
    if (type == "arme") {
        hero->changerArme(nom, valeur);
    } else if (type == "armure") {
        hero->changerArmure(nom, valeur);
    }
}
