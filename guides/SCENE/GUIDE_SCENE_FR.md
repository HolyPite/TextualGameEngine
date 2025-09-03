# Guide de creation de scenes (fichiers .txt)

Ce guide explique comment ecrire des scenes lisibles par le moteur, et comment utiliser les directives dynamiques REMOVE/ADD.

## 1. Principes
- Chaque fichier `.txt` represente une scene; le nom du fichier (ex: `12.txt`) est son identifiant.
- Les scenes sont reliees par des choix declares avec `*PATH*`.
- Les blocs commencent par `*...*`. Le moteur lit un bloc jusqu'a une ligne vide ou une nouvelle balise `*...*`.
- Placez une ligne vide entre description et blocs pour la lisibilite.

Repertoire attendu: `data/scenes/` au meme niveau que l'executable.

## 2. Blocs disponibles

### PATH
Liste des options vers d'autres scenes.

Format:
```
*PATH*
<id> <texte>
<id> <texte>
```

### COMBAT
Declenche un combat.
```
*COMBAT*
<Nom>
<PV>
<DEF>
<ATK>
```

### ARME / ARMURE
Ajoute un equipement.
```
*ARME*
<Nom>
<Valeur>

*ARMURE*
<Nom>
<Valeur>
```

### VICTOIRE / GO
Fin de l'histoire (victoire) ou fin immediate.

## 3. Directives dynamiques

### REMOVE
Supprime une option PATH, un COMBAT, ou un loot.
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
this ARMURE Bouclier leger
```

Placez `*REMOVE*` apres les evenements pour qu'il soit pris en compte avant le changement de scene.

### ADD
Ajoute du contenu a une scene cible.
```
*ADD*
<this|sceneId> PATH
*PATH*
<id> <texte>

<this|sceneId> COMBAT
*COMBAT*
<Nom>
<PV>
<DEF>
<ATK>

<this|sceneId> ARME  # ou ARMURE
*ARME*
<Nom>
<Valeur>
```
Notes:
- Les ajouts sont fusionnes avec le contenu existant de la scene cible.
- Les suppressions `*REMOVE*` s'appliquent aussi aux ajouts.

## 4. Bonnes pratiques
- Ordre recommande: Description -> COMBAT -> ARME/ARMURE -> PATH -> REMOVE/ADD.
- Separez les blocs par des lignes vides.
- Evitez les boucles sans issue; offrez toujours un retour possible.
- Pour rendre un loot unique: ajoutez `*REMOVE* this ARME/ARMURE <Nom>` juste apres le bloc.
- Pour remplacer un chemin: `*ADD*` (nouveau PATH) + `*REMOVE*` (ancien PATH).

## 5. Exemple complet
```
Vous entrez dans l'atelier. Une table croule sous les plans.

*COMBAT*
Garde automatique
14
3
4

*ARMURE*
Plastron d'airain
3

*PATH*
7 Prendre l'ascenseur vers le toit.
2 Revenir au marche.

*REMOVE*
this ARMURE Plastron d'airain
2 PATH 6

*ADD*
2 PATH
*PATH*
4 Ouvrir la grille de l'atelier.
```
