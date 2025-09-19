#include "Histoire.h"
#include <vector>
#include <thread>
#include <chrono>
#include <sstream>
#include "GameUI.h"
#include "ui.h"
#include <iomanip>
#include <cctype>
#include <limits>
#include <random>
#include <variant>
#include <iostream>


Histoire::Histoire(std::unique_ptr<Entite> hero,
                   const scene::SceneWorld& worldRef,
                   const std::filesystem::path& dataDir,
                   const std::string& startSceneKey)
    : hero(std::move(hero)),
      world(&worldRef),
      sceneActuelleId(startSceneKey),
      dataDir(dataDir) {
    if (!world || !world->getScene(sceneActuelleId)) {
        std::cerr << "Impossible de trouver la scene de depart \"" << startSceneKey << "\"." << std::endl;
        sceneActuelleId.clear();
    }
}

void Histoire::jouer() {
    GameUI& io = activeGameUI();
    while (!sceneActuelleId.empty()) {
        chargerScene2();
    }

    io.typewriteWords("Appuyez sur une touche pour fermer le jeu...\n", 28, 110);
    io.discardLine();
    std::string waitInput;
    io.readLine(waitInput);
    io.typewriteWords("Fin du jeu.\n", 28, 110);
}

// New parser-driven scene execution
void Histoire::chargerScene2() {
    GameUI& io = activeGameUI();
    if (hero) hero->setUI(&io);
    if (clearOnNextScene) { io.clear(); clearOnNextScene = false; }
    const Scene* sc = (world ? world->getScene(sceneActuelleId) : nullptr);
    if (!sc) {
        std::cerr << "Impossible de charger la scene : " << sceneActuelleId << std::endl;
        sceneActuelleId.clear();
        return;
    }

    SceneState state;
    state.sid = sceneActuelleId;

    for (const auto& blk : sc->blocks) {
        BlockFlow flow = processBlock(blk, state);
        if (flow == BlockFlow::Return) {
            flushDeferredDirectives(state);
            return;
        }
        if (flow == BlockFlow::Break) {
            break;
        }
    }

    flushDeferredDirectives(state);
    if (!state.prochaineScene.empty()) {
        previousSceneId = sceneActuelleId;
        sceneActuelleId = state.prochaineScene;
    }
}

Histoire::BlockFlow Histoire::processBlock(const SceneBlock& blk, SceneState& state) {
    return std::visit([&](const auto& concrete) -> BlockFlow {
        return handleBlock(concrete, state);
    }, blk);
}

Histoire::BlockFlow Histoire::handleBlock(const BlockLore& blk, SceneState& state) {
    std::string keyIndex = std::to_string(++state.loreIndex);
    if (!isRemoved(state.sid, "LORE", "*") && !isRemoved(state.sid, "LORE", keyIndex)) {
        afficherLore(blk.text);
    }
    return BlockFlow::Continue;
}

Histoire::BlockFlow Histoire::handleBlock(const BlockCombat& blk, SceneState& state) {
    for (const auto& enemy : blk.enemies) {
        if (isRemoved(state.sid, "COMBAT", enemy.name)) {
            continue;
        }
        auto outcome = gererCombat(enemy.name, enemy.hp, enemy.atk, enemy.def, enemy.gold, enemy.spd, enemy.reveal);
        if (outcome == CombatOutcome::Flee) {
            state.prochaineScene = previousSceneId.empty() ? sceneActuelleId : previousSceneId;
            return BlockFlow::Break;
        }
        if (outcome == CombatOutcome::Lose) {
            return BlockFlow::Break;
        }
        if (!hero->estVivant()) {
            break;
        }
    }
    if (!hero->estVivant()) {
        return BlockFlow::Break;
    }
    return BlockFlow::Continue;
}

