#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "scene.h"

struct BaseShader {
    GLuint program;
    GLuint u_MVP;
    GLuint u_lightdir;
    GLuint u_color;
};

struct Camera {
    glm::vec3 position;
    float yaw;
    float pitch;

    Camera(glm::vec3 position, float yaw, float pitch): position(position), yaw(yaw), pitch(pitch) {} 

    glm::vec3 GetForwardDirection();
    glm::vec3 GetRightDirection();
};

int mkprog(const char* vertex_path, const char* fragment_path);
void render(glm::mat4 projection, Scene* scene, Camera* cam, BaseShader* shader, int depth);