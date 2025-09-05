#include <iostream>
#include <fstream>
#include <cassert>
#include <filesystem>
#include "src/SceneParser.h"

static void writeFile(const std::string& path, const std::string& content) {
    std::ofstream f(path); assert(f.is_open()); f << content; f.close();
}

int main() {
    std::filesystem::create_directories("tests/data");
    // Basic scene with PATH, COMBAT, ITEM, GOLD, SHOP
    writeFile("tests/data/s1.txt",
        "Vous entrez.\n\n"
        "*COMBAT*\nGob;10;1;2\n\n"
        "*ITEM*\nARME;Epee;3;DOT;Poison;2;1\n\n"
        "*GOLD*\n5\n\n"
        "*SHOP*\nARME;Dague;1;10\n\n"
        "*PATH*\n2 Continuer\n");

    auto sc1 = scene::parse("tests/data/s1.txt");
    assert(!sc1.blocks.empty());
    bool hasCombat=false, hasItem=false, hasGold=false, hasShop=false, hasPath=false;
    for (const auto& b : sc1.blocks) {
        hasCombat |= std::holds_alternative<BlockCombat>(b);
        hasItem   |= std::holds_alternative<BlockItems>(b);
        hasGold   |= std::holds_alternative<BlockGold>(b);
        hasShop   |= std::holds_alternative<BlockShop>(b);
        hasPath   |= std::holds_alternative<BlockPath>(b);
    }
    assert(hasCombat && hasItem && hasGold && hasShop && hasPath);

    // ADD and REMOVE blocks
    writeFile("tests/data/s2.txt",
        "*REMOVE*\nthis PATH 2\nthis COMBAT Gob\nthis GOLD *\nthis SHOP *\n\n"
        "*ADD*\nthis GOLD\n*GOLD*\n10\n\n"
        "this SHOP\n*SHOP*\nARME;Hache;2;20\n\n");
    auto sc2 = scene::parse("tests/data/s2.txt");
    bool hasAdd=false, hasRemove=false;
    for (const auto& b : sc2.blocks) {
        hasAdd   |= std::holds_alternative<BlockAdd>(b);
        hasRemove|= std::holds_alternative<BlockRemove>(b);
    }
    assert(hasAdd && hasRemove);

    std::cout << "Parser tests passed.\n";
    return 0;
}
