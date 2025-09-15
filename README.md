# Text RPG Engine

Ce projet est un moteur de jeu textuel piloté par des fichiers .txt structurés. Il permet d’écrire des aventures avec scènes, choix, combats, objets et boutiques, puis de les jouer dans la console avec un rendu "écriture humaine".

## Lancer le jeu

- Place l’exécutable et le dossier data/ au même niveau.
- Démarre sur la scène data/scenes/start.txt par défaut.
- Option CLI: --start=<sceneKey> pour un autre point d’entrée (clé = nom de fichier sans .txt).

Exemple: jeu.exe --start=marche

## Organisation des données

`
data/
  class/          # Fichiers de classes jouables
  scenes/         # Fichiers de scènes (clé = nom de fichier sans .txt)
`

## Scènes – Spécification

Règles strictes:
- Un fichier .txt = une scène. La clé de scène = nom du fichier (sans .txt).
- Tous les blocs de contenu doivent être avant *PATH*.
- Il ne doit y avoir qu’un seul bloc *PATH* par scène.
- Les blocs *REMOVE* et *ADD* doivent être après *PATH*.
- Séparateurs obligatoires: ; (pas d’espaces comme séparateurs).
- Pas de description libre hors blocs: tout texte affichable doit être dans un bloc (*LORE*, etc.).
- Fins d’histoire: *VICTORY* ou *END* (terminent immédiatement la partie). Pas d’anciens *VICTOIRE*/*GO*.

Ordre minimal recommandé:
`
[*LORE* | *COMBAT* | *ITEM* | *SHOP* | *GOLD* | *VICTORY* | *END*] ... (0..n)

*PATH*
<id>;texte
...

*REMOVE* ... (0..n)
*ADD*    ... (0..n)
`

Blocs disponibles:

- LORE
`
*LORE*
Texte sur une ou plusieurs lignes...
`

- COMBAT
`
*COMBAT*
Nom;PV;DEF;ATK[;GOLD[;SPD[;REVEAL]]]
`
REVEAL: MIN | FULL | HIDE (visibilité des stats adverses)

- ITEM (équipement)
`
*ITEM*
Type;Nom;Valeur[;Effet;NomEffet;Durée;ValeurEffet]
`
Type: ARME | ARMURE — Effet: DOT | BUFF_DEF | NONE

- SHOP (boutique)
`
*SHOP*
Prix;Type;Nom;Valeur[;Effet;NomEffet;Durée;ValeurEffet]
`

- GOLD (delta d’or)
`
*GOLD*
<+n|-n>
`

- PATH (un seul par scène)
`
*PATH*
sceneKey;Texte du choix
...
`
sceneKey = nom du fichier cible sans .txt (ex: marche, 	oit, 	our_de_guet)

- REMOVE (applique des suppressions dynamiques)
`
*REMOVE*
<this|sceneKey>;TYPE;param
`
TYPE = PATH | COMBAT | ARME | ARMURE | GOLD | SHOP | LORE  
param pour PATH = id du choix (la sceneKey utilisée dans le *PATH*)

- ADD (ajoute du contenu dynamiquement)
`
*ADD*
<this|sceneKey>;PATH
*PATH*
sceneKey;Texte

<this|sceneKey>;COMBAT
*COMBAT*
Nom;PV;DEF;ATK[;GOLD[;SPD[;REVEAL]]]

<this|sceneKey>;ITEM
*ITEM*
Type;Nom;Valeur[;Effet;NomEffet;Durée;ValeurEffet]

<this|sceneKey>;SHOP
*SHOP*
Prix;Type;Nom;Valeur[;Effet;NomEffet;Durée;ValeurEffet]

<this|sceneKey>;LORE
*LORE*
Texte...
`

## Classes – Spécification

Chaque fichier de data/class/ décrit une classe.

Format moderne recommandé:
`
NomDeClasse

PV;120
PM;40
ARM;15

COMP
Charge;Attaque;+;20;10
Bouclier;Protection;%;50;15
Soin;Soin;+;18;8
`
- COMP liste des compétences: Nom;Type;ValeurType;Valeur;Mana
  - Type: Attaque | Soin | Protection
  - ValeurType: + (valeur fixe) ou % (pourcentage)

## Exemple minimal de scène
`
*LORE*
Vous ouvrez la porte.

*PATH*
marche;Rejoindre le marché animé.
atelier;Forcer l'atelier fermé.

*REMOVE*
this;PATH;atelier
`

## Conseils
- Utilisez des sceneKey explicites (ex: entrepot, marche, caves).
- Préférez plusieurs *LORE* courts à un gros bloc pour rythmer l’affichage.
- *VICTORY*/*END* n’ayant pas de contenu, placez-les quand vous voulez conclure la scène (sans *PATH* ensuite).

## Développement
- --start=<sceneKey> pour démarrer ailleurs que start.
- L’UI applique un affichage mot‑à‑mot global et lettre‑par‑lettre pour *LORE*.
- Le moteur ignore tout texte hors blocs (warnings de parse si présent).
