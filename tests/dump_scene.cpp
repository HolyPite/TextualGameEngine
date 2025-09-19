#include <iostream>
#include <variant>
#include "../src/SceneParser.h"

int main(int argc, char** argv){
    if (argc<2) { std::cerr << "usage: dump_scene <file>\n"; return 1; }
    auto sc = scene::parse(argv[1]);
    int i=0;
    for (const auto& b : sc.blocks) {
        std::cout << ++i << ": ";
        if (std::holds_alternative<BlockLore>(b)) {
            std::cout << "LORE: ";
            std::string t = std::get<BlockLore>(b).text; for(char& c: t) if(c=='\n') c=' ';
            std::cout << t << "\n";
        } else if (std::holds_alternative<BlockPath>(b)) {
            std::cout << "PATH\n";
        } else if (std::holds_alternative<BlockItems>(b)) {
            std::cout << "ITEMS\n";
        } else if (std::holds_alternative<BlockCombat>(b)) {
            std::cout << "COMBAT\n";
        } else if (std::holds_alternative<BlockGold>(b)) {
            std::cout << "GOLD\n";
        } else if (std::holds_alternative<BlockShop>(b)) {
            std::cout << "SHOP\n";
        } else if (std::holds_alternative<BlockRemove>(b)) {
            std::cout << "REMOVE\n";
        } else if (std::holds_alternative<BlockAdd>(b)) {
            std::cout << "ADD\n";
        } else if (std::holds_alternative<BlockVictory>(b)) {
            std::cout << "VICTOIRE\n";
        } else if (std::holds_alternative<BlockGo>(b)) {
            std::cout << "GO\n";
        }
    }
    return 0;
}

