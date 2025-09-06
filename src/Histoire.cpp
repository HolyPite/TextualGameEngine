#include "Histoire.h"
#include "SceneParser.h"
#include <vector>
#include <thread>
#include <chrono>
#include <iostream>
#include <sstream>
#include "ui.h"
#include <iomanip>
#include <cctype>
#include <limits>
#include <random>

void Histoire::jouer() {
    while (!sceneActuelle.empty()) {
        chargerScene2();
    }

    std::cout << "Appuyez sur une touche pour fermer le jeu..." << std::endl;
    std::cin.ignore();
    std::cin.get();
    std::cout << "Fin du jeu." << std::endl;
}

// New parser-driven scene execution
void Histoire::chargerScene2() {
    auto sc = scene::parse(sceneActuelle);
    if (sc.blocks.empty()) {
        std::cerr << "Impossible de charger la scene : " << sceneActuelle << std::endl;
        sceneActuelle.clear();
        return;
    }

    std::string description;
    std::vector<std::pair<int, std::string>> chemins;
    std::string prochaineScene;
    int sid = extractSceneId(sceneActuelle);

    for (const auto& blk : sc.blocks) {
        if (std::holds_alternative<BlockDescription>(blk)) {
            description += std::get<BlockDescription>(blk).text;
        } else if (std::holds_alternative<BlockCombat>(blk)) {
            afficherDescription(description);
            const auto& b = std::get<BlockCombat>(blk);
            bool fled = false;
            for (const auto& e : b.enemies) {
                if (!isRemoved(sid, "COMBAT", e.name)) {
                    auto outcome = gererCombat(e.name, e.hp, e.atk, e.def, e.gold, e.spd, e.reveal);
                    if (outcome == CombatOutcome::Flee) {
                        // Retour à la scène précédente, ne pas exécuter la suite
                        std::string target = previousScene.empty() ? sceneActuelle : previousScene;
                        prochaineScene = target;
                        fled = true;
                        break;
                    } else if (outcome == CombatOutcome::Lose) {
                        // Défaite: sceneActuelle a été vidée dans la fonction
                        break;
                    }
                }
                if (!hero->estVivant()) break;
            }
            if (fled || !hero->estVivant()) break;
        } else if (std::holds_alternative<BlockItems>(blk)) {
            afficherDescription(description);
            const auto& b = std::get<BlockItems>(blk);
            for (const auto& it : b.items) {
                std::string typeStr = (it.type==ItemType::Arme?"ARME":"ARMURE");
                if (!isRemoved(sid, typeStr, it.name)) {
                    if (it.effect.has_value()) {
                        const auto& eff = *it.effect;
                        Entite::ItemEffectType et = (eff.kind==EffectKind::DOT?Entite::ItemEffectType::DOT:(eff.kind==EffectKind::BuffDef?Entite::ItemEffectType::BUFF_DEF:Entite::ItemEffectType::None));
                        if (!eff.label.empty()) hero->registerItemEffect(it.name, et, eff.label, eff.value, eff.duration);
                        else hero->registerItemEffect(it.name, et, eff.value, eff.duration);
                    }
                    gererEquipement(it.type==ItemType::Arme?"arme":"armure", it.name, it.value);
                }
            }
        } else if (std::holds_alternative<BlockPath>(blk)) {
            afficherDescription(description);
            auto itAddCI = additions.find(sid);
            if (itAddCI != additions.end()) {
                for (const auto& c : itAddCI->second.combats) {
                    if (!isRemoved(sid, "COMBAT", c.nom)) {
                        auto outcome = gererCombat(c.nom, c.pv, c.atk, c.def, c.orGain, c.spd, c.reveal);
                        if (outcome == CombatOutcome::Flee) {
                            std::string target = previousScene.empty() ? sceneActuelle : previousScene;
                            sceneActuelle = target; return; // revenir immédiatement
                        }
                        if (outcome == CombatOutcome::Lose) break;
                    }
                }
                for (const auto& it : itAddCI->second.items) {
                    std::string T = (it.type == "arme" ? "ARME" : "ARMURE");
                    if (!isRemoved(sid, T, it.nom)) gererEquipement(it.type, it.nom, it.valeur);
                }
                for (int gd : itAddCI->second.goldDeltas) {
                    if (!isRemoved(sid, "GOLD", "*") && !isRemoved(sid, "GOLD", std::to_string(gd)))
                        gererGoldDelta(gd);
                }
                if (!isRemoved(sid, "SHOP", "*") && !itAddCI->second.shops.empty()) {
                    std::vector<ShopItem> filtered;
                    for (const auto& sh : itAddCI->second.shops) {
                        if (!isRemoved(sid, "SHOP", sh.item.name)) filtered.push_back(sh);
                    }
                    if (!filtered.empty()) gererShop(filtered);
                }
                for (const auto& p : itAddCI->second.paths) {
                    if (!isRemoved(sid, "PATH", std::to_string(p.first))) chemins.emplace_back(p);
                }
            }
            const auto& b = std::get<BlockPath>(blk);
            for (const auto& p : b.options) {
                if (!isRemoved(sid, "PATH", std::to_string(p.id))) {
                    std::string txt = p.text.empty()?"Continuer":p.text;
                    chemins.emplace_back(p.id, txt);
                }
            }
            std::cout << ui::color("\n=== Chemins ===\n", ui::CYAN);
            for (size_t i = 0; i < chemins.size(); ++i) {
                std::cout << ui::color(std::to_string(i + 1), ui::YELLOW) << ". " << chemins[i].second << std::endl;
            }
            while (true) {
                std::cout << "\n" << ui::color("Choisissez une option ('I' pour les stats): ", ui::BOLD);
                std::string input; std::cin >> input;
                if (input == "I" || input == "i") {
                    hero->afficherStats();
                    std::cout << "\nOptions disponibles :\n";
                    std::cout << "1. Retour au choix precedent\n";
                    std::cout << "2. Afficher l'inventaire\n";
                    std::string choix; std::cin >> choix;
                    if (choix == "2") hero->equiperObjet();
                } else {
                    try {
                        int choix = std::stoi(input);
                        if (choix > 0 && choix <= static_cast<int>(chemins.size())) {
                            prochaineScene = (dataDir / "scenes" / (std::to_string(chemins[choix-1].first) + ".txt")).string();
                            break;
                        } else {
                            std::cout << ui::color("Choix invalide. Réessayez.\n", ui::RED);
                        }
                    } catch (...) {
                        std::cout << ui::color("Entrée invalide. Réessayez.\n", ui::RED);
                    }
                }
            }
        } else if (std::holds_alternative<BlockGold>(blk)) {
            int d = std::get<BlockGold>(blk).delta;
            if (!isRemoved(sid, "GOLD", "*") && !isRemoved(sid, "GOLD", std::to_string(d)))
                gererGoldDelta(d);
        } else if (std::holds_alternative<BlockShop>(blk)) {
            if (isRemoved(sid, "SHOP", "*")) {
                // Entire shop removed
            } else {
                std::vector<ShopItem> filtered;
                for (const auto& it : std::get<BlockShop>(blk).items) {
                    if (!isRemoved(sid, "SHOP", it.item.name)) filtered.push_back(it);
                }
                if (!filtered.empty()) gererShop(filtered);
            }
        } else if (std::holds_alternative<BlockRemove>(blk)) {
            appliquerRemoveBlock(std::get<BlockRemove>(blk), sid);
        } else if (std::holds_alternative<BlockAdd>(blk)) {
            appliquerAddBlock(std::get<BlockAdd>(blk));
        } else if (std::holds_alternative<BlockGo>(blk)) {
            afficherDescription(description);
            std::cout << "C'est la fin de votre aventure...\n";
            sceneActuelle.clear(); return;
        } else if (std::holds_alternative<BlockVictory>(blk)) {
            afficherDescription(description);
            std::cout << "Felicitations, vous avez termine l'aventure avec succes !\n";
            sceneActuelle.clear(); return;
        }
    }
    afficherDescription(description);
    if (!prochaineScene.empty()) {
        previousScene = sceneActuelle;
        sceneActuelle = prochaineScene;
    }
}

