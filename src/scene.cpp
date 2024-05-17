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

glm::mat4 pcam_transform(Camera* real_cam, Portal* portal, Portal* linked_portal) {
    glm::mat4 p1model = glm::translate(glm::mat4(1.0f), portal->position);
    glm::mat4 p2model = glm::translate(glm::mat4(1.0f), linked_portal->position);
    glm::mat4 p2model_rotated = glm::rotate(p2model, glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 cam_model = real_cam->GetTransform();
    
    return p2model_rotated * glm::inverse(p1model) * cam_model;
}

bool find_portal_intersection(glm::vec3 start, glm::vec3 stop, Portal* portal, glm::vec3* intersection) {
    float dist;
    glm::vec3 translationVector = stop - start;
    glm::vec3 dir = glm::normalize(translationVector);
    if (glm::dot(dir, portal->normal) < 0.0f && glm::intersectRayPlane(start, dir, portal->position, portal->normal, dist)) {
        glm::vec3 planeIntersection = start + dir * dist;
        if (dist < glm::length(translationVector) + 0.001 && planeIntersection.x > portal->position.x - portal->width && planeIntersection.x < portal->position.x + portal->width) {
            *intersection = planeIntersection;
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

void portal_aware_movement(Camera* cam, glm::vec3 targetPos, Scene* scene) {
    glm::vec3 intersection;
    if (find_portal_intersection(cam->position, targetPos, &scene->portal1, &intersection)) {
        cam->SetTransform(pcam_transform(cam, &scene->portal1, &scene->portal2));
        std::cout << "P1 -> P2" << std::endl;
    } else if (find_portal_intersection(cam->position, targetPos, &scene->portal2, &intersection)) {
        cam->SetTransform(pcam_transform(cam, &scene->portal2, &scene->portal1));
        std::cout << "P2 -> P1" << std::endl;
    } else {
        cam->position = targetPos;
    }
}