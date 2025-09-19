#include "SceneWorld.h"
#include "SceneParser.h"

#include <filesystem>
#include <iostream>

namespace scene {

const Scene* SceneWorld::getScene(const std::string& key) const {
    auto it = sceneMap.find(key);
    if (it == sceneMap.end()) {
        return nullptr;
    }
    return &it->second;
}

static bool isSceneFile(const std::filesystem::directory_entry& entry) {
    return entry.is_regular_file() && entry.path().extension() == ".txt";
}

SceneWorld loadSceneWorld(const std::filesystem::path& sceneDirectory) {
    SceneWorld world;

    if (!std::filesystem::exists(sceneDirectory)) {
        world.diag.errors.push_back("Scene directory not found: " + sceneDirectory.string());
        return world;
    }

    std::filesystem::path canonicalRoot;
    try {
        canonicalRoot = std::filesystem::weakly_canonical(sceneDirectory);
    } catch (...) {
        canonicalRoot = sceneDirectory;
    }

    for (std::filesystem::recursive_directory_iterator it(canonicalRoot), end; it != end; ++it) {
        const auto& entry = *it;
        if (entry.is_directory()) {
            if (entry.path().filename() == "old_scenes") {
                it.disable_recursion_pending();
            }
            continue;
        }

        if (!isSceneFile(entry)) {
            continue;
        }

        std::string key = sceneKeyFromPath(entry.path(), canonicalRoot);
        Scene parsed = parse(entry.path().string());
        if (parsed.blocks.empty()) {
            world.diag.errors.push_back("Failed to parse scene: " + entry.path().string());
            continue;
        }

        parsed.path = entry.path().string();
        auto existing = world.sceneMap.find(key);
        if (existing != world.sceneMap.end()) {
            world.diag.warnings.push_back("Duplicate scene key '" + key + "' replaced with file " + entry.path().string());
            existing->second = std::move(parsed);
        } else {
            world.sceneMap.emplace(key, std::move(parsed));
        }
    }

    if (world.sceneMap.empty() && world.diag.errors.empty()) {
        world.diag.warnings.push_back("No scene files found in directory: " + sceneDirectory.string());
    }

    return world;
}

std::string sceneKeyFromPath(const std::filesystem::path& path, const std::filesystem::path& root) {
    std::filesystem::path relative;
    try {
        relative = std::filesystem::relative(path, root);
    } catch (...) {
        relative = path;
        if (relative.has_root_path()) {
            relative = relative.filename();
        }
    }
    if (relative.empty()) {
        relative = path.stem();
    }
    relative.replace_extension();
    std::string key = relative.generic_string();
    if (key.empty()) {
        key = path.stem().string();
    }
    return key;
}

} // namespace scene