void Histoire::gererGoldDelta(int delta) {
    hero->ajouterOr(delta);
}

void Histoire::gererShop(const std::vector<ShopItem>& items) {
    if (items.empty()) return;
    while (true) {
        std::cout << ui::color("\n=== Boutique ===\n", ui::CYAN);
        std::cout << "Or: " << hero->getOr() << "\n";
        std::cout << ui::color(" N°  Type     Objet                         Val   Prix\n", ui::BOLD);
        for (size_t i=0;i<items.size();++i) {
            const auto& it = items[i];
            bool afford = hero->getOr() >= it.price;
            std::string idx = (i+1 < 10 ? " " : "") + std::to_string(i+1);
            std::string type = (it.item.type==ItemType::Arme?"arme":"armure");
            std::string name = it.item.name; if (name.size()>27) name = name.substr(0,27) + "…";
            std::ostringstream line;
            line << " " << idx << "  "
                 << std::left << std::setw(8) << type << " "
                 << std::left << std::setw(28) << name << " "
                 << std::right << std::setw(3) << ("+" + std::to_string(it.item.value)) << "  "
                 << std::right << std::setw(4) << it.price << " or";
            if (afford) std::cout << ui::color(line.str(), ui::GREEN) << "\n";
            else std::cout << ui::color(line.str(), ui::RED) << "\n";
        }
        std::cout << ui::color("0. Quitter\nChoix: ", ui::BOLD);
        int c; if(!(std::cin>>c)){ std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); continue; }
        if (c==0) break;
        if (c<1 || c>(int)items.size()) { std::cout << "Choix invalide.\n"; continue; }
        ShopItem it = items[c-1];
        if (hero->getOr() < it.price) { std::cout << "Pas assez d'or.\n"; continue; }
        hero->ajouterOr(-it.price);
        if (it.item.effect.has_value()) {
            const auto& eff = *it.item.effect;
            std::string effName = (eff.kind==EffectKind::DOT?"DOT":(eff.kind==EffectKind::BuffDef?"BUFF_DEF":"NONE"));
            if (!eff.label.empty()) hero->registerItemEffect(it.item.name, effName, eff.label, eff.value, eff.duration);
            else hero->registerItemEffect(it.item.name, effName, eff.value, eff.duration);
        }
        if (it.item.type == ItemType::Arme) hero->ajouterArme(it.item.name, it.item.value);
        else hero->ajouterArmure(it.item.name, it.item.value);
    }
}

