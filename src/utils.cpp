#include "utils.h"
#include "Competence.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <algorithm>
#include <cctype>

static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

static SkillType parseSkillType(const std::string& s) {
    if (s == "Attaque") return SkillType::Attaque;
    if (s == "Soin") return SkillType::Soin;
    return SkillType::Protection;
}

static ValueType parseValueType(char c) {
    return (c == '%') ? ValueType::Percent : ValueType::Flat;
}

std::unique_ptr<Entite> chargerClasse(const std::string& cheminFichier) {
    std::ifstream fichier(cheminFichier);
    if (!fichier.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + cheminFichier);
    }

    std::string ligne, nomClasse;
    int pv = 0, pm = 0, armure = 0;
    std::vector<Competence> competences;
    bool nomClasseDefini = false;

    while (std::getline(fichier, ligne)) {
        ligne = trim(ligne);

        if (!nomClasseDefini && !ligne.empty()) {
            nomClasse = ligne;
            nomClasseDefini = true;
            continue;
        }
        if (ligne == "PV") {
            fichier >> pv; fichier.ignore();
        } else if (ligne == "PM") {
            fichier >> pm; fichier.ignore();
        } else if (ligne == "ARM") {
            fichier >> armure; fichier.ignore();
        } else if (ligne == "COMP") {
            while (std::getline(fichier, ligne) && !ligne.empty()) {
                std::string nomCompetence = trim(ligne);
                std::string type; char typeValeur; int valeur, coutMana;
                std::getline(fichier, type);
                fichier >> typeValeur >> valeur >> coutMana;
                fichier.ignore();
                competences.emplace_back(nomCompetence, parseSkillType(trim(type)), parseValueType(typeValeur), valeur, coutMana);
            }
        }
    }

    auto classe = std::make_unique<Entite>(nomClasse.empty() ? "Inconnu" : nomClasse, pv, pm, armure);
    for (const auto& competence : competences) {
        classe->ajouterCompetence(competence);
    }
    return classe;
}

