#include "scene.h"

#include <algorithm>
#include <initializer_list>
#include <limits>
#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#define ARRAY_TO_VEC3(arr) glm::vec3((arr)[0], (arr)[1], (arr)[2])
#define READ_INT32(value) file.read(reinterpret_cast<char*>((value)), sizeof(int))
#define READ_FLOAT(value) file.read(reinterpret_cast<char*>((value)), sizeof(float))
#define READ_FLOAT3(value) file.read(reinterpret_cast<char*>((value)), sizeof(float) * 3)
#define VERY_CLOSE(vec1, vec2) glm::length(vec1 - vec2) < 0.001

void load_scene_file(const char* path, Scene* scene) {
    std::ifstream file(path, std::ios::binary);

    float light_dir[3];
    READ_FLOAT3(light_dir);
    scene->light_dir = ARRAY_TO_VEC3(light_dir);

    int brush_count;
    READ_INT32(&brush_count);

    scene->geometry.clear();

    for (int i = 0; i<brush_count; i++) {
        float min_xyz[3];
        float max_xyz[3];
        float rgb[3];

        READ_FLOAT3(min_xyz);
        READ_FLOAT3(max_xyz);
        READ_FLOAT3(rgb);

        Brush brush = Brush(ARRAY_TO_VEC3(min_xyz), ARRAY_TO_VEC3(max_xyz), ARRAY_TO_VEC3(rgb));

        scene->geometry.push_back(brush);
    }

    scene->portal1.width = 1.0f;
    scene->portal1.height = 1.0f;
    scene->portal2.width = 1.0f;
    scene->portal2.height = 1.0f;

    scene->cubes.push_back(Cube(glm::vec3(-10.0f, 10.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f)));
}

Camera::Camera(glm::mat4 transform) {
    this->SetTransform(transform);
}

void Camera::SetTransform(glm::mat4 transform) {
    this->yaw = -glm::sign(transform[0][2]) * glm::degrees(glm::acos(transform[0][0]));
    this->pitch = -glm::sign(transform[2][1]) * glm::degrees(glm::acos(transform[1][1]));
    this->position = glm::vec3(transform[3]);
} 

glm::vec3 Camera::GetForwardDirection() {
    // direction.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    // direction.y = sin(glm::radians(this->pitch));
    // direction.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

    glm::vec3 direction;
    direction.x = -sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    direction.y = sin(glm::radians(this->pitch));
    direction.z = -cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    return direction;
}

glm::vec3 Camera::GetPitchlessForwardDirection() {
    // direction.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    // direction.y = sin(glm::radians(this->pitch));
    // direction.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));

    glm::vec3 direction;
    direction.x = -sin(glm::radians(this->yaw));
    direction.y = 0.0f;
    direction.z = -cos(glm::radians(this->yaw));
    return direction;
}

glm::vec3 Camera::GetRightDirection() {
    return glm::cross(this->GetForwardDirection(), glm::vec3(0.0f, 1.0f, 0.0f));
}

