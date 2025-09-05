#ifndef HISTOIRE_H
#define HISTOIRE_H

#include <vector>
#include <iostream>
#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include "Entite.h"
#include "Scene.h"

class Histoire {
private:
    std::unique_ptr<Entite> hero;
    std::string sceneActuelle;
    // Runtime removals and additions
    std::unordered_set<std::string> removals; // key: sceneId:TYPE:param
    std::filesystem::path dataDir;

    struct CombatDef { std::string nom; int pv; int def; int atk; int orGain; };
    struct ItemDef { std::string type; std::string nom; int valeur; };
    struct SceneAdditions {
        std::vector<std::pair<int,std::string>> paths;
        std::vector<CombatDef> combats;
        std::vector<ItemDef> items;
        std::vector<int> goldDeltas;
        std::vector<ShopItem> shops;
    };
    std::unordered_map<int, SceneAdditions> additions;

public:
    Histoire(std::unique_ptr<Entite> hero, const std::filesystem::path& dataDir, int startSceneId = 0)
        : hero(std::move(hero)), dataDir(dataDir) {
        sceneActuelle = (dataDir / "scenes" / (std::to_string(startSceneId) + ".txt")).string();
    }

    void jouer();

private:
    void chargerScene2(); // new reader using SceneParser
    void afficherDescription(std::string& description);
    void gererCombat(const std::string& nomMonstre, int pv, int attaque, int defense, int gold);
    void gererEquipement(const std::string& type, const std::string& nom, int valeur);
    // Directives
    void appliquerRemoveBlock(const BlockRemove& blk, int currentSceneId);
    void appliquerAddBlock(const BlockAdd& blk);
    bool isRemoved(int sceneId, const std::string& type, const std::string& param) const;
    static int extractSceneId(const std::string& path);
    void addRemoval(int sceneId, const std::string& type, const std::string& param);

    // Economie / Boutique
    void gererGoldDelta(int delta);
    void gererShop(const std::vector<ShopItem>& items);
};

#endif // HISTOIRE_H
