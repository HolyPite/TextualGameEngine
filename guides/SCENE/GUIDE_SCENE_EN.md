# Scene Authoring Guide (text files)

This guide explains how to write scenes the engine can read, and how to use dynamic directives REMOVE/ADD.

## 1. Basics
- Each `.txt` file is a scene; the file name (e.g. `12.txt`) is its id.
- Scenes are linked by choices declared with `*PATH*`.
- Blocks start with `*...*`. The engine reads a block until a blank line or the next `*...*` marker.
- Put a blank line between description and blocks for readability.

Expected folder: `data/scenes/` next to the executable.

## 2. Available blocks

### PATH
List options to other scenes.
```
*PATH*
<id> <text>
<id> <text>
```

### COMBAT
Trigger a battle.
```
*COMBAT*
<Name>
<HP>
<DEF>
<ATK>
```

### ARME / ARMURE (Weapon / Armor)
Add an equipment item.
```
*ARME*
<Name>
<Value>

*ARMURE*
<Name>
<Value>
```

### VICTOIRE / GO
End of story (victory) or abrupt end.

## 3. Dynamic directives

### REMOVE
Remove a PATH option, a COMBAT, or a loot.
```
*REMOVE*
<sceneId|this> <TYPE> <param>
```
- TYPE = PATH, param = target id.
- TYPE = COMBAT, param = exact monster name.
- TYPE = ARME/ARMURE, param = exact item name.

Examples:
```
*REMOVE*
4 PATH 9
this ARMURE Light Shield
```
Place `*REMOVE*` after events so it gets applied before leaving the scene.

### ADD
Add content to a target scene.
```
*ADD*
<this|sceneId> PATH
*PATH*
<id> <text>

<this|sceneId> COMBAT
*COMBAT*
<Name>
<HP>
<DEF>
<ATK>

<this|sceneId> ARME  # or ARMURE
*ARME*
<Name>
<Value>
```
Notes:
- Additions are merged with the target scene content.
- `*REMOVE*` also applies to added content.

## 4. Best practices
- Recommended order: Description -> COMBAT -> ARME/ARMURE -> PATH -> REMOVE/ADD.
- Separate blocks with blank lines.
- Avoid dead loops; always offer a way back or a conclusion.
- To make a loot one-shot: add `*REMOVE* this ARME/ARMURE <Name>` right after the block.
- To replace a path: use `*ADD*` (new PATH) + `*REMOVE*` (old PATH).

## 5. Full example
```
You enter the workshop. Plans cover the table.

*COMBAT*
Auto Guard
14
3
4

*ARMURE*
Bronze Cuirass
3

*PATH*
7 Take the lift to the roof.
2 Return to the market.

*REMOVE*
this ARMURE Bronze Cuirass
2 PATH 6

*ADD*
2 PATH
*PATH*
4 Open the bent gate.
```
