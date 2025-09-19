#ifndef SCENE_WORLD_H
#define SCENE_WORLD_H

#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

#include "Scene.h"

namespace scene {

struct SceneDiagnostics {
    std::vector<std::string> warnings;
    std::vector<std::string> errors;
};

class SceneWorld {
public:
    SceneWorld() = default;

    const Scene* getScene(const std::string& key) const;
    const std::unordered_map<std::string, Scene>& scenes() const { return sceneMap; }

    SceneDiagnostics& diagnostics() { return diag; }
    const SceneDiagnostics& diagnostics() const { return diag; }

private:
    friend SceneWorld loadSceneWorld(const std::filesystem::path& sceneDirectory);

    std::unordered_map<std::string, Scene> sceneMap;
    SceneDiagnostics diag;
};

SceneWorld loadSceneWorld(const std::filesystem::path& sceneDirectory);

std::string sceneKeyFromPath(const std::filesystem::path& path, const std::filesystem::path& root);

} // namespace scene

#endif // SCENE_WORLD_H

