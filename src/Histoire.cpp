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
            gererCombat(nom, pv, attaque,def);
        } else if (ligne.find("*ARME*") != std::string::npos || ligne.find("*ARMURE*") != std::string::npos) {
            std::string type = ligne.find("*ARME*") != std::string::npos ? "arme" : "armure";
            std::string nom;
            int valeur;
            std::getline(fichier, nom);
            fichier >> valeur;
            fichier.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            afficherDescription(description);
            gererEquipement(type, nom, valeur);
        } else if (ligne.find("*PATH*") != std::string::npos) {
            afficherDescription(description);
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

