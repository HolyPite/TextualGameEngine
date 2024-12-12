# Guide: Creating a New Class (`.txt` Files)

## General Principle
Each `.txt` file corresponds to a playable class in the game. Adding a class file to the designated folder makes it automatically available in the game.

## Class File Structure

Each class file must contain the following information, in this order:

1. **Class Name**  
   Example: `Paladin`

2. **HP (Health Points)**  
   Defines the class’s maximum HP.  
   Example:  
   ```
   PV
   120
   ```

3. **MP (Mana Points)**  
   Defines the class’s maximum MP.  
   Example:  
   ```
   PM
   40
   ```

4. **ARM (Armor)**  
   Defines the class’s base armor value.  
   Example:  
   ```
   ARM
   15
   ```

5. **COMP (Skills)**  
   Lists the class’s skills. Each skill includes:  
   - Skill Name  
   - Type: `Attaque` (Attack), `Soin` (Heal), or `Protection`  
   - TypeValue: `+` (fixed value) or `%` (percentage)  
   - Value: The skill’s effect intensity (damage, healing, protection)  
   - Mana: The MP cost to use the skill

   Example:  
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

## Complete Example
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

## Integrating into the Game
- Place the class `.txt` file in the `data/class/` folder.
- When the game starts, the new class will be detected and offered to the player.
