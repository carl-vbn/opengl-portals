#pragma once

#include <vector>

#include "mesh.h"

struct Scene {
    std::vector<Brush*> geometry;
    glm::vec3 light_dir;
    double time;
};

void load_scene(Scene* scene);
void unload_scene(Scene* scene);