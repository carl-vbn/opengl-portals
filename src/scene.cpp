#include "scene.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>
#include <glm/gtx/intersect.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>

#define ARRAY_TO_VEC3(arr) glm::vec3((arr)[0], (arr)[1], (arr)[2])
#define READ_INT32(value) file.read(reinterpret_cast<char*>((value)), sizeof(int))
#define READ_FLOAT(value) file.read(reinterpret_cast<char*>((value)), sizeof(float))
#define READ_FLOAT3(value) file.read(reinterpret_cast<char*>((value)), sizeof(float) * 3)

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

    // Hard code portals for now
    scene->portal1 = Portal(glm::vec3(-8.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f);
    scene->portal2 = Portal(glm::vec3(8.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 1.0f, 1.0f);
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

glm::mat4 Camera::GetLocalToWorldMatrix() {
    glm::mat4 rotationMatrix = glm::eulerAngleXY(glm::radians(this->pitch), glm::degrees(this->yaw));
    
    glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);
    
    return translationMatrix * rotationMatrix;
}

bool find_portal_intersection(glm::vec3 start, glm::vec3 stop, Portal* portal, glm::vec3* intersection) {
    float dist;
    glm::vec3 translationVector = stop - start;
    glm::vec3 dir = glm::normalize(translationVector);
    if (glm::intersectRayPlane(start, dir, portal->position, portal->normal, dist)) {
        glm::vec3 planeIntersection = start + dir * dist;
        if (dist < glm::length(translationVector) + 0.001 && planeIntersection.x < portal->position.x + portal->width) {
            *intersection = planeIntersection;
            return true;
        }
    }

    return false;
}

void portal_aware_movement(Camera* cam, glm::vec3 targetPos, Scene* scene) {
    glm::vec3 intersection;
    if (find_portal_intersection(cam->position, targetPos, &scene->portal1, &intersection)) {
        // glm::vec3 remainingTranslation = targetPos - intersection;
        // remainingTranslation.z *= -1.0f;
        cam->position = scene->portal2.position;
        cam->yaw += 180.0f;
    } else {
        cam->position = targetPos;
    }
}