int Histoire::extractSceneId(const std::string& path) {
    size_t slash = path.find_last_of("/\\");
    std::string fname = (slash == std::string::npos) ? path : path.substr(slash + 1);
    size_t dot = fname.find('.');
    std::string idstr = (dot == std::string::npos) ? fname : fname.substr(0, dot);
    try { return std::stoi(idstr); } catch (...) { return -1; }
}

void Histoire::addRemoval(int sceneId, const std::string& type, const std::string& param) {
    std::string p = param.empty()?std::string("*"):param;
    std::string key = std::to_string(sceneId) + ":" + type + ":" + p;
    removals.insert(key);
}

Histoire::CombatOutcome Histoire::gererCombat(const std::string& nomMonstre, int pv, int attaque, int defense, int gold, int enemySpeed, EnemyReveal reveal) {
    Entite monstre(nomMonstre, pv, 0, defense);
    std::cout << "\nUn combat commence contre " << nomMonstre << " !\n";

    // Simple speed model
    int heroSpeed = hero->getSpdBase(); // valeur provenant de la classe

    while (hero->estVivant() && monstre.estVivant()) {
        hero->tickEffects();
        monstre.tickEffects();
        if (!hero->estVivant() || !monstre.estVivant()) break;

        bool heroFirst = (heroSpeed >= enemySpeed);

        auto printEnemy = [&]() {
            if (reveal == EnemyReveal::Hide) return;
            if (reveal == EnemyReveal::Full) {
                std::cout << "\n--- Adversaire ---\n";
                monstre.afficherStats();
            } else {
                std::cout << "\nAdversaire: " << nomMonstre
                          << " | PV: " << monstre.getPV() << "/" << monstre.getPVmax() << "\n";
            }
        };
        printEnemy();

        auto tourHero = [&]() -> bool {
            // Quick status line
            std::cout << ui::color("[Vous] ", ui::BOLD)
                      << "PV " << hero->getPV() << "/" << hero->getPVmax()
                      << " | ATK " << hero->getAtkBase()
                      << " | SPD " << heroSpeed
                      << " | Or " << hero->getOr() << "\n";
            // Menu d'action enrichi
            while (true) {
                std::cout << "\nActions :\n"
                          << ui::color("1.", ui::YELLOW) << " Attaque de base\n"
                          << ui::color("2.", ui::YELLOW) << " Utiliser une compétence\n"
                          << ui::color("3.", ui::YELLOW) << " Inventaire / Équipement\n"
                          << ui::color("4.", ui::YELLOW) << " Afficher les statistiques\n"
                          << ui::color("5.", ui::YELLOW) << " Fuir le combat\n";
                std::cout << ui::color("Votre choix : ", ui::BOLD);
                std::string choixAction; if (!(std::cin >> choixAction)) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); continue; }
                if (choixAction == "1") {
                    // Attaque de base
                    hero->attaquer(monstre, '+', hero->getAtkBase());
                    return true; // tour consommé
                } else if (choixAction == "2") {
                    std::cout << ui::color("Choisissez une compétence (0 pour annuler) :\n", ui::BOLD);
                    hero->afficherCompetences();
                    int choix;
                    std::cout << ui::color("Compétence à utiliser : ", ui::BOLD);
                    if (!(std::cin >> choix)) { std::cin.clear(); std::cin.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); continue; }
                    std::cout << "\n";
                    if (choix == 0) {
                        // Retour au menu d'action sans consommer de tour
                        continue;
                    }
                    if (choix > 0 && static_cast<size_t>(choix) <= hero->getNombreCompetences() && hero->utiliserCompetence(choix - 1, monstre)) {
                        return true; // tour consommé
                    } else {
                        std::cout << ui::color("Choix invalide, veuillez réessayer.\n", ui::RED);
                    }
                } else if (choixAction == "3") {
                    hero->equiperObjet();
                    // ne consomme pas le tour
                } else if (choixAction == "4") {
                    std::cout << "\n--- Vos statistiques ---\n";
                    hero->afficherStats();
                    std::cout << "PV:  " << hero->getPV() << "/" << hero->getPVmax()
                              << "  " << ui::bar(hero->getPV(), hero->getPVmax(), 24, '#', '-') << "\n";
                    // ne consomme pas le tour
                } else if (choixAction == "5") {
                    // Chance de fuite = vJ/(vJ+vE)
                    double chance = (double)heroSpeed / (double)(heroSpeed + std::max(1, enemySpeed));
                    std::random_device rd; std::mt19937 gen(rd()); std::uniform_real_distribution<> dis(0.0, 1.0);
                    double r = dis(gen);
                    if (r < chance) {
                        std::cout << ui::color("Vous parvenez à fuir !\n", ui::GREEN);
                        hero->reinitialiserProtection();
                        return false; // signale fin de combat par fuite
                    } else {
                        std::cout << ui::color("Échec de la fuite !\n", ui::RED);
                        return true; // tour consommé, l'ennemi jouera
                    }
                } else {
                    std::cout << ui::color("Choix invalide.\n", ui::RED);
                }
            }
        };

        if (heroFirst) {
            bool continuer = tourHero();
            if (!continuer) { return CombatOutcome::Flee; }
            if (!monstre.estVivant()) {
                std::cout << "Vous avez vaincu " << nomMonstre << " !\n";
                if (gold > 0) { hero->ajouterOr(gold); }
                hero->reinitialiserProtection();
                return CombatOutcome::Win;
            }
            std::cout << nomMonstre << " attaque !\n";
            monstre.attaquer(*hero,'%',attaque);
        } else {
            std::cout << nomMonstre << " attaque !\n";
            monstre.attaquer(*hero,'%',attaque);
            if (!hero->estVivant()) {
                std::cout << "Vous avez ete vaincu... Fin du jeu.\n";
                sceneActuelle.clear();
                hero->reinitialiserProtection();
                return CombatOutcome::Lose;
            }
            bool continuer = tourHero();
            if (!continuer) { return CombatOutcome::Flee; }
            if (!monstre.estVivant()) {
                std::cout << "Vous avez vaincu " << nomMonstre << " !\n";
                if (gold > 0) { hero->ajouterOr(gold); }
                hero->reinitialiserProtection();
                return CombatOutcome::Win;
            }
        }

    }
    // sortie de boucle sans retour explicite: victoire ou défaite déjà gérée
    hero->reinitialiserProtection();
    if (!hero->estVivant()) { return CombatOutcome::Lose; }
    if (!monstre.estVivant()) { if (gold > 0) hero->ajouterOr(gold); return CombatOutcome::Win; }
    return CombatOutcome::Win;
}