Histoire::BlockFlow Histoire::handleBlock(const BlockItems& blk, SceneState& state) {
    for (const auto& it : blk.items) {
        std::string typeStr = (it.type == ItemType::Arme ? "ARME" : "ARMURE");
        if (isRemoved(state.sid, typeStr, it.name)) {
            continue;
        }
        if (it.effect.has_value()) {
            const auto& eff = *it.effect;
            Entite::ItemEffectType et = (eff.kind == EffectKind::DOT ? Entite::ItemEffectType::DOT : (eff.kind == EffectKind::BuffDef ? Entite::ItemEffectType::BUFF_DEF : Entite::ItemEffectType::None));
            if (!eff.label.empty()) {
                hero->registerItemEffect(it.name, et, eff.label, eff.value, eff.duration);
            } else {
                hero->registerItemEffect(it.name, et, eff.value, eff.duration);
            }
        }
        gererEquipement(it.type == ItemType::Arme ? "arme" : "armure", it.name, it.value);
    }
    return BlockFlow::Continue;
}

Histoire::BlockFlow Histoire::handleBlock(const BlockPath& blk, SceneState& state) {
    std::vector<std::pair<std::string, std::string>> chemins;
    BlockFlow flow = applySceneAdditionsForPaths(state, chemins);
    if (flow != BlockFlow::Continue) {
        return flow;
    }
    ajouterCheminsDepuisBloc(blk, state, chemins);
    if (chemins.empty()) {
        activeGameUI().typewriteWords(ui::color("Aucun chemin disponible.\n", ui::RED), 28, 110);
        return BlockFlow::Break;
    }
    afficherChemins(chemins);
    return gererChoixChemin(state, chemins);
}

Histoire::BlockFlow Histoire::applySceneAdditionsForPaths(SceneState& state, std::vector<std::pair<std::string, std::string>>& chemins) {
    auto itAdd = additions.find(state.sid);
    if (itAdd == additions.end()) {
        return BlockFlow::Continue;
    }
    const auto& add = itAdd->second;

    if (!isRemoved(state.sid, "LORE", "*")) {
        for (const auto& text : add.lores) {
            std::string idxStr = std::to_string(++state.loreIndex);
            if (!isRemoved(state.sid, "LORE", idxStr)) {
                afficherLore(text);
            }
        }
    }

    for (const auto& enemy : add.combats) {
        if (isRemoved(state.sid, "COMBAT", enemy.nom)) {
            continue;
        }
        auto outcome = gererCombat(enemy.nom, enemy.pv, enemy.atk, enemy.def, enemy.orGain, enemy.spd, enemy.reveal);
        if (outcome == CombatOutcome::Flee) {
            std::string target = previousSceneId.empty() ? sceneActuelleId : previousSceneId;
            sceneActuelleId = target;
            return BlockFlow::Return;
        }
        if (outcome == CombatOutcome::Lose) {
            return BlockFlow::Return;
        }
    }

    for (const auto& item : add.items) {
        std::string typeKey = (item.type == "arme" ? "ARME" : "ARMURE");
        if (!isRemoved(state.sid, typeKey, item.nom)) {
            gererEquipement(item.type, item.nom, item.valeur);
        }
    }

    for (int delta : add.goldDeltas) {
        if (!isRemoved(state.sid, "GOLD", "*") && !isRemoved(state.sid, "GOLD", std::to_string(delta))) {
            gererGoldDelta(delta);
        }
    }

    if (!isRemoved(state.sid, "SHOP", "*") && !add.shops.empty()) {
        std::vector<ShopItem> filtered;
        for (const auto& shopItem : add.shops) {
            if (!isRemoved(state.sid, "SHOP", shopItem.item.name)) {
                filtered.push_back(shopItem);
            }
        }
        if (!filtered.empty()) {
            gererShop(filtered);
        }
    }

    for (const auto& p : add.paths) {
        if (!isRemoved(state.sid, "PATH", p.first)) {
            std::string label = p.second.empty() ? std::string("Continuer") : p.second;
            chemins.emplace_back(p.first, label);
        }
    }

    return BlockFlow::Continue;
}

