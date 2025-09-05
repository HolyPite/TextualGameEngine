# Guide de création de scènes (fichiers .txt)

Ce guide explique comment écrire des scènes lisibles par le moteur, et comment utiliser les directives dynamiques REMOVE/ADD.

## 1. Principes
- Chaque fichier `.txt` représente une scène; le nom du fichier (ex: `12.txt`) est son identifiant.
- Les scènes sont reliées par des choix déclarés avec `*PATH*`.
- Les blocs commencent par `*...*`. Le moteur lit un bloc jusqu'à une ligne vide ou une nouvelle balise `*...*`.
- Placez une ligne vide entre description et blocs pour la lisibilité.

Répertoire attendu: `data/scenes/` au même niveau que l'exécutable.

## 2. Blocs disponibles

### PATH
Liste des options vers d'autres scènes.

Format:
```
*PATH*
<id> <texte>
<id> <texte>
```

### COMBAT
Déclenche un combat (syntaxe stricte par ligne).
```
*COMBAT*
Nom;PV;DEF;ATK[;GOLD]
Nom;PV;DEF;ATK[;GOLD]
```
• `GOLD` est optionnel (butin gagné à la victoire, défaut 0).

### ITEM (équipement)
Ajoute un ou plusieurs équipements (syntaxe stricte 7 champs par ligne).
```
*ITEM*
Type;Nom;Valeur;Effet;NomEffet;Durée;ValeurEffet
```
- Type: `ARME` ou `ARMURE`
- Effet: `DOT` ou `BUFF_DEF` ou `NONE`
- NomEffet: libellé affiché (ex: Poison, Bénédiction)
- Durée: 0 pour passif (BUFF_DEF), >0 pour temporaire
- ValeurEffet: intensité de l'effet

Exemples:
```
*ITEM*
ARME;Sabre ligotant;4;DOT;Poison;3;2
ARMURE;Veste de cuir;1;BUFF_DEF;Cuir;0;1
```

### VICTOIRE / GO
Fin de l'histoire (victoire) ou fin immédiate.

### GOLD / SHOP
Gestion de l'or et boutique.
```
*GOLD*
<+n|-n>

*SHOP*
ARME;Nom;Valeur;Prix
ARMURE;Nom;Valeur;Prix
```

## 3. Directives dynamiques

### REMOVE
Supprime une option PATH, un COMBAT, un item, etc.
```
*REMOVE*
<sceneId|this> <TYPE> <param>
```
- TYPE = PATH, param = id de la cible.
- TYPE = COMBAT, param = nom exact du monstre.
- TYPE = ARME/ARMURE, param = nom exact de l'objet.

Exemples:
```
*REMOVE*
4 PATH 9
this ARMURE Bouclier léger
```

Placez `*REMOVE*` après les événements pour qu'il soit pris en compte avant le changement de scène.

### ADD
Ajoute du contenu à une scène cible.
```
*ADD*
<this|sceneId> PATH
*PATH*
<id> <texte>

<this|sceneId> COMBAT
*COMBAT*
Nom;PV;DEF;ATK[;GOLD]

<this|sceneId> ITEM
*ITEM*
Type;Nom;Valeur;Effet;NomEffet;Durée;ValeurEffet
```
Notes:
- Les ajouts sont fusionnés avec le contenu existant de la scène cible.
- Les suppressions `*REMOVE*` s'appliquent aussi aux ajouts.

## 4. Bonnes pratiques
- Ordre recommandé: Description -> COMBAT -> ARME/ARMURE -> PATH -> REMOVE/ADD.
- Séparez les blocs par des lignes vides.
- Évitez les boucles sans issue; offrez toujours un retour possible.
- Pour rendre un loot unique: ajoutez `*REMOVE* this ARME/ARMURE <Nom>` juste après le bloc.
- Pour remplacer un chemin: `*ADD*` (nouveau PATH) + `*REMOVE*` (ancien PATH).

## 5. Exemple complet
```
Vous entrez dans l'atelier. Une table croule sous les plans.

*COMBAT*
Garde automatique;14;3;4

*ITEM*
ARMURE;Plastron d'airain;3;BUFF_DEF;Airain;0;1

*PATH*
7 Prendre l'ascenseur vers le toit.
2 Revenir au marché.

*REMOVE*
this ARMURE Plastron d'airain
2 PATH 6

*ADD*
2 PATH
*PATH*
4 Ouvrir la grille de l'atelier.
```