void Histoire::gererEquipement(const std::string& type, const std::string& nom, int valeur) {
    if (type == "arme") {
        hero->ajouterArme(nom, valeur);
    } else if (type == "armure") {
        hero->ajouterArmure(nom, valeur);
    }
}

bool Histoire::isRemoved(int sceneId, const std::string& type, const std::string& param) const {
    // exact match
    std::string key = std::to_string(sceneId) + ":" + type + ":" + param;
    if (removals.find(key) != removals.end()) return true;
    // wildcard removal for this type
    std::string keyAny = std::to_string(sceneId) + ":" + type + ":*";
    return removals.find(keyAny) != removals.end();
}

void Histoire::appliquerRemoveBlock(const BlockRemove& blk, int currentSceneId) {
    for (const auto& r : blk.rules) {
        int sid = r.isThis ? currentSceneId : r.sceneId;
        std::string type;
        switch (r.type) {
            case RemoveType::Path: type = "PATH"; break;
            case RemoveType::Combat: type = "COMBAT"; break;
            case RemoveType::Arme: type = "ARME"; break;
            case RemoveType::Armure: type = "ARMURE"; break;
            case RemoveType::Gold: type = "GOLD"; break;
            case RemoveType::Shop: type = "SHOP"; break;
        }
        addRemoval(sid, type, r.param);
    }
}

