#pragma once

#include <vector>

#include "mesh.h"

struct Brush
{
    glm::vec3 min;
    glm::vec3 max;

    glm::vec3 color;

    Brush(glm::vec3 min, glm::vec3 max, glm::vec3 color) : min(min), max(max), color(color) {}
};

struct Portal {
    glm::vec3 position;
    glm::vec3 normal;
    float width;
    float height;

    Portal(glm::vec3 position, glm::vec3 normal, float width, float height) : position(position), normal(normal), width(width), height(height) {}
};

struct Scene {
    std::vector<Brush*> geometry;
    std::vector<Portal*> portals;
    glm::vec3 light_dir;
    double time;
};