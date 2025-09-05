#ifndef SCENE_PARSER_H
#define SCENE_PARSER_H

#include <string>
#include "Scene.h"

namespace scene {
    // Parse a scene file (UTF-8 text) into a structured representation preserving block order.
    Scene parse(const std::string& filepath);

    // Helpers to convert strings to enums used in Scene
    ItemType parseItemType(const std::string& s); // "ARME"/"ARMURE"/"arme"/"armure"
    EffectKind parseEffectKind(const std::string& s); // "DOT"/"BUFF_DEF"/"NONE"
}

#endif // SCENE_PARSER_H

