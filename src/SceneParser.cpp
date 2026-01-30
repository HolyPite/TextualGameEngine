#include "SceneParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <utility>
#include <cctype>
#include <limits>
#include <iostream>

namespace {
    static inline std::string trim_copy(const std::string& s){
        size_t a=0,b=s.size();
        while(a<b && std::isspace((unsigned char)s[a])) ++a;
        while(b>a && std::isspace((unsigned char)s[b-1])) --b;
        return s.substr(a,b-a);
    }
    static inline std::vector<std::string> splitSemi(const std::string& s) {
        std::vector<std::string> out; std::string cur;
        for(char ch: s){ if(ch==';'){ out.push_back(trim_copy(cur)); cur.clear(); } else cur.push_back(ch);} out.push_back(trim_copy(cur));
        return out;
    }

}

namespace scene {

ItemType parseItemType(const std::string& ss) {
    std::string s = ss; for (auto& c : s) c = (char)std::tolower((unsigned char)c);
    if (s == "arme") return ItemType::Arme;
    return ItemType::Armure;
}

EffectKind parseEffectKind(const std::string& ss) {
    std::string s = ss; for (auto& c : s) c = (char)std::toupper((unsigned char)c);
    if (s == "DOT") return EffectKind::DOT;
    if (s == "BUFF_DEF") return EffectKind::BuffDef;
    if (s == "LIFESTEAL") return EffectKind::LifeSteal;
    if (s == "LIFE_STEAL") return EffectKind::LifeSteal;
    if (s == "STUN") return EffectKind::Stun;
    return EffectKind::None;
}

EnemyReveal parseEnemyReveal(const std::string& ss) {
    std::string s = ss; for (auto& c : s) c = (char)std::toupper((unsigned char)c);
    if (s == "FULL") return EnemyReveal::Full;
    if (s == "HIDE") return EnemyReveal::Hide;
    return EnemyReveal::Min;
}

static ItemSpec parseItemLineGeneric(const std::vector<std::string>& parts) {
    // parts: Type;Nom;Valeur[;Effet;NomEffet;Duree;ValEff]
    ItemSpec it;
    it.type = parseItemType(parts[0]);
    it.name = parts[1];
    try { it.value = std::stoi(parts[2]); } catch(...) { it.value = 0; }
    if (parts.size() >= 7) {
        EffectSpec eff;
        eff.kind = parseEffectKind(parts[3]);
        eff.label = parts[4];
        try { eff.duration = std::stoi(parts[5]); } catch(...) { eff.duration = 0; }
        try { eff.value = std::stoi(parts[6]); } catch(...) { eff.value = 0; }
        if (eff.kind != EffectKind::None) it.effect = eff;
    }
    return it;
}

Scene parse(const std::string& filepath) {
    Scene scene; scene.path = filepath;
    std::ifstream f(filepath);
    if (!f.is_open()) return scene;

    std::string line;
    long long lineNo = 0;


    bool firstLine = true;
    while (std::getline(f, line)) { ++lineNo;
        if (line.empty()) { continue; }
        std::string l = trim_copy(line);
        if (firstLine) {
            // Strip UTF-8 BOM if present
            if (l.size() >= 3 && (unsigned char)l[0] == 0xEF && (unsigned char)l[1] == 0xBB && (unsigned char)l[2] == 0xBF) {
                l.erase(0, 3);
            }
            firstLine = false;
        }
        if (l.empty()) { continue; }

        if (l.rfind("*COMBAT*", 0) == 0) {
            std::vector<CombatEnemy> enemies;
            // Try strict multi-line Nom;PV;DEF;ATK[;GOLD[;SPD[;REVEAL]]] until blank or next tag
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2);
                if (l2.empty()) break;
                if (!l2.empty() && l2[0]=='*') { f.seekg(pos); break; }
                auto parts = splitSemi(l2);
                if (parts.size() >= 4) {
                    CombatEnemy e; e.name = parts[0];
                    try{ e.hp = std::stoi(parts[1]); }catch(...){ e.hp=0; }
                    try{ e.def = std::stoi(parts[2]); }catch(...){ e.def=0; }
                    try{ e.atk = std::stoi(parts[3]); }catch(...){ e.atk=0; }
                    if (parts.size() >= 5) { try{ e.gold = std::stoi(parts[4]); }catch(...){ e.gold=0; } }
                    if (parts.size() >= 6) { try{ e.spd  = std::stoi(parts[5]); }catch(...){ e.spd=10; } }
                    if (parts.size() >= 7) { e.reveal = parseEnemyReveal(parts[6]); }
                    enemies.push_back(e);
                }
            }
            if (enemies.empty()) {
                // Legacy 1 enemy on 4 lines
                std::string nom; int pv=0,def=0,atk=0;
                std::getline(f, nom);
                f >> pv; f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                f >> def; f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                f >> atk; f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
                enemies.push_back(CombatEnemy{trim_copy(nom), pv, def, atk, 0, 10});
            }
            scene.blocks.emplace_back(BlockCombat{enemies});
        } else if (l.rfind("*ARME*", 0) == 0 || l.rfind("*ARMURE*", 0) == 0) {
            ItemType t = (l.rfind("*ARME*",0)==0)?ItemType::Arme:ItemType::Armure;
            std::vector<ItemSpec> items;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2);
                if (l2.empty()) break;
                if (!l2.empty() && l2[0]=='*') { f.seekg(pos); break; }
                auto parts = splitSemi(l2);
                if (parts.size() >= 2) {
                    ItemSpec it; it.type = t; it.name = parts[0];
                    try{ it.value = std::stoi(parts[1]); }catch(...){ it.value=0; }
                    if (parts.size() >= 7) {
                        EffectSpec eff; eff.kind = parseEffectKind(parts[3]); eff.label = parts[4];
                        try{ eff.duration = std::stoi(parts[5]); }catch(...){ eff.duration=0; }
                        try{ eff.value = std::stoi(parts[6]); }catch(...){ eff.value=0; }
                        if (eff.kind != EffectKind::None) it.effect = eff;
                    } else if (parts.size() >= 6) {
                        // legacy variant without label
                        EffectSpec eff; eff.kind = parseEffectKind(parts[3]); eff.label = "";
                        try{ eff.duration = std::stoi(parts[4]); }catch(...){ eff.duration=0; }
                        try{ eff.value = std::stoi(parts[5]); }catch(...){ eff.value=0; }
                        if (eff.kind != EffectKind::None) it.effect = eff;
                    }
                    items.push_back(it);
                } else {
                    // Legacy 1-line name + 1-line value
                    std::string nom = trim_copy(l2); int val=0; f >> val; f.ignore(std::numeric_limits<std::streamsize>::max(),'\n');
                    items.push_back(ItemSpec{t, nom, val, std::nullopt});
                }
            }
            if (!items.empty()) scene.blocks.emplace_back(BlockItems{items});
        } else if (l.rfind("*ITEM*", 0) == 0) {
            std::vector<ItemSpec> items;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2);
                if (l2.empty()) break;
                if (!l2.empty() && l2[0]=='*') { f.seekg(pos); break; }
                auto parts = splitSemi(l2);
                if (parts.size() >= 3) items.push_back(parseItemLineGeneric(parts));
            }
            if (!items.empty()) scene.blocks.emplace_back(BlockItems{items});
        } else if (l.rfind("*PATH*", 0) == 0) {
            std::vector<PathOption> options;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2); if (l2.empty()) break;
                if (l2[0]=='*') { f.seekg(pos); break; }
                auto parts = splitSemi(l2);
                if (parts.size() >= 2) { options.push_back(PathOption{parts[0], parts[1]}); }
                else { std::cerr << "Parse warning: malformed PATH at " << filepath << ":" << lineNo << "\n"; }
            }
            if (!options.empty()) scene.blocks.emplace_back(BlockPath{options});
        } else if (l.rfind("*GOLD*", 0) == 0) {
            std::string v; if (std::getline(f, v)) { ++lineNo;
                v = trim_copy(v); int d=0; try{ d = std::stoi(v);}catch(...){ d=0; }
                scene.blocks.emplace_back(BlockGold{d});
            }
        } else if (l.rfind("*SHOP*", 0) == 0) {
            std::vector<ShopItem> items;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2);
                if (l2.empty()) break;
                if (!l2.empty() && l2[0]=='*') { f.seekg(pos); break; }
                auto parts = splitSemi(l2);
                if (parts.size() >= 4) {
                    // Normalized: Prix;Type;Nom;Valeur[;Effet;NomEffet;Durée;ValeurEffet]
                    bool priceFirst = true; int price=0; try{ price = std::stoi(parts[0]); }catch(...){ priceFirst=false; }
                    ItemSpec it;
                    if (priceFirst) {
                        it.type = parseItemType(parts[1]); it.name = parts[2];
                        try{ it.value = std::stoi(parts[3]); }catch(...){ it.value=0; }
                        if (parts.size() >= 8) {
                            EffectSpec eff; eff.kind = parseEffectKind(parts[4]); eff.label = parts[5];
                            try{ eff.duration = std::stoi(parts[6]); }catch(...){ eff.duration=0; }
                            try{ eff.value = std::stoi(parts[7]); }catch(...){ eff.value=0; }
                            if (eff.kind != EffectKind::None) it.effect = eff;
                        }
                    } else {
                        // Back-compat: Type;Nom;Valeur[;Effet;NomEffet;Durée;ValeurEffet];Prix
                        it.type = parseItemType(parts[0]); it.name = parts[1];
                        try{ it.value = std::stoi(parts[2]); }catch(...){ it.value=0; }
                        if (parts.size() >= 7) {
                            EffectSpec eff; eff.kind = parseEffectKind(parts[3]); eff.label = parts[4];
                            try{ eff.duration = std::stoi(parts[5]); }catch(...){ eff.duration=0; }
                            try{ eff.value = std::stoi(parts[6]); }catch(...){ eff.value=0; }
                            if (eff.kind != EffectKind::None) it.effect = eff;
                        }
                        try{ price = std::stoi(parts.back()); }catch(...){ price=0; }
                    }
                    items.push_back(ShopItem{it, price});
                } else { std::cerr << "Parse warning: malformed SHOP at " << filepath << ":" << lineNo << "\n"; }
            }
            if (!items.empty()) scene.blocks.emplace_back(BlockShop{items});
        } else if (l.rfind("*LORE*", 0) == 0) {
            // Lore text block: accumulate lines until blank or next tag
            std::ostringstream oss;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo;
                if (l2.empty()) break;
                std::string t = trim_copy(l2);
                if (!t.empty() && t[0] == '*') { f.seekg(pos); break; }
                oss << l2 << "\n";
            }
            std::string lore = oss.str();
            if (!lore.empty()) scene.blocks.emplace_back(BlockLore{lore});
        } else if (l.rfind("*REMOVE*", 0) == 0) {
            std::vector<RemoveRule> rules;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2);
                if (l2.empty()) break;
                if (!l2.empty() && l2[0]=='*') { f.seekg(pos); break; }
                auto rparts = splitSemi(l2);
                if (rparts.size() < 3) { std::cerr<<"Parse warning: malformed REMOVE at "<<filepath<<":"<<lineNo<<"\n"; continue; }
                std::string sceneTok = rparts[0]; std::string type = rparts[1]; std::string param = rparts[2];
                RemoveRule r;
                if (sceneTok=="this") { r.isThis = true; r.sceneKey.clear(); }
                else { r.isThis = false; r.sceneKey = sceneTok; }
                for (auto& c : type) c=(char)std::toupper((unsigned char)c);
                if (type=="PATH") r.type=RemoveType::Path;
                else if (type=="COMBAT") r.type=RemoveType::Combat;
                else if (type=="ARME") r.type=RemoveType::Arme;
                else if (type=="ARMURE") r.type=RemoveType::Armure;
                else if (type=="GOLD") r.type=RemoveType::Gold;
                else if (type=="SHOP") r.type=RemoveType::Shop;
                else if (type=="LORE") r.type=RemoveType::Lore;
                else { std::cerr<<"Parse warning: unknown REMOVE type at "<<filepath<<":"<<lineNo<<"\n"; continue; }
                r.param = param; rules.push_back(r);
            }
            if (!rules.empty()) scene.blocks.emplace_back(BlockRemove{rules});
        } else if (l.rfind("*ADD*", 0) == 0) {
            std::vector<AddRule> rules;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2);
                if (l2.empty()) break;
                if (!l2.empty() && l2[0]=='*') { f.seekg(pos); break; }
                auto head = splitSemi(l2);
                if (head.size() < 2) { std::cerr<<"Parse warning: malformed ADD at "<<filepath<<":"<<lineNo<<"\n"; continue; }
                std::string sceneTok = head[0]; std::string type = head[1];
                AddRule ar; if (sceneTok=="this") { ar.isThis = true; ar.sceneKey.clear(); } else { ar.isThis=false; ar.sceneKey = sceneTok; }
                for (auto& c : type) c=(char)std::toupper((unsigned char)c);

                // Expect a following header *PATH* / *COMBAT* / *ITEM* or *ARME*/ *ARMURE*
                std::streampos pos2 = f.tellg();
                std::string header;
                while (std::getline(f, header)) { ++lineNo; header = trim_copy(header); if (!header.empty()) break; }
                if (header.empty() || header[0] != '*') { f.seekg(pos2); continue; }

                if (type=="PATH" && header.find("*PATH*")!=std::string::npos) {
                    ar.type = AddType::Path;
                    std::streampos p3;
                    while (true) {
                        p3 = f.tellg();
                        std::string l3; if (!std::getline(f, l3)) break; ++lineNo; l3 = trim_copy(l3); if (l3.empty()) break; if (!l3.empty() && l3[0]=='*'){ f.seekg(p3); break; }
                        auto pparts = splitSemi(l3);
                        if (pparts.size() >= 2) { ar.paths.push_back(PathOption{pparts[0], pparts[1]}); }
                    }
                    rules.push_back(ar);
                } else if (type=="COMBAT" && header.find("*COMBAT*")!=std::string::npos) {
                    ar.type = AddType::Combat;
                    // inline payload allowed: *COMBAT* Nom;PV;DEF;ATK[;GOLD[;SPD[;REVEAL]]]
                    std::string pay = trim_copy(header.substr(header.find("*COMBAT*")+8));
                    if (!pay.empty()) {
                        auto parts = splitSemi(pay); if (parts.size()>=4) {
                            CombatEnemy e{parts[0],0,0,0,0,10};
                            try{e.hp=std::stoi(parts[1]);}catch(...){e.hp=0;}
                            try{e.def=std::stoi(parts[2]);}catch(...){e.def=0;}
                            try{e.atk=std::stoi(parts[3]);}catch(...){e.atk=0;}
                            if (parts.size()>=5) { try{ e.gold = std::stoi(parts[4]); }catch(...){ e.gold = 0; } }
                            if (parts.size()>=6) { try{ e.spd  = std::stoi(parts[5]); }catch(...){ e.spd  = 10; } }
                            if (parts.size()>=7) { e.reveal = parseEnemyReveal(parts[6]); }
                            ar.combats.push_back(e);
                        }
                    } else {
                        // legacy 4 lines
                        std::string nom; int pv=0,def=0,atk=0; std::getline(f, nom); ++lineNo;
                        f >> pv; f.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); ++lineNo;
                        f >> def; f.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); ++lineNo;
                        f >> atk; f.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); ++lineNo;
                        ar.combats.push_back(CombatEnemy{trim_copy(nom), pv, def, atk, 0, 10});
                    }
                    rules.push_back(ar);
                } else if ((type=="ITEM" && header.find("*ITEM*")!=std::string::npos) || header.find("*ARME*")!=std::string::npos || header.find("*ARMURE*")!=std::string::npos) {
                    ar.type = AddType::Item;
                    if (header.find("*ITEM*")!=std::string::npos) {
                        std::streampos p3;
                        while (true) {
                            p3 = f.tellg();
                            std::string l3; if (!std::getline(f, l3)) break; ++lineNo; l3 = trim_copy(l3);
                            if (l3.empty()) break;
                            if (l3[0]=='*'){ f.seekg(p3); break; }
                            auto parts = splitSemi(l3); if (parts.size()>=3) ar.items.push_back(parseItemLineGeneric(parts));
                        }
                    } else {
                        // ARME/ARMURE single inline or legacy 2 lines
                        bool isArme = header.find("*ARME*")!=std::string::npos;
                        std::string pay = trim_copy(header.substr(header.find(isArme?"*ARME*":"*ARMURE*") + (isArme?6:8)));
                        if (!pay.empty()) {
                            auto parts = splitSemi(pay);
                            if (parts.size()>=2) {
                                ItemSpec it; it.type = isArme?ItemType::Arme:ItemType::Armure; it.name = parts[0]; try{ it.value = std::stoi(parts[1]); }catch(...){ it.value=0; }
                                ar.items.push_back(it);
                            }
                        } else {
                            std::string nom; std::getline(f, nom); ++lineNo; int val=0; f >> val; f.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); ++lineNo;
                            ar.items.push_back(ItemSpec{isArme?ItemType::Arme:ItemType::Armure, trim_copy(nom), val, std::nullopt});
                        }
                    }
                    rules.push_back(ar);
                } else if (type=="GOLD" && header.find("*GOLD*")!=std::string::npos) {
                    ar.type = AddType::Gold;
                    std::string v; if (std::getline(f, v)) { ++lineNo; v = trim_copy(v); try{ ar.goldDelta = std::stoi(v);}catch(...){ ar.goldDelta=0; } }
                    rules.push_back(ar);
                } else if (type=="SHOP" && header.find("*SHOP*")!=std::string::npos) {
                    ar.type = AddType::Shop;
                    std::streampos p3;
                    while (true) {
                        p3 = f.tellg();
                        std::string l3; if (!std::getline(f, l3)) break; ++lineNo; l3 = trim_copy(l3);
                        if (l3.empty()) break;
                        if (l3[0]=='*'){ f.seekg(p3); break; }
                        auto parts = splitSemi(l3);
                        if (parts.size() >= 4) {
                            bool priceFirst = true; int price=0; try{ price = std::stoi(parts[0]); }catch(...){ priceFirst=false; }
                            ItemSpec it;
                            if (priceFirst) {
                                it.type = parseItemType(parts[1]); it.name = parts[2]; try{ it.value = std::stoi(parts[3]); }catch(...){ it.value=0; }
                                if (parts.size() >= 8) {
                                    EffectSpec eff; eff.kind = parseEffectKind(parts[4]); eff.label = parts[5];
                                    try{ eff.duration = std::stoi(parts[6]); }catch(...){ eff.duration=0; }
                                    try{ eff.value = std::stoi(parts[7]); }catch(...){ eff.value=0; }
                                    if (eff.kind != EffectKind::None) it.effect = eff;
                                }
                            } else {
                                it.type = parseItemType(parts[0]); it.name = parts[1]; try{ it.value = std::stoi(parts[2]); }catch(...){ it.value=0; }
                                if (parts.size() >= 7) {
                                    EffectSpec eff; eff.kind = parseEffectKind(parts[3]); eff.label = parts[4];
                                    try{ eff.duration = std::stoi(parts[5]); }catch(...){ eff.duration=0; }
                                    try{ eff.value = std::stoi(parts[6]); }catch(...){ eff.value=0; }
                                    if (eff.kind != EffectKind::None) it.effect = eff;
                                }
                                try{ price = std::stoi(parts.back()); }catch(...){ price=0; }
                            }
                            ar.shopItems.push_back(ShopItem{it, price});
                        }
                    }
                    rules.push_back(ar);
                } else if (type=="LORE" && header.find("*LORE*")!=std::string::npos) {
                    ar.type = AddType::Lore;
                    // Read text lines until blank or next tag
                    std::ostringstream lore;
                    std::streampos p3;
                    while (true) {
                        p3 = f.tellg();
                        std::string l3; if (!std::getline(f, l3)) break; ++lineNo;
                        if (l3.empty()) break;
                        std::string t3 = trim_copy(l3);
                        if (!t3.empty() && t3[0]=='*'){ f.seekg(p3); break; }
                        lore << l3 << "\n";
                    }
                    std::string L = lore.str();
                    if (!L.empty()) ar.lores.push_back(L);
                    rules.push_back(ar);
                } else {
                    // mismatch header; rewind
                    f.seekg(pos2);
                }
            }
            if (!rules.empty()) scene.blocks.emplace_back(BlockAdd{rules});
        } else if (l.rfind("*VICTORY*", 0) == 0) {
     scene.blocks.emplace_back(BlockVictory{});
        } else if (l.rfind("*END*", 0) == 0) {
     scene.blocks.emplace_back(BlockGo{});
        } else {
            // Free description is not allowed anymore; ignore stray lines
            std::cerr << "Parse warning: ignored text outside block at " << filepath << ":" << lineNo << "\n";
        }
    }
    return scene;
}

} // namespace scene

