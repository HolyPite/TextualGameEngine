# NomDuProjet (à définir)

## Description

Ce projet est un moteur de jeu textuel permettant de créer et de jouer à des aventures narratives personnalisées. Grâce à un système de fichiers `.txt` structurés, il est possible de définir :

- Des **scènes** : Chaque fichier décrit une étape de l’histoire, avec textes, combats, objets (armes, armures) et choix menant à d’autres scènes.
- Des **classes** : Chaque fichier définit une classe de personnage (PV, PM, Armure, Compétences) que le joueur peut incarner, offrant une expérience de jeu variée.

Le programme lit ces fichiers, interprète leur contenu et exécute l’aventure en temps réel, proposant au joueur de prendre des décisions, combattre des monstres, récupérer des récompenses et progresser dans une histoire immersive.

## Fonctionnalités

- **Scènes dynamiques** : Les scènes, définies dans `data/scene/`, sont reliées par des chemins (`*PATH*`), permettant au joueur de naviguer dans l’histoire.
- **Combats** : Les ennemis, définis par `*COMBAT*`, affrontent le joueur selon leurs statistiques.
- **Équipement** : Les armes (`*ARME*`) et armures (`*ARMURE*`) améliorent les capacités du joueur.
- **Fin de l’histoire** : `*VICTOIRE*` et `*GO*` marquent la fin de l’aventure (victoire, défaite, etc.).
- **Classes personnalisées** : Ajoutez de nouvelles classes dans `data/class/` avec leurs propres PV, PM, Armure et compétences (`*COMP*`).

## Prérequis
- Le dossier `data/` doit être présent dans le même répertoire que l’exécutable `.exe`.
- Aucune installation supplémentaire n’est requise.
- (A noter que le dossier `src/` contient les fichiers .h et .cpp. Ils ne sont **PAS NECESSAIRE** pour l'execution du projet. Ils sont fournis pour ceux qui souhaite voir et modifier les fonctionnement du projet.)

## Installation
1. Téléchargez projet a partir de la dernière [**RELEASE**](https://github.com/PGarn/jeu-cpp/releases/tag/pre-alpha) et extrayez-la.
2. Ajoutez ou modifiez vos fichiers de scènes dans `data/scene/` et vos fichiers de classes dans `data/class/`(Des fichiers de base sont déjà présent pour donner un exemple de jeu).

## Utilisation
1. Assurez-vous que le dossier `data/` se trouve dans le même répertoire que l’exécutable `.exe`.
2. Lancez simplement le fichier `.exe`.
3. Le programme charge automatiquement les classes et scènes disponibles.
4. Choisissez votre classe, suivez les instructions à l’écran, prenez des décisions, et profitez de l’aventure.

## Personnalisation
- **Ajouter de nouvelles scènes** : Créez un fichier `.txt` dans `data/scene/` et suivez le format défini dans le guide.
- **Ajouter de nouvelles classes** : Créez un fichier `.txt` dans `data/class/` et définissez PV, PM, ARM, COMP. La classe sera automatiquement proposée lors du choix initial.

## Contributions
- Vos contributions sont les bienvenues !
- Proposez de nouvelles idées, corrigez des bugs ou améliorez la documentation via issues ou pull requests.

## Licence

Ce projet est sous licence [MIT](https://opensource.org/licenses/MIT).  
Vous pouvez donc utiliser, modifier et distribuer ce projet librement, tant que vous conservez la mention d'origine.

---

Ce projet est en cours de développement. N’hésitez pas à apporter vos idées et suggestions !