void Histoire::ajouterCheminsDepuisBloc(const BlockPath& blk, SceneState& state, std::vector<std::pair<std::string, std::string>>& chemins) {
    for (const auto& option : blk.options) {
        if (!isRemoved(state.sid, "PATH", option.id)) {
            std::string label = option.text.empty() ? std::string("Continuer") : option.text;
#ifdef DEBUG_REMOVE
            std::cerr << "[PATH] keep " << state.sid << ":" << option.id << "\n";
#endif
            chemins.emplace_back(option.id, label);
        } else {
#ifdef DEBUG_REMOVE
            std::cerr << "[PATH] removed " << state.sid << ":" << option.id << "\n";
#endif
        }
    }
}

void Histoire::afficherChemins(const std::vector<std::pair<std::string, std::string>>& chemins) {
    GameUI& io = activeGameUI();
    io.typewriteWords(ui::color("\n=== Chemins ===\n", ui::CYAN), 28, 110);
    for (size_t i = 0; i < chemins.size(); ++i) {
        std::string line = ui::color(std::to_string(i + 1), ui::YELLOW) + ". " + chemins[i].second + "\n";
        io.typewriteWords(line, 28, 110);
    }
}

Histoire::BlockFlow Histoire::gererChoixChemin(SceneState& state, const std::vector<std::pair<std::string, std::string>>& chemins) {
    GameUI& io = activeGameUI();
    while (true) {
        io.typewriteWords("\n" + ui::color("Choisissez une option ('I' pour les stats): ", ui::BOLD), 28, 110);
        std::string input;
        if (!io.readToken(input)) {
            io.discardLine();
            continue;
        }
        io.discardLine();
        if (input == "I" || input == "i") {
            afficherMenuInformationsChemin();
            continue;
        }
        try {
            int choix = std::stoi(input);
            if (choix > 0 && choix <= static_cast<int>(chemins.size())) {
                state.prochaineScene = chemins[choix - 1].first;
                clearOnNextScene = true;
                return BlockFlow::Continue;
            }
            io.typewriteWords(ui::color("Choix invalide. Reessayez.\n", ui::RED), 28, 110);
        } catch (...) {
            io.typewriteWords(ui::color("Entree invalide. Reessayez.\n", ui::RED), 28, 110);
        }
    }
}

void Histoire::afficherMenuInformationsChemin() {
    if (!hero) {
        return;
    }
    GameUI& io = activeGameUI();
    hero->afficherStats();
    io.typewriteWords("\nOptions disponibles :\n", 28, 110);
    io.typewriteWords("1. Retour au choix precedent\n", 28, 110);
    io.typewriteWords("2. Afficher l'inventaire\n", 28, 110);
    std::string choix;
    if (!io.readToken(choix)) {
        io.discardLine();
        return;
    }
    io.discardLine();
    if (choix == "2") {
        hero->equiperObjet();
    }
}

Histoire::BlockFlow Histoire::handleBlock(const BlockGold& blk, SceneState& state) {
    int delta = blk.delta;
    if (!isRemoved(state.sid, "GOLD", "*") && !isRemoved(state.sid, "GOLD", std::to_string(delta))) {
        gererGoldDelta(delta);
    }
    return BlockFlow::Continue;
}

Histoire::BlockFlow Histoire::handleBlock(const BlockShop& blk, SceneState& state) {
    if (isRemoved(state.sid, "SHOP", "*")) {
        return BlockFlow::Continue;
    }
    std::vector<ShopItem> filtered;
    for (const auto& it : blk.items) {
        if (!isRemoved(state.sid, "SHOP", it.item.name)) {
            filtered.push_back(it);
        }
    }
    if (!filtered.empty()) {
        gererShop(filtered);
    }
    return BlockFlow::Continue;
}

Histoire::BlockFlow Histoire::handleBlock(const BlockRemove& blk, SceneState& state) {
    state.deferredRemoves.push_back(&blk);
    return BlockFlow::Continue;
}

