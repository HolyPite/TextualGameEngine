#ifndef HISTOIRE_H
#define HISTOIRE_H

#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>

#include "Entite.h"
#include "Scene.h"
#include "SceneWorld.h"

class Histoire {
private:
    std::unique_ptr<Entite> hero;
    const scene::SceneWorld* world{nullptr};
    std::string sceneActuelleId;
    std::string previousSceneId;
    // Runtime removals and additions
    std::unordered_set<std::string> removals; // key: sceneKey:TYPE:param
    std::filesystem::path dataDir;
    bool clearOnNextScene{false};

    struct CombatDef { std::string nom; int pv; int def; int atk; int orGain; int spd; EnemyReveal reveal; };
    struct ItemDef { std::string type; std::string nom; int valeur; };
    struct SceneAdditions {
        std::vector<std::pair<std::string,std::string>> paths;
        std::vector<CombatDef> combats;
        std::vector<ItemDef> items;
        std::vector<int> goldDeltas;
        std::vector<ShopItem> shops;
        std::vector<std::string> lores;
    };
    std::unordered_map<std::string, SceneAdditions> additions;

public:
    enum class CombatOutcome { Win, Lose, Flee };
    Histoire(std::unique_ptr<Entite> hero,
             const scene::SceneWorld& worldRef,
             const std::filesystem::path& dataDir,
             const std::string& startSceneKey = std::string("start"));

    void jouer();

private:
    void chargerScene2(); // new reader using SceneParser
    void afficherDescription(std::string& description);
    void afficherLore(const std::string& text);
    CombatOutcome gererCombat(const std::string& nomMonstre, int pv, int attaque, int defense, int gold, int enemySpeed, EnemyReveal reveal);
    void gererEquipement(const std::string& type, const std::string& nom, int valeur);
    // Directives
    void appliquerRemoveBlock(const BlockRemove& blk, const std::string& currentSceneKey);
    void appliquerAddBlock(const BlockAdd& blk, const std::string& currentSceneKey);
    bool isRemoved(const std::string& sceneKey, const std::string& type, const std::string& param) const;
    void addRemoval(const std::string& sceneKey, const std::string& type, const std::string& param);

    // Economie / Boutique
    enum class BlockFlow { Continue, Break, Return };
    struct SceneState {
        std::string description;
        std::string prochaineScene;
        std::string sid;
        int loreIndex{0};
        std::vector<const BlockRemove*> deferredRemoves;
        std::vector<const BlockAdd*> deferredAdds;
    };

    void flushDeferredDirectives(SceneState& state);
    BlockFlow processBlock(const SceneBlock& blk, SceneState& state);
    BlockFlow handleBlock(const BlockDescription& blk, SceneState& state);
    BlockFlow handleBlock(const BlockLore& blk, SceneState& state);
    BlockFlow handleBlock(const BlockCombat& blk, SceneState& state);
    BlockFlow handleBlock(const BlockItems& blk, SceneState& state);
    BlockFlow handleBlock(const BlockPath& blk, SceneState& state);
    BlockFlow handleBlock(const BlockGold& blk, SceneState& state);
    BlockFlow handleBlock(const BlockShop& blk, SceneState& state);
    BlockFlow handleBlock(const BlockRemove& blk, SceneState& state);
    BlockFlow handleBlock(const BlockAdd& blk, SceneState& state);
    BlockFlow handleBlock(const BlockVictory& blk, SceneState& state);
    BlockFlow handleBlock(const BlockGo& blk, SceneState& state);
    void gererGoldDelta(int delta);
    void gererShop(const std::vector<ShopItem>& items);
};

#endif // HISTOIRE_H
