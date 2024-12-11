#ifndef UTILS_H
#define UTILS_H

#include <memory>
#include "Entite.h"

std::unique_ptr<Entite> chargerClasse(const std::string& cheminFichier);

#endif // UTILS_H
