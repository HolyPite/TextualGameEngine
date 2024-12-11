# Guide de création d’une histoire par étapes

#### 1. Principe général
- Chaque fichier .txt représente une scène de l’histoire.
- Les fichiers sont numérotés et reliés entre eux par des choix.
- Le joueur progresse en ouvrant les fichiers dans l’ordre dicté par ses décisions.

#### 2. Mots-clés disponibles
**Les mots-clés s’écrivent en majuscules, encadrés d’astérisques.**

1. **PATH**  
   Permet de proposer un choix menant à un ou plusieurs fichiers suivants.  
   *Exemple :*  
   ```
   1. Entrer dans la salle sombre.
   2. Retourner sur vos pas.

   *PATH*
   6
   7
   ```
   Les numéros (6, 7) renvoient aux noms de fichiers correspondants (6.txt, 7.txt).

2. **COMBAT**  
   Initialise un combat contre un monstre.  
   *Format :*
   ```
   *COMBAT*
   Nom_du_monstre
   Points_de_vie
   Points_d’attaque
   ```
   *Exemple :*  
   ```
   *COMBAT*
   Mimic
   15
   3
   ```

3. **ARME**  
   Ajoute une arme à l’inventaire du joueur, améliorant son attaque.  
   *Format :*
   ```
   *ARME*
   Nom_de_l’Arme
   Bonus_d’attaque
   ```
   *Exemple :*
   ```
   *ARME*
   Epée Rouillée
   2
   ```

4. **ARMURE**  
   Ajoute une armure à l’inventaire du joueur, améliorant sa protection.  
   *Format :*
   ```
   *ARMURE*
   Nom_de_l’Armure
   Bonus_de_protection
   ```
   *Exemple :*
   ```
   *ARMURE*
   Armure Légère
   3
   ```

5. **VICTOIRE**  
   Indique la fin de l’histoire ou une victoire majeure.  
   *Exemple :*
   ```
   *VICTOIRE*
   ```

6. **GO**  
   Met fin à l’histoire de manière abrupte (défaite, abandon, etc.).  
   *Exemple :*
   ```
   *GO*
   ```

#### 3. Structure recommandée d’un fichier de scène
- **Description** : Commencez par décrire la scène, l’environnement, les personnages ou objets présents.
- **Événements** : S’il y a un combat, placez le bloc *COMBAT* après la description.
- **Récompenses** : S’il y a une arme ou une armure à gagner, placez les blocs *ARME* ou *ARMURE* après le combat ou l’action liée.
- **Choix de direction** : Proposez des options au joueur, puis utilisez *PATH* pour indiquer vers quels fichiers aller.

#### 4. Conseils de conception
- Évitez les boucles sans fin : chaque choix doit mener à une nouvelle scène, une conclusion ou un retour en arrière cohérent.
- Assurez-vous que la difficulté des combats correspond aux récompenses offertes.
- Variez les ambiances, les descriptions et les types d’événements (combats, découvertes, récompenses) pour maintenir l’intérêt.

#### 5. Exemple de départ
**Fichier 0.txt :**  
```
Vous commencez dans une forêt dense. Deux sentiers s’offrent à vous.

1. Prendre à gauche.
2. Prendre à droite.

*PATH*
1
2
```

**Fichier 1.txt :**  
```
Un loup se dresse devant vous, prêt à attaquer.

*COMBAT*
Loup
10
3

Vous vainquez le loup et trouvez une épée rouillée.

*ARME*
Epée Rouillée
2

1. Continuer.
*PATH*
3
```

**Fichier 2.txt :**  
```
Vous débouchez sur une clairière ensoleillée. Au centre, une armure scintille.

*ARMURE*
Armure Légère
3

1. Revenir sur vos pas.
2. Avancer plus loin.
*PATH*
0
4
```

---

Ce guide récapitulatif devrait vous aider à créer, structurer et lier vos fichiers d’histoire simplement et efficacement.
