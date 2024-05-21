#pragma once

#include <vector>

#include "mesh.h"

#define PORTAL_THICKNESS 0.1f

struct Brush
{
    glm::vec3 min;
    glm::vec3 max;

    glm::vec3 color;

    Brush(glm::vec3 min, glm::vec3 max, glm::vec3 color) : min(min), max(max), color(color) {}
};

struct Portal {
    bool open;
    float spawn_time;
    glm::vec3 position;
    glm::vec3 normal;
    float width;
    float height;

    Portal() : open(false), spawn_time(0.0f), position(glm::vec3(0.0f)), normal(glm::vec3(0.0f)), width(0.0f), height(0.0f) {}
    Portal(glm::vec3 position, glm::vec3 normal, float width, float height) : open(true), position(position), normal(normal), width(width), height(height) {}
};

struct Scene {
    std::vector<Brush> geometry;
    Portal portal1;
    Portal portal2;
    glm::vec3 light_dir;
    double time;
};

struct Camera {
    glm::vec3 position;
    float yaw;
    float pitch;

    Camera(glm::vec3 position, float yaw, float pitch): position(position), yaw(yaw), pitch(pitch) {}
    Camera(glm::mat4 transform);

    glm::vec3 GetForwardDirection();
    glm::vec3 GetRightDirection();

    glm::mat4 GetView();
    glm::mat4 GetTransform();

    void SetTransform(glm::mat4 transform);
};

struct RaycastHitInfo {
    glm::vec3 intersection;
    glm::vec3 normal;
    glm::vec3 face_min;
    glm::vec3 face_max;
};

void load_scene_file(const char* path, Scene* scene);
glm::mat4 portal_rotation(Portal* portal);
glm::mat4 pcam_transform(Camera* real_cam, Portal* portal, Portal* linked_portal);
bool is_in_portal(glm::vec3 point, Portal* portal);
void portal_aware_movement(Camera* cam, glm::vec3 targetPos, Scene* scene);
bool raycast(Camera* cam, Scene* scene, RaycastHitInfo* hit_info);