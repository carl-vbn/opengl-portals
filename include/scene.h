#pragma once

#include <vector>

#include "mesh.h"

#define PORTAL_THICKNESS 0.1f
#define GRAVITY -8.0f
#define GRAB_REACH 5.0f
#define HOLDING_DISTANCE 3.0f
#define PRINT_VEC3(vec3) std::cout << (vec3).x << " " << (vec3).y << " " << (vec3).z << std::endl

struct Brush {
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
    Brush* brush;

    Portal() : open(false), spawn_time(0.0f), position(glm::vec3(0.0f)), normal(glm::vec3(0.0f)), width(0.0f), height(0.0f) {}
    Portal(glm::vec3 position, glm::vec3 normal, float width, float height) : open(true), position(position), normal(normal), width(width), height(height) {}
};

struct Cube {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec3 color;
    float size;
    bool grabbed;

    Cube(glm::vec3 position, glm::vec3 color): position(position), velocity(glm::vec3(0.0f)), color(color), size(0.25f), grabbed(false) {}
    
    glm::mat4 GetTransform();
};

struct Scene {
    std::vector<Brush> geometry;
    std::vector<Cube> cubes;
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
    glm::vec3 GetPitchlessForwardDirection();
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
    Brush* brush;
};

/** General **/
void load_scene_file(const char* path, Scene* scene);

/** Portal related **/
glm::mat4 portal_rotation(Portal* portal);
glm::mat4 portal_transform(Portal* portal, Portal* linked_portal);
glm::mat4 pcam_transform(Camera* real_cam, Portal* portal, Portal* linked_portal);
bool portals_open(Scene* scene);
bool is_in_portal(glm::vec3 point, Portal* portal);
bool portal_aabb_collision_test(Portal* portal, glm::vec3 min, glm::vec3 max);

/** Movement and Physics **/
bool check_aabb_intersection(glm::vec3 a_min, glm::vec3 a_max, glm::vec3 b_min, glm::vec3 b_max);
bool aabb_brush_collision(glm::vec3 aabb_min, glm::vec3 aabb_max, glm::vec3 translation, Brush* brush, glm::vec3* hit_normal);
bool raycast(Camera* cam, Scene* scene, RaycastHitInfo* hit_info);
void scene_aware_movement(Camera* cam, glm::vec3 translation, Scene* scene, bool* on_ground);
void update_cubes(Scene* scene, Camera* camera, float deltaTime);