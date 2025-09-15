#ifndef SCENE_H
#define SCENE_H

#include <string>
#include <vector>
#include <optional>
#include <variant>

// Strong types for scene content
enum class ItemType { Arme, Armure };
enum class EffectKind { None, DOT, BuffDef };
// Visibility of enemy stats during combat
enum class EnemyReveal { Min, Full, Hide };

struct EffectSpec {
    EffectKind kind{EffectKind::None};
    std::string label; // optional display label
    int duration{0};   // 0 = passive (for BuffDef)
    int value{0};
};

struct ItemSpec {
    ItemType type{ItemType::Arme};
    std::string name;
    int value{0};
    std::optional<EffectSpec> effect; // optional effect
};

struct CombatEnemy {
    std::string name;
    int hp{0};
    int def{0};
    int atk{0};
    int gold{0}; // loot on victory
    int spd{10}; // speed (default 10)
    EnemyReveal reveal{EnemyReveal::Min};
};

struct PathOption {
    std::string id;   // scene key (file stem)
    std::string text; // empty => default label
};

struct ShopItem {
    ItemSpec item;
    int price{0};
};

enum class RemoveType { Path, Combat, Arme, Armure, Gold, Shop, Lore };
struct RemoveRule {
    bool isThis{true};
    std::string sceneKey; // if !isThis: target scene key (file stem)
    RemoveType type{RemoveType::Path};
    std::string param; // path id (as string), monster name or item name
};

enum class AddType { Path, Combat, Item, Gold, Shop, Lore };
struct AddRule {
    bool isThis{true};
    std::string sceneKey; // if !isThis: target scene key
    AddType type{AddType::Path};
    // Payloads
    std::vector<PathOption> paths;
    std::vector<CombatEnemy> combats;
    std::vector<ItemSpec> items;
    int goldDelta{0};
    std::vector<ShopItem> shopItems;
    std::vector<std::string> lores;
};

// Blocks in original order
struct BlockDescription { std::string text; };
struct BlockCombat { std::vector<CombatEnemy> enemies; };
struct BlockItems { std::vector<ItemSpec> items; };
struct BlockPath { std::vector<PathOption> options; };
struct BlockGold { int delta{0}; };
struct BlockShop { std::vector<ShopItem> items; };
struct BlockRemove { std::vector<RemoveRule> rules; };
struct BlockAdd { std::vector<AddRule> rules; };
struct BlockVictory {};
struct BlockGo {};
struct BlockLore { std::string text; };

using SceneBlock = std::variant<
    BlockDescription,
    BlockCombat,
    BlockItems,
    BlockPath,
    BlockGold,
    BlockShop,
    BlockRemove,
    BlockAdd,
    BlockVictory,
    BlockGo,
    BlockLore
>;

struct Scene {
    std::string path; // source file path
    std::vector<SceneBlock> blocks; // in order
};

#endif // SCENE_H