Histoire::BlockFlow Histoire::handleBlock(const BlockAdd& blk, SceneState& state) {
    state.deferredAdds.push_back(&blk);
    return BlockFlow::Continue;
}

Histoire::BlockFlow Histoire::handleBlock(const BlockVictory& blk, SceneState& state) {
    (void)blk;
    activeGameUI().typewriteWords("Felicitations, vous avez termine l'aventure avec succes !\n", 28, 110);
    sceneActuelleId.clear();
    return BlockFlow::Return;
}

Histoire::BlockFlow Histoire::handleBlock(const BlockGo& blk, SceneState& state) {
    (void)blk;
    activeGameUI().typewriteWords("C'est la fin de votre aventure...\n", 28, 110);
    sceneActuelleId.clear();
    return BlockFlow::Return;
}
void Histoire::gererGoldDelta(int delta) {
    hero->ajouterOr(delta);
}

void Histoire::gererShop(const std::vector<ShopItem>& items) {
    GameUI& io = activeGameUI();
    if (items.empty()) return;
    while (true) {
        io.print("\n");
        io.hr(u8"-", ui::CYAN);
        io.typewriteWords(ui::color(" Boutique \n", ui::BOLD), 28, 110);
        io.typewriteWords(std::string("Or: ") + std::to_string(hero->getOr()) + "\n", 28, 110);
        io.typewriteWords(ui::color(" N°  Type     Objet                         Val   Prix\n", ui::BOLD), 28, 110);
        for (size_t i=0;i<items.size();++i) {
            const auto& it = items[i];
            bool afford = hero->getOr() >= it.price;
            std::string idx = (i+1 < 10 ? " " : "") + std::to_string(i+1);
            std::string type = (it.item.type==ItemType::Arme?"arme":"armure");
            std::string name = it.item.name; if (name.size()>27) name = name.substr(0,27) + u8"…";
            std::ostringstream line;
            line << " " << idx << "  "
                 << std::left << std::setw(8) << type << " "
                 << std::left << std::setw(28) << name << " "
                 << std::right << std::setw(3) << ("+" + std::to_string(it.item.value)) << "  "
                 << std::right << std::setw(4) << it.price << " or";
            if (afford) io.typewriteWords(ui::color(line.str() + "\n", ui::GREEN), 28, 110);
            else io.typewriteWords(ui::color(line.str() + "\n", ui::RED), 28, 110);
        }
        io.typewriteWords(ui::color("0. Quitter\nChoix: ", ui::BOLD), 28, 110);
        int c = 0; if (!io.readInt(c)) { io.discardLine(); continue; }
        if (c==0) break;
        if (c<1 || c>(int)items.size()) { io.typewriteWords("Choix invalide.\n", 28, 110); continue; }
        ShopItem it = items[c-1];
        if (hero->getOr() < it.price) { io.typewriteWords("Pas assez d'or.\n", 28, 110); continue; }
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

void Histoire::addRemoval(const std::string& sceneKey, const std::string& type, const std::string& param) {
    std::string p = param.empty()?std::string("*"):param;
    std::string key = sceneKey + ":" + type + ":" + p;
#ifdef DEBUG_REMOVE
    std::cerr << "[remove] add " << key << "\n";
#endif
    removals.insert(key);
}

Histoire::CombatOutcome Histoire::gererCombat(const std::string& nomMonstre, int pv, int attaque, int defense, int gold, int enemySpeed, EnemyReveal reveal) {
    GameUI& io = activeGameUI();
    if (hero) hero->setUI(&io);
    Entite monstre(nomMonstre, pv, 0, defense);
    monstre.setUI(&io);
    io.typewriteWords("\nUn combat commence contre " + ui::color(nomMonstre, ui::MAGENTA) + " !\n", 28, 110);

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
            io.typewriteWords("\n--- Adversaire: " + ui::color(nomMonstre, ui::MAGENTA) + " ---\n", 28, 110);
                monstre.afficherStats(true, ui::MAGENTA);
            } else {
                {
                    std::ostringstream oss;
                    oss << "\nAdversaire: " << ui::color(nomMonstre, ui::MAGENTA)
                        << " | PV: " << monstre.getPV() << "/" << monstre.getPVmax() << "\n";
                    io.typewriteWords(oss.str(), 28, 110);
                }
            }
        };
        printEnemy();

        auto tourHero = [&]() -> bool {
            // Quick status line
            {
                std::ostringstream oss;
                oss << ui::color("[Vous] ", ui::BOLD)
                    << "PV " << hero->getPV() << "/" << hero->getPVmax()
                    << " | ATK " << hero->getAtkBase()
                    << " | SPD " << heroSpeed
                    << " | Or " << hero->getOr() << "\n";
                io.typewriteWords(oss.str(), 28, 110);
            }
            // Menu d'action enrichi
            while (true) {
                {
                    std::string menu;
                    menu += "\nActions :\n";
                    menu += ui::color("1.", ui::YELLOW); menu += " Attaque de base\n";
                    menu += ui::color("2.", ui::YELLOW); menu += " Utiliser une compétence\n";
                    menu += ui::color("3.", ui::YELLOW); menu += " Inventaire / Équipement\n";
                    menu += ui::color("4.", ui::YELLOW); menu += " Afficher les statistiques\n";
                    menu += ui::color("5.", ui::YELLOW); menu += " Fuir le combat\n";
                    io.typewriteWords(menu, 28, 110);
                }
                io.typewriteWords(ui::color("Votre choix : ", ui::BOLD), 28, 110);
                std::string choixAction; if (!io.readToken(choixAction)) { io.discardLine(); continue; }
                if (choixAction == "1") {
                    // Attaque de base
                    hero->attaquer(monstre, '+', hero->getAtkBase());
                    return true; // tour consommÃ©
                } else if (choixAction == "2") {
                    io.typewriteWords(ui::color("Choisissez une compétence (0 pour annuler) :\n", ui::BOLD), 28, 110);
                    hero->afficherCompetences();
                    int choix;
                    io.typewriteWords(ui::color("Compétence à utiliser : ", ui::BOLD), 28, 110);
                    if (!io.readInt(choix)) { io.discardLine(); continue; }
                    io.print("\n");
                    if (choix == 0) {
                        // Retour au menu d'action sans consommer de tour
                        continue;
                    }
                    if (choix > 0 && static_cast<size_t>(choix) <= hero->getNombreCompetences() && hero->utiliserCompetence(choix - 1, monstre)) {
                        return true; // tour consommé
                    } else {
                        io.typewriteWords(ui::color("Choix invalide, veuillez réessayer.\n", ui::RED), 28, 110);
                    }
                } else if (choixAction == "3") {
                    hero->equiperObjet();
                    // ne consomme pas le tour
                } else if (choixAction == "4") {
                    io.typewriteWords("\n--- Vos statistiques ---\n", 28, 110);
                    hero->afficherStats();
                    {
                        std::ostringstream oss;
                        oss << "PV:  " << hero->getPV() << "/" << hero->getPVmax()
                            << "  " << ui::bar(hero->getPV(), hero->getPVmax(), 24, '#', '-') << "\n";
                        io.typewriteWords(oss.str(), 28, 110);
                    }
                    // ne consomme pas le tour
                } else if (choixAction == "5") {
                    // Chance de fuite = vJ/(vJ+vE)
                    double chance = (double)heroSpeed / (double)(heroSpeed + std::max(1, enemySpeed));
                    double r = fleeDistribution(rng);
                    if (r < chance) {
                        io.typewriteWords(ui::color("Vous parvenez à fuir !\n", ui::GREEN), 28, 110);
                        hero->reinitialiserProtection();
                        return false; // signale fin de combat par fuite
                    } else {
                        io.typewriteWords(ui::color("Échec de la fuite !\n", ui::RED), 28, 110);
                        return true; // tour consommé, l'ennemi jouera
                    }
                } else {
                    io.typewriteWords(ui::color("Choix invalide.\n", ui::RED), 28, 110);
                }
            }
        };

        if (heroFirst) {
            bool continuer = tourHero();
            if (!continuer) { return CombatOutcome::Flee; }
            if (!monstre.estVivant()) {
                io.typewriteWords(std::string("Vous avez vaincu ") + nomMonstre + " !\n", 28, 110);
                if (gold > 0) { hero->ajouterOr(gold); }
                hero->reinitialiserProtection();
                return CombatOutcome::Win;
            }
            io.typewriteWords(ui::color(nomMonstre, ui::MAGENTA) + " attaque !\n", 28, 110);
            monstre.attaquer(*hero,'%',attaque);
        } else {
            io.typewriteWords(ui::color(nomMonstre, ui::MAGENTA) + " attaque !\n", 28, 110);
            monstre.attaquer(*hero,'%',attaque);
            if (!hero->estVivant()) {
                io.typewriteWords("Vous avez été vaincu... Fin du jeu.\n", 28, 110);
                sceneActuelleId.clear();
                hero->reinitialiserProtection();
                return CombatOutcome::Lose;
            }
            bool continuer = tourHero();
            if (!continuer) { return CombatOutcome::Flee; }
            if (!monstre.estVivant()) {
                io.typewriteWords(std::string("Vous avez vaincu ") + ui::color(nomMonstre, ui::MAGENTA) + " !\n", 28, 110);
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

bool Histoire::isRemoved(const std::string& sceneId, const std::string& type, const std::string& param) const {
    std::string key = sceneId + ":" + type + ":" + param;
    auto it = removals.find(key);
#ifdef DEBUG_REMOVE
    std::cerr << "[isRemoved] " << key << " -> " << (it!=removals.end()) << "\n";
#endif
    if (it != removals.end()) return true;
    std::string keyAny = sceneId + ":" + type + ":*";
#ifdef DEBUG_REMOVE
    std::cerr << "[isRemoved] " << keyAny << " -> " << (removals.find(keyAny)!=removals.end()) << "\n";
#endif
    return removals.find(keyAny) != removals.end();
}

void Histoire::appliquerRemoveBlock(const BlockRemove& blk, const std::string& currentSceneId) {
    for (const auto& r : blk.rules) {
        std::string sid = r.isThis ? currentSceneId : r.sceneKey;
        std::string type;
        switch (r.type) {
            case RemoveType::Path: type = "PATH"; break;
            case RemoveType::Combat: type = "COMBAT"; break;
            case RemoveType::Arme: type = "ARME"; break;
            case RemoveType::Armure: type = "ARMURE"; break;
            case RemoveType::Gold: type = "GOLD"; break;
            case RemoveType::Shop: type = "SHOP"; break;
            case RemoveType::Lore: type = "LORE"; break;
        }
        addRemoval(sid, type, r.param);
    }
}

void Histoire::appliquerAddBlock(const BlockAdd& blk, const std::string& currentSceneKey) {

    for (const auto& ar : blk.rules) {
        std::string sid = ar.isThis ? currentSceneKey : ar.sceneKey;
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
        } else if (ar.type == AddType::Lore) {
            for (const auto& L : ar.lores) additions[sid].lores.push_back(L);
        }
    }
}

void Histoire::flushDeferredDirectives(SceneState& state) {
    for (const auto* blk : state.deferredRemoves) {
        appliquerRemoveBlock(*blk, state.sid);
    }
    for (const auto* blk : state.deferredAdds) {
        appliquerAddBlock(*blk, state.sid);
    }
    state.deferredRemoves.clear();
    state.deferredAdds.clear();
}



void Histoire::afficherLore(const std::string& text) {
    if (text.empty()) return;
    GameUI& io = activeGameUI();
    io.hr(u8"-", ui::CYAN);
    io.print("\n");
    // Human-like handwriting effect: base per-char delay with jitter
    io.typewriteJitter(text, 22, 12);
    io.print("\n");
    io.hr(u8"-", ui::CYAN);
}