void Histoire::appliquerAddBlock(const BlockAdd& blk) {
    for (const auto& ar : blk.rules) {
        int sid = ar.isThis ? extractSceneId(sceneActuelle) : ar.sceneId;
        if (ar.type == AddType::Path) {
            for (const auto& p : ar.paths) additions[sid].paths.emplace_back(p.id, p.text);
        } else if (ar.type == AddType::Combat) {
            for (const auto& e : ar.combats) additions[sid].combats.push_back({e.name, e.hp, e.def, e.atk, e.gold, e.spd, e.reveal});
        } else if (ar.type == AddType::Item) {
            for (const auto& it : ar.items) {
                additions[sid].items.push_back({it.type==ItemType::Arme?std::string("arme"):std::string("armure"), it.name, it.value});
            }
        } else if (ar.type == AddType::Gold) {
            additions[sid].goldDeltas.push_back(ar.goldDelta);
        } else if (ar.type == AddType::Shop) {
            for (const auto& s : ar.shopItems) additions[sid].shops.push_back(s);
        }
    }
}

void Histoire::afficherDescription(std::string& description) {
    if (!description.empty()) {
        ui::clear();
        std::cout << ui::color("******************************\n\n", ui::CYAN);
        ui::typewrite(description);
        std::cout << "\n" << ui::color("\n******************************\n", ui::CYAN);
        description.clear(); // Efface la description après affichage
    }
}





