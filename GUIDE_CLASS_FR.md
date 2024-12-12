# Guide : Créer une nouvelle classe (fichiers `.txt`)

## Principe général
Chaque fichier `.txt` correspond à une classe jouable du jeu. En ajoutant un fichier au dossier prévu, le jeu détecte automatiquement la nouvelle classe.

## Structure d’un fichier de classe

Un fichier de classe contient les informations suivantes, dans cet ordre :

1. **Nom de la classe**  
   Exemple : `Paladin`

2. **PV (Points de Vie)**  
   Indique les PV max de la classe.  
   Exemple :  
   ```
   PV
   120
   ```

3. **PM (Points de Mana)**  
   Indique les PM max de la classe.  
   Exemple :  
   ```
   PM
   40
   ```

4. **ARM (Armure)**  
   Indique l’armure de base de la classe.  
   Exemple :  
   ```
   ARM
   15
   ```

5. **COMP (Compétences)**  
   Liste les compétences de la classe. Chaque compétence comprend :  
   - Nom de la compétence  
   - Type : `Attaque`, `Soin` ou `Protection`  
   - TypeValeur : `+` (valeur fixe) ou `%` (pourcentage)  
   - Valeur : Intensité de l’effet (dégâts, soin, protection)  
   - Mana : Coût en PM pour utiliser la compétence

   Exemple :  
   ```
   COMP
   Châtiment Divin
       Attaque
       +
       25
       10
   Bouclier Sacré
       Protection
       %
       50
       15
   Lumière Guérisseuse
       Soin
       +
       20
       5
   ```

## Exemple de fichier complet
```
Paladin

PV
120

PM
40

ARM
15

COMP
Châtiment Divin
    Attaque
    +
    25
    10
Bouclier Sacré
    Protection
    %
    50
    15
Lumière Guérisseuse
    Soin
    +
    20
    5
```

## Intégration au jeu
- Placez le fichier `.txt` de la classe dans `data/class/`.
- Au lancement du jeu, la nouvelle classe sera détectée et proposée au joueur.
