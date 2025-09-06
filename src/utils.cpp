#include "utils.h"
#include "Competence.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <tuple>
#include <vector>
#include <algorithm>
#include <limits>
#include <cctype>

static std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    return (start == std::string::npos || end == std::string::npos) ? "" : str.substr(start, end - start + 1);
}

static std::vector<std::string> splitSemi(const std::string& s) {
    std::vector<std::string> out; std::string cur;
    for (char ch : s) {
        if (ch == ';') { out.push_back(trim(cur)); cur.clear(); }
        else cur.push_back(ch);
    }
    out.push_back(trim(cur));
    return out;
}

static SkillType parseSkillType(const std::string& s) {
    if (s == "Attaque") return SkillType::Attaque;
    if (s == "Soin") return SkillType::Soin;
    return SkillType::Protection;
}

static ValueType parseValueType(char c) {
    return (c == '%') ? ValueType::Percent : ValueType::Flat;
}

static ValueType parseValueTypeToken(const std::string& s) {
    if (s.empty()) return ValueType::Flat;
    char c = s[0];
    if (c == '%' || s == "%" || s == "Percent" || s == "PERCENT") return ValueType::Percent;
    return ValueType::Flat; // '+', 'Flat', etc.
}

std::unique_ptr<Entite> chargerClasse(const std::string& cheminFichier) {
    std::ifstream fichier(cheminFichier);
    if (!fichier.is_open()) {
        throw std::runtime_error("Impossible d'ouvrir le fichier : " + cheminFichier);
    }

    std::string ligne, nomClasse;
    int pv = 0, pm = 0, armure = 0;
    int atk = 5, spd = 10;
    std::vector<Competence> competences;
    bool nomClasseDefini = false;

    while (std::getline(fichier, ligne)) {
        ligne = trim(ligne);

        if (!nomClasseDefini && !ligne.empty()) {
            nomClasse = ligne;
            nomClasseDefini = true;
            continue;
        }
        if (ligne.empty()) {
            continue;
        }
        // New semicolon style for stats: PV;100 / PM;50 / ARM;10
        if (ligne.find(';') != std::string::npos) {
            auto parts = splitSemi(ligne);
            if (parts.size() >= 2) {
                std::string key = parts[0];
                if (key == "PV") { try { pv = std::stoi(parts[1]); } catch(...) {} continue; }
                if (key == "PM") { try { pm = std::stoi(parts[1]); } catch(...) {} continue; }
                if (key == "ARM") { try { armure = std::stoi(parts[1]); } catch(...) {} continue; }
                if (key == "ATK") { try { atk = std::stoi(parts[1]); } catch(...) {} continue; }
                if (key == "SPD") { try { spd = std::stoi(parts[1]); } catch(...) {} continue; }
            }
        }
        if (ligne == "COMP") {
            while (std::getline(fichier, ligne)) {
                ligne = trim(ligne);
                if (ligne.empty()) break;
                // New one-line semicolon style: Nom;Type;ValueType;Valeur;Mana
                if (ligne.find(';') != std::string::npos) {
                    auto parts = splitSemi(ligne);
                    if (parts.size() >= 5) {
                        std::string nomCompetence = parts[0];
                        SkillType st = parseSkillType(parts[1]);
                        ValueType vt = parseValueTypeToken(parts[2]);
                        int valeur = 0, coutMana = 0;
                        try { valeur = std::stoi(parts[3]); } catch(...) {}
                        try { coutMana = std::stoi(parts[4]); } catch(...) {}
                        competences.emplace_back(nomCompetence, st, vt, valeur, coutMana);
                        continue;
                    }
                }
                // Ignore malformed/non-semicolon lines in COMP
            }
        }
    }

    auto classe = std::make_unique<Entite>(nomClasse.empty() ? "Inconnu" : nomClasse, pv, pm, armure);
    for (const auto& competence : competences) {
        classe->ajouterCompetence(competence);
    }
    classe->setAtkBase(atk);
    classe->setSpdBase(spd);
    return classe;
}
