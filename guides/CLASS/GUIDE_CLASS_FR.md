# Guide : Créer une nouvelle classe (fichiers `.txt`)

## Principe général
Chaque fichier `.txt` correspond à une classe jouable. En ajoutant un fichier au dossier prévu, le jeu détecte automatiquement la nouvelle classe.

## Structure d’un fichier de classe

Un fichier de classe contient les informations suivantes, dans cet ordre :

1. Nom de la classe (ligne simple)
   - Exemple : `Paladin`

2. Stats (format moderne recommandé, en lignes séparées)
   - `PV;120`
   - `PM;40`
   - `ARM;15`

   Le format historique avec une étiquette sur une ligne puis la valeur sur la suivante reste pris en charge (compatibilité) :
   ```
   PV
   120
   PM
   40
   ARM
   15
   ```

3. COMP (Compétences)
   Format moderne en une ligne par compétence, séparée par `;` :
   ```
   COMP
   Nom;Type;ValeurType;Valeur;Mana
   ```
   - `Type`: `Attaque`, `Soin`, `Protection`
   - `ValeurType`: `+` (valeur fixe) ou `%` (pourcentage)
   - `Valeur`: intensité (dégâts/soin/protection)
   - `Mana`: coût en PM

   Exemple (moderne) :
   ```
   COMP
   Châtiment Divin;Attaque;+;25;10
   Bouclier Sacré;Protection;%;50;15
   Lumière Guérisseuse;Soin;+;20;5
   ```

   Exemple (historique, encore supporté) :
   ```
   COMP
   Châtiment Divin
       Attaque
       +
       25
       10
   ```

## Exemple de fichier complet (moderne)
```
Paladin

PV;120
PM;40
ARM;15

COMP
Châtiment Divin;Attaque;+;25;10
Bouclier Sacré;Protection;%;50;15
Lumière Guérisseuse;Soin;+;20;5
```

## Intégration au jeu
- Placez le fichier `.txt` de la classe dans `data/class/`.
- Au lancement du jeu, la nouvelle classe est détectée et proposée au joueur.

