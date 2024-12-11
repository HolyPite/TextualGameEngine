#ifndef HISTOIRE_H
#define HISTOIRE_H

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <memory>
#include "Entite.h"

class Histoire {
private:
    std::unique_ptr<Entite> hero;
    std::string sceneActuelle;

public:
    Histoire(std::unique_ptr<Entite> hero) : hero(std::move(hero)), sceneActuelle("data/scenes/0.txt") {}

    void jouer();

private:
    void chargerScene();
    void gererCombat(const std::string& nomMonstre, int pv, int attaque);
    void gererEquipement(const std::string& type, const std::string& nom, int valeur);
};

#endif // HISTOIRE_H
