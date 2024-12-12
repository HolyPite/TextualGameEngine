# Step-by-Step Story Creation Guide (Text Files)

## 1. General Principle
- Each `.txt` file represents one scene of the story.
- The files are numbered and linked together by choices (e.g., `0.txt`, `1.txt`, `2.txt`…).
- The player progresses from file to file based on their decisions.

## 2. Available Keywords
Keywords are written in uppercase, enclosed in asterisks. They define specific actions or game elements.

### PATH
Offers a choice leading to other scenes.

**Format:**
```
*PATH*
X Choice description 1
Y Choice description 2
```
Where `X` and `Y` are the file numbers that the choices lead to (e.g., `1.txt`, `2.txt`).

**Example:**
```
*PATH*
7 Take the left path.
6 Take the right path.
```

**Display:**
```
1. Take the left path.
2. Take the right path.
```

### COMBAT
Initiates a battle against a monster.

**Format:**
```
*COMBAT*
Monster_Name
Health_Points
Armor_Points
Attack_Points
```

**Example:**
```
*COMBAT*
Mimic
15
1
3
```

### ARME (Weapon)
Adds a weapon to the player’s inventory, increasing their attack power.

**Format:**
```
*ARME*
Weapon_Name
Attack_Bonus
```

**Example:**
```
*ARME*
Rusty Sword
2
```

### ARMURE (Armor)
Adds an armor piece to the player’s inventory, increasing their protection.

**Format:**
```
*ARMURE*
Armor_Name
Protection_Bonus
```

**Example:**
```
*ARMURE*
Light Armor
3
```

### VICTOIRE (Victory)
Indicates the end of the story or a major victory.

**Format:**
```
*VICTOIRE*
```

### GO
Abruptly ends the story (defeat, abandonment, etc.).

**Format:**
```
*GO*
```

## 3. Recommended Scene File Structure
1. **Scene Description:** Set the scene (environment, ambiance).
2. **Events:** If there is a battle, place the `*COMBAT*` block after the description.
3. **Rewards:** After the battle, if a weapon or armor is found, use `*ARME*` or `*ARMURE*`.
4. **Choices:** Offer options, then use `*PATH*` to indicate paths to other files.

## 4. Design Tips
- **Avoid infinite loops:** Each choice should lead to a conclusion, a new event, or a logical way back.
- **Consistent rewards:** Weapons and armor should be balanced with the difficulty.
- **Variety in situations:** Include battles, discoveries, rewards, and descriptions to maintain player interest.

## 5. Example

**File 0.txt:**
```
You start in a dense forest. Two paths lie before you.

*PATH*
1 Take the left path.
2 Take the right path.
```

**File 1.txt:**
```
A wolf stands before you, ready to attack.

*COMBAT*
Wolf
10
1
3

You defeat the wolf and find a rusty sword.

*ARME*
Rusty Sword
2

*PATH*
3
```

**File 2.txt:**
```
You emerge into a sunny clearing. In the center, you see a shining piece of armor.

*ARMURE*
Light Armor
3

*PATH*
0 Go back.
4 Move forward.
```
