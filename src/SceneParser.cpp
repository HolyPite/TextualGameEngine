#include "SceneParser.h"
#include <fstream>
#include <sstream>
#include <algorithm>
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
    return EffectKind::None;
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
    std::string descBuf;
    long long lineNo = 0;

    auto flushDesc = [&](){
        if (!descBuf.empty()) {
            scene.blocks.emplace_back(BlockDescription{descBuf});
            descBuf.clear();
        }
    };

    while (std::getline(f, line)) { ++lineNo;
        if (line.empty()) { descBuf += "\n"; continue; }
        std::string l = trim_copy(line);
        if (l.empty()) { descBuf += "\n"; continue; }

        if (l.rfind("*COMBAT*", 0) == 0) {
            flushDesc();
            std::vector<CombatEnemy> enemies;
            // Try strict multi-line Nom;PV;DEF;ATK[;GOLD] until blank or next tag
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
                enemies.push_back(CombatEnemy{trim_copy(nom), pv, def, atk, 0});
            }
            scene.blocks.emplace_back(BlockCombat{enemies});
        } else if (l.rfind("*ARME*", 0) == 0 || l.rfind("*ARMURE*", 0) == 0) {
            flushDesc();
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
            flushDesc();
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
            flushDesc();
            std::vector<PathOption> options;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; if (l2.empty()) break;
                if (l2[0]=='*') { f.seekg(pos); break; }
                std::istringstream iss(l2);
                int id; std::string rest;
                if (iss >> id) {
                    std::getline(iss, rest);
                    if (!rest.empty() && rest[0]==' ') rest.erase(0,1);
                    options.push_back(PathOption{id, trim_copy(rest)});
                } else { std::cerr << "Parse warning: malformed PATH at " << filepath << ":" << lineNo << "\n"; }
            }
            if (!options.empty()) scene.blocks.emplace_back(BlockPath{options});
        } else if (l.rfind("*GOLD*", 0) == 0) {
            flushDesc();
            std::string v; if (std::getline(f, v)) { ++lineNo;
                v = trim_copy(v); int d=0; try{ d = std::stoi(v);}catch(...){ d=0; }
                scene.blocks.emplace_back(BlockGold{d});
            }
        } else if (l.rfind("*SHOP*", 0) == 0) {
            flushDesc();
            std::vector<ShopItem> items;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2);
                if (l2.empty()) break;
                if (!l2.empty() && l2[0]=='*') { f.seekg(pos); break; }
                auto parts = splitSemi(l2);
                // Accept both legacy 4 fields and generic 7 fields for effects
                if (parts.size() >= 4) {
                    ItemSpec it;
                    it.type = parseItemType(parts[0]);
                    it.name = parts[1];
                    try{ it.value = std::stoi(parts[2]); }catch(...){ it.value=0; }
                    if (parts.size() >= 7) {
                        EffectSpec eff; eff.kind = parseEffectKind(parts[3]); eff.label = parts[4];
                        try{ eff.duration = std::stoi(parts[5]); }catch(...){ eff.duration=0; }
                        try{ eff.value = std::stoi(parts[6]); }catch(...){ eff.value=0; }
                        if (eff.kind != EffectKind::None) it.effect = eff;
                    }
                    int price = 0; try{ price = std::stoi(parts.back()); }catch(...){ price = 0; }
                    items.push_back(ShopItem{it, price});
                } else { std::cerr << "Parse warning: malformed SHOP at " << filepath << ":" << lineNo << "\n"; }
            }
            if (!items.empty()) scene.blocks.emplace_back(BlockShop{items});
        } else if (l.rfind("*REMOVE*", 0) == 0) {
            flushDesc();
            std::vector<RemoveRule> rules;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2);
                if (l2.empty()) break;
                if (!l2.empty() && l2[0]=='*') { f.seekg(pos); break; }
                std::istringstream iss(l2);
                std::string sceneTok, type, param; if (!(iss>>sceneTok>>type)) continue;
                std::getline(iss, param); if (!param.empty() && param[0]==' ') param.erase(0,1);
                RemoveRule r;
                if (sceneTok=="this") { r.isThis = true; r.sceneId=0; }
                else { r.isThis = false; try{ r.sceneId = std::stoi(sceneTok);}catch(...){ r.sceneId=0; } }
                for (auto& c : type) c=(char)std::toupper((unsigned char)c);
                if (type=="PATH") r.type=RemoveType::Path; else if (type=="COMBAT") r.type=RemoveType::Combat; else if (type=="ARME") r.type=RemoveType::Arme; else if (type=="ARMURE") r.type=RemoveType::Armure; else if (type=="GOLD") r.type=RemoveType::Gold; else if (type=="SHOP") r.type=RemoveType::Shop; else { std::cerr<<"Parse warning: unknown REMOVE type at "<<filepath<<":"<<lineNo<<"\n"; continue; }
                r.param = param; rules.push_back(r);
            }
            if (!rules.empty()) scene.blocks.emplace_back(BlockRemove{rules});
        } else if (l.rfind("*ADD*", 0) == 0) {
            flushDesc();
            std::vector<AddRule> rules;
            std::streampos pos;
            while (true) {
                pos = f.tellg();
                std::string l2; if (!std::getline(f, l2)) break; ++lineNo; l2 = trim_copy(l2);
                if (l2.empty()) break;
                if (!l2.empty() && l2[0]=='*') { f.seekg(pos); break; }
                std::istringstream iss(l2);
                std::string sceneTok, type; if (!(iss>>sceneTok>>type)) continue;
                AddRule ar; if (sceneTok=="this") { ar.isThis = true; ar.sceneId = 0; } else { ar.isThis=false; try{ ar.sceneId=std::stoi(sceneTok);}catch(...){ar.sceneId=0;} }
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
                        std::string l3; if (!std::getline(f, l3)) break; ++lineNo; if (l3.empty()) break; if (!l3.empty() && l3[0]=='*'){ f.seekg(p3); break; }
                        std::istringstream iss2(l3);
                        int id; std::string rest; if (iss2>>id){ std::getline(iss2, rest); if (!rest.empty() && rest[0]==' ') rest.erase(0,1); ar.paths.push_back(PathOption{id, trim_copy(rest)}); }
                    }
                    rules.push_back(ar);
                } else if (type=="COMBAT" && header.find("*COMBAT*")!=std::string::npos) {
                    ar.type = AddType::Combat;
                    // inline payload allowed: *COMBAT* Nom;PV;DEF;ATK
                    std::string pay = trim_copy(header.substr(header.find("*COMBAT*")+8));
                    if (!pay.empty()) {
                        auto parts = splitSemi(pay); if (parts.size()>=4) {
                            CombatEnemy e{parts[0],0,0,0,0};
                            try{e.hp=std::stoi(parts[1]);}catch(...){e.hp=0;}
                            try{e.def=std::stoi(parts[2]);}catch(...){e.def=0;}
                            try{e.atk=std::stoi(parts[3]);}catch(...){e.atk=0;}
                            if (parts.size()>=5) { try{ e.gold = std::stoi(parts[4]); }catch(...){ e.gold = 0; } }
                            ar.combats.push_back(e);
                        }
                    } else {
                        // legacy 4 lines
                        std::string nom; int pv=0,def=0,atk=0; std::getline(f, nom); ++lineNo;
                        f >> pv; f.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); ++lineNo;
                        f >> def; f.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); ++lineNo;
                        f >> atk; f.ignore(std::numeric_limits<std::streamsize>::max(),'\n'); ++lineNo;
                        ar.combats.push_back(CombatEnemy{trim_copy(nom), pv, def, atk, 0});
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
                            ItemSpec it; it.type = parseItemType(parts[0]); it.name = parts[1];
                            try{ it.value = std::stoi(parts[2]); }catch(...){ it.value=0; }
                            if (parts.size() >= 7) {
                                EffectSpec eff; eff.kind = parseEffectKind(parts[3]); eff.label = parts[4];
                                try{ eff.duration = std::stoi(parts[5]); }catch(...){ eff.duration=0; }
                                try{ eff.value = std::stoi(parts[6]); }catch(...){ eff.value=0; }
                                if (eff.kind != EffectKind::None) it.effect = eff;
                            }
                            int price=0; try{ price = std::stoi(parts.back()); }catch(...){ price=0; }
                            ar.shopItems.push_back(ShopItem{it, price});
                        }
                    }
                    rules.push_back(ar);
                } else {
                    // mismatch header; rewind
                    f.seekg(pos2);
                }
            }
            if (!rules.empty()) scene.blocks.emplace_back(BlockAdd{rules});
        } else if (l.rfind("*VICTOIRE*", 0) == 0) {
            flushDesc(); scene.blocks.emplace_back(BlockVictory{});
        } else if (l.rfind("*GO*", 0) == 0) {
            flushDesc(); scene.blocks.emplace_back(BlockGo{});
        } else {
            // part of description
            descBuf += line + "\n";
        }
    }
    flushDesc();
    return scene;
}

} // namespace scene