glm::mat4 Camera::GetView() {
    // glm::vec3 direction = this->GetForwardDirection();
    // return glm::lookAt(
    //     this->position, 
    //     this->position + direction, 
    //     glm::vec3(0.0f, 1.0f, 0.0f)
    // );

    glm::mat4 view(1.0f);

    view = glm::rotate(view, glm::radians(-this->pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    view = glm::rotate(view, glm::radians(-this->yaw), glm::vec3(0.0f, 1.0f, 0.0f));

    view = glm::translate(view, -this->position);
    
    return view;
}

glm::mat4 Camera::GetTransform() {
    glm::mat4 rotationMatrix = glm::eulerAngleYX(glm::radians(this->yaw), glm::radians(this->pitch));
    
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    
    return translationMatrix * rotationMatrix;
}

glm::mat4 Cube::GetTransform() {
    return glm::translate(glm::mat4(1.0f), this->position);
}

glm::mat4 portal_rotation(Portal* portal) {
    return glm::eulerAngleYX(glm::atan(portal->normal.x, portal->normal.z), glm::asin(-portal->normal.y));
}

glm::mat4 portal_transform(Portal* portal, Portal* linked_portal) {
    glm::mat4 p1model = glm::translate(glm::mat4(1.0f), portal->position) * portal_rotation(portal);
    glm::mat4 p2model = glm::translate(glm::mat4(1.0f), linked_portal->position) * portal_rotation(linked_portal);
    glm::mat4 p2model_rotated = glm::rotate(p2model, glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    return p2model_rotated * glm::inverse(p1model);
}

glm::mat4 pcam_transform(Camera* real_cam, Portal* portal, Portal* linked_portal) {
    return portal_transform(portal, linked_portal) * real_cam->GetTransform();
}

bool find_portal_intersection(glm::vec3 start, glm::vec3 translation, Portal* portal, glm::vec3* intersection) {
    float dist;
    glm::vec3 dir = glm::normalize(translation);
    if (glm::dot(dir, portal->normal) < 0.0f && glm::intersectRayPlane(start, dir, portal->position, portal->normal, dist)) {
        glm::vec3 planeIntersection = start + dir * dist;
        if (dist < glm::length(translation) + 0.001 && glm::distance(planeIntersection, portal->position) < portal->width) {
            if (intersection) *intersection = planeIntersection;
            return true;
        }
    }

    return false;
}

inline glm::vec3 portal_min(Portal* portal) {
    return portal->position - glm::vec3(portal->width, portal->height, PORTAL_THICKNESS);
}

inline glm::vec3 portal_max(Portal* portal) {
    return portal->position + glm::vec3(portal->width, portal->height, PORTAL_THICKNESS);
}

bool is_in_portal(glm::vec3 point, Portal* portal) {
    glm::vec3 min = portal_min(portal);
    glm::vec3 max = portal_max(portal);

    return point.x > min.x && point.y > min.y && point.z > min.z && point.x < max.x && point.y < max.y && point.z < max.z;
}

bool handle_portal_movement(Camera* cam, glm::vec3 translation, Scene* scene) {
    glm::vec3 intersection;
    bool both_portals_open = portals_open(scene);
    if (both_portals_open && find_portal_intersection(cam->position, translation, &scene->portal1, &intersection)) {
        cam->SetTransform(pcam_transform(cam, &scene->portal1, &scene->portal2));
        std::cout << "P1 -> P2" << std::endl;
        return true;
    } else if (both_portals_open && find_portal_intersection(cam->position, translation, &scene->portal2, &intersection)) {
        cam->SetTransform(pcam_transform(cam, &scene->portal2, &scene->portal1));
        std::cout << "P2 -> P1" << std::endl;
        return true;
    } else {
        return false;
    }
}

// Adapted from https://web.archive.org/web/20090803054252/http://tog.acm.org/resources/GraphicsGems/gems/RayBox.c
bool intersect_AABB(glm::vec3 minB, glm::vec3 maxB, glm::vec3 origin, glm::vec3 dir, glm::vec3* intersection, glm::vec3* normal, glm::vec3* face_min, glm::vec3* face_max)
{
    bool inside = true;
    char quadrant[3];
    int i;
    int whichPlane;
    float maxT[3];
    float candidatePlane[3];
    float coord[3];

    /* Find candidate planes; this loop can be avoided if
    rays cast all from the eye(assume perpsective view) */
    for (i = 0; i < 3; i++) {
        if (origin[i] < minB[i]) {
            quadrant[i] = 1;
            candidatePlane[i] = minB[i];
            inside = false;
        } else if (origin[i] > maxB[i]) {
            quadrant[i] = 0;
            candidatePlane[i] = maxB[i];
            inside = false;
        } else {
            quadrant[i] = 2;
        }
    }

    /* Ray origin inside bounding box */
    if (inside) {
        *intersection = origin;
        return true;
    }

    /* Calculate T distances to candidate planes */
    for (i = 0; i < 3; i++) {
        if (quadrant[i] != 2 && dir[i] != 0.0f)
            maxT[i] = (candidatePlane[i] - origin[i]) / dir[i];
        else
            maxT[i] = -1.0f;
    }

    /* Get largest of the maxT's for final choice of intersection */
    whichPlane = 0;
    for (i = 1; i < 3; i++) {
        if (maxT[whichPlane] < maxT[i])
            whichPlane = i;
    }

    /* Check final candidate actually inside box */
    if (maxT[whichPlane] < 0.0f) return false;
    for (i = 0; i < 3; i++) {
        if (whichPlane != i) {
            coord[i] = origin[i] + maxT[whichPlane] * dir[i];
            if (coord[i] < minB[i] || coord[i] > maxB[i])
                return false;
        } else {
            coord[i] = candidatePlane[i];
        }
    }

    *intersection = glm::vec3(coord[0], coord[1], coord[2]);

    // Determine the normal of the hit surface
    glm::vec3 hitNormal(0.0f, 0.0f, 0.0f);
    glm::vec3 hitFace_min(minB);
    glm::vec3 hitFace_max(maxB);
    if (whichPlane == 0) {
        hitNormal.x = (quadrant[0] == 1) ? -1.0f : 1.0f;
        if (quadrant[0] == 1) {
            hitFace_min.x = minB.x;
            hitFace_max.x = minB.x;
        } else {
            hitFace_min.x = maxB.x;
            hitFace_max.x = maxB.x;
        }
    } else if (whichPlane == 1) {
        hitNormal.y = (quadrant[1] == 1) ? -1.0f : 1.0f;
        if (quadrant[1] == 1) {
            hitFace_min.y = minB.y;
            hitFace_max.y = minB.y;
        } else {
            hitFace_min.y = maxB.y;
            hitFace_max.y = maxB.y;
        }
    } else if (whichPlane == 2) {
        hitNormal.z = (quadrant[2] == 1) ? -1.0f : 1.0f;
        if (quadrant[2] == 1) {
            hitFace_min.z = minB.z;
            hitFace_max.z = minB.z;
        } else {
            hitFace_min.z = maxB.z;
            hitFace_max.z = maxB.z;
        }
    }

    *normal = hitNormal;
    *face_min = hitFace_min;
    *face_max = hitFace_max;

    return true; /* ray hits box */
}

bool raycast(Camera* cam, Scene* scene, RaycastHitInfo* hit_info) {
    bool hit = false;
    for (size_t brush_index = 0; brush_index < scene->geometry.size(); brush_index++) {
        Brush* brush = &scene->geometry[brush_index];

        glm::vec3 brush_intersection;
        glm::vec3 brush_normal;
        glm::vec3 brush_face_min;
        glm::vec3 brush_face_max;
        if (intersect_AABB(brush->min, brush->max, cam->position, cam->GetForwardDirection(), &brush_intersection, &brush_normal, &brush_face_min, &brush_face_max)) {
            if (hit) {
                if (glm::distance(cam->position, brush_intersection) < glm::distance(cam->position, hit_info->intersection)) {
                    hit_info->intersection = brush_intersection;
                    hit_info->normal = brush_normal;
                    hit_info->face_min = brush_face_min;
                    hit_info->face_max = brush_face_max;
                    hit_info->brush = brush;
                }
            } else {
                hit = true;
                hit_info->intersection = brush_intersection;
                hit_info->normal = brush_normal;
                hit_info->face_min = brush_face_min;
                hit_info->face_max = brush_face_max;
                hit_info->brush = brush;
            }
        }
    }

    return hit;
}

bool check_aabb_intersection(glm::vec3 a_min, glm::vec3 a_max, glm::vec3 b_min, glm::vec3 b_max) {
    return (
        a_min.x <= b_max.x &&
        a_max.x >= b_min.x &&
        a_min.y <= b_max.y &&
        a_max.y >= b_min.y &&
        a_min.z <= b_max.z &&
        a_max.z >= b_min.z
    );
}

// Checks if the AABB would collide with the brush if it was to be translated. Returns true and sets the hit_normal if a collision is detected.
bool aabb_brush_collision(glm::vec3 aabb_min, glm::vec3 aabb_max, glm::vec3 translation, Brush* brush, glm::vec3* hit_normal) {
    // Calculate the translated AABB
    glm::vec3 translated_min = aabb_min + translation;
    glm::vec3 translated_max = aabb_max + translation;
    
    // Check if there is a collision
    bool collision = (translated_min.x <= brush->max.x && translated_max.x >= brush->min.x) &&
                     (translated_min.y <= brush->max.y && translated_max.y >= brush->min.y) &&
                     (translated_min.z <= brush->max.z && translated_max.z >= brush->min.z);

    if (!collision) {
        return false;
    }
    
    // Determine the hit normal
    glm::vec3 overlap_min = glm::max(translated_min, brush->min);
    glm::vec3 overlap_max = glm::min(translated_max, brush->max);
    glm::vec3 overlap_size = overlap_max - overlap_min;

    if (overlap_size.x < overlap_size.y && overlap_size.x < overlap_size.z) {
        if (translated_max.x > brush->min.x && translated_min.x < brush->min.x) {
            *hit_normal = glm::vec3(-1, 0, 0);  // Collision on the left face of the brush
        } else {
            *hit_normal = glm::vec3(1, 0, 0);   // Collision on the right face of the brush
        }
    } else if (overlap_size.y < overlap_size.x && overlap_size.y < overlap_size.z) {
        if (translated_max.y > brush->min.y && translated_min.y < brush->min.y) {
            *hit_normal = glm::vec3(0, -1, 0);  // Collision on the bottom face of the brush
        } else {
            *hit_normal = glm::vec3(0, 1, 0);   // Collision on the top face of the brush
        }
    } else {
        if (translated_max.z > brush->min.z && translated_min.z < brush->min.z) {
            *hit_normal = glm::vec3(0, 0, -1);  // Collision on the front face of the brush
        } else {
            *hit_normal = glm::vec3(0, 0, 1);   // Collision on the back face of the brush
        }
    }

    return true;
}

bool portal_aabb_collision_test(Portal* portal, glm::vec3 max, glm::vec3 min) {
    glm::vec3 portal_pos = portal->position;
    if (glm::abs(1.0f - glm::abs(glm::dot(portal->normal, glm::vec3(1,0,0)))) < 0.001) {
        // Portal normal is parallel to x-axis
        return portal_pos.y - portal->height <= min.y && portal_pos.y + portal->height >= max.y && portal_pos.z - portal->width <= min.z && portal_pos.z + portal->width >= max.z;
    } else if (glm::abs(1.0f - glm::abs(glm::dot(portal->normal, glm::vec3(0,1,0)))) < 0.001) {
        // Portal normal is parallel to y-axis
        return portal_pos.x - portal->width <= min.x && portal_pos.x + portal->width >= max.x && portal_pos.z - portal->height <= min.z && portal_pos.z + portal->height >= max.z;
    } else if (glm::abs(1.0f - glm::abs(glm::dot(portal->normal, glm::vec3(0,0,1)))) < 0.001) {
        // Portal normal is parallel to z-axis
        return portal_pos.x - portal->width <= min.x && portal_pos.x + portal->width >= max.x && portal_pos.y - portal->height <= min.y && portal_pos.y + portal->height >= max.y;
    } else {
        assert(false);
    }
}

// Move the camera while handling collision and portal teleportation
void scene_aware_movement(Camera* cam, glm::vec3 translation, Scene* scene, bool* on_ground) {
    *on_ground = false;

    glm::vec3 player_aabb_min = cam->position - glm::vec3(0.2f, 1.5f, 0.2f);
    glm::vec3 player_aabb_max = cam->position + glm::vec3(0.2f);
    glm::vec3 player_center = (player_aabb_min + player_aabb_max) / 2.0f;

    // First run the portal logic
    // This will teleport the camera if it is moving through a portal
    if (!handle_portal_movement(cam, translation, scene)) {
        // If the portal logic did not move the camera, we do a collision check
        for (size_t brush_index = 0; brush_index < scene->geometry.size(); brush_index++) {
            Brush* brush = &scene->geometry[brush_index];

            glm::vec3 hit_normal;
            if (aabb_brush_collision(player_aabb_min, player_aabb_max, translation, brush, &hit_normal)) {
                // If this brush has an open portal that is facing the same way as the hit face
                // and is close enough to the center of the player's AABB, ignore the collision
                if (
                    portals_open(scene) &&
                    (
                        (
                            scene->portal1.brush == brush &&
                            VERY_CLOSE(scene->portal1.normal, hit_normal) &&
                            glm::length(scene->portal1.position - player_center) < scene->portal1.width
                        ) ||
                        (
                            scene->portal2.brush == brush &&
                            VERY_CLOSE(scene->portal2.normal, hit_normal) &&
                            glm::length(scene->portal2.position - player_center) < scene->portal2.width
                        )
                    )
                ) {
                    continue;
                }
                
                glm::vec3 projection = glm::dot(hit_normal, translation) * hit_normal;
                translation = translation - projection;

                if (glm::dot(hit_normal, glm::vec3(0.0f, 1.0f, 0.0f)) > 0.1f) {
                    *on_ground = true;
                }
            }
        }

        // If no collision is detected, just move the camera
        cam->position += translation;
    }
}

void teleport_cube(Cube* cube, Portal* in_portal, Portal* out_portal) {
    glm::mat4 ptransform = portal_transform(in_portal, out_portal);
    glm::mat4 cube_transform = cube->GetTransform();

    glm::mat4 teleported_transform = ptransform * cube_transform;

    cube->position = teleported_transform[3];
    cube->velocity = ptransform * glm::vec4(cube->velocity, 0.0f);
}

glm::vec3 find_holding_position(Camera* cam, Scene* scene, float cube_size) {
    glm::vec3 translation = cam->GetForwardDirection() * HOLDING_DISTANCE;
    glm::vec3 holding_pos = cam->position + translation;

    // Check if there is a portal in the way
    glm::vec3 intersection;
    if (find_portal_intersection(cam->position, translation, &scene->portal1, &intersection)) {
        holding_pos = portal_transform(&scene->portal1, &scene->portal2) * glm::vec4(holding_pos, 1.0f);
    } else if (find_portal_intersection(cam->position, translation, &scene->portal2, &intersection)) {
        holding_pos = portal_transform(&scene->portal2, &scene->portal1) * glm::vec4(holding_pos, 1.0f);
    } else {
        RaycastHitInfo hit;
        if (raycast(cam, scene, &hit) && glm::distance(cam->position, hit.intersection) < HOLDING_DISTANCE) {
            holding_pos = hit.intersection + hit.normal * cube_size;
        }
    }

    return holding_pos;
}

glm::vec3 portal_aware_direction(glm::vec3 start, glm::vec3 target, Scene* scene) {
    glm::vec3 p1transformed_target = portal_transform(&scene->portal1, &scene->portal2) * glm::vec4(target, 1.0f);
    glm::vec3 p2transformed_target = portal_transform(&scene->portal2, &scene->portal1) * glm::vec4(target, 1.0f);

    if (glm::distance(start, target) > glm::distance(start, p2transformed_target) && find_portal_intersection(start, p2transformed_target-start, &scene->portal1, NULL)) {
        // Faster through portal 1
        return p2transformed_target - start;
    } else if (glm::distance(start, target) > glm::distance(start, p1transformed_target) && find_portal_intersection(start, p1transformed_target-start, &scene->portal2, NULL)) {
        // Faster through portal 2
        return p1transformed_target - start;
    } else {
        return target - start;
    }
}

void update_cubes(Scene* scene, Camera* cam, float deltaTime) {
    for (size_t cube_index = 0; cube_index < scene->cubes.size(); cube_index++) {
        Cube* cube = &scene->cubes[cube_index];
        glm::vec3 cube_aabb_min = cube->position - cube->size;
        glm::vec3 cube_aabb_max = cube->position + cube->size;

        if (cube->grabbed) {
            glm::vec3 target_pos = find_holding_position(cam, scene, cube->size);
            cube->velocity = portal_aware_direction(cube->position, target_pos, scene) * 1000.0f * deltaTime;
            // cube->velocity = glm::vec3(0.0f);
            // cube->position = target_pos;
        } else {
            cube->velocity.y += GRAVITY * deltaTime;
        }

        glm::vec3 translation = cube->velocity * deltaTime;

        // Handle portal logic
        glm::vec3 intersection;
        bool both_portals_open = scene->portal1.open && scene->portal2.open;
        if (both_portals_open && find_portal_intersection(cube->position, translation, &scene->portal1, &intersection)) {
            teleport_cube(cube, &scene->portal1, &scene->portal2);
        } else if (both_portals_open && find_portal_intersection(cube->position, translation, &scene->portal2, &intersection)) {
            teleport_cube(cube, &scene->portal2, &scene->portal1);
        }

        cube->in_portal = portals_open(scene) && (
            portal_aabb_collision_test(&scene->portal1, cube_aabb_max, cube_aabb_min) || portal_aabb_collision_test(&scene->portal2, cube_aabb_max, cube_aabb_min)
        );

        translation = cube->velocity * deltaTime;

        // Collision logic
        for (size_t brush_index = 0; brush_index < scene->geometry.size(); brush_index++) {
            Brush* brush = &scene->geometry[brush_index];

            glm::vec3 hit_normal;
            if (aabb_brush_collision(cube_aabb_min, cube_aabb_max, translation, brush, &hit_normal)) {
                // If this brush has an open portal that is facing the same way as the hit face
                // and is close enough to the center of the player's AABB, ignore the collision
                if (
                    portals_open(scene) &&
                    (
                        (
                            scene->portal1.brush == brush &&
                            VERY_CLOSE(scene->portal1.normal, hit_normal) &&
                            glm::length(scene->portal1.position - cube->position) < scene->portal1.width
                        ) ||
                        (
                            scene->portal2.brush == brush &&
                            VERY_CLOSE(scene->portal2.normal, hit_normal) &&
                            glm::length(scene->portal2.position - cube->position) < scene->portal2.width
                        )
                    )
                ) {
                    continue;
                }
                
                glm::vec3 projection = glm::dot(hit_normal, translation) * hit_normal;
                translation = translation - projection;

                if (glm::dot(hit_normal, glm::vec3(0.0f, 1.0f, 0.0f)) > 0.1f) {
                    cube->velocity = glm::vec3(0.0f);
                }
            }
        }

        cube->position += translation;
    }
}

bool portals_open(Scene* scene) {
    return scene->portal1.open && scene->portal2.open;
}