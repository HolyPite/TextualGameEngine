#ifndef HISTOIRE_H
#define HISTOIRE_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include <unordered_set>
#include <unordered_map>
#include <filesystem>
#include "Entite.h"

class Histoire {
private:
    std::unique_ptr<Entite> hero;
    std::string sceneActuelle;
    // Runtime removals and additions
    std::unordered_set<std::string> removals; // key: sceneId:TYPE:param
    std::filesystem::path cachePath;

    struct CombatDef { std::string nom; int pv; int def; int atk; };
    struct ItemDef { std::string type; std::string nom; int valeur; };
    struct SceneAdditions {
        std::vector<std::pair<int,std::string>> paths;
        std::vector<CombatDef> combats;
        std::vector<ItemDef> items;
    };
    std::unordered_map<int, SceneAdditions> additions;

public:
    Histoire(std::unique_ptr<Entite> hero) : hero(std::move(hero)), sceneActuelle("data/scenes/0.txt") {}

    void jouer();

private:
    void chargerScene();
    void afficherDescription(std::string& description);
    void gererCombat(const std::string& nomMonstre, int pv, int attaque, int defense);
    void gererEquipement(const std::string& type, const std::string& nom, int valeur);
    // Directives
    void appliquerRemoveDirective(std::ifstream& fichier);
    void appliquerAddDirective(std::ifstream& fichier);
    bool isRemoved(int sceneId, const std::string& type, const std::string& param) const;
    static int extractSceneId(const std::string& path);
    void addRemoval(int sceneId, const std::string& type, const std::string& param);
};

#endif // HISTOIRE_H
