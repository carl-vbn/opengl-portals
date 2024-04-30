#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "scene.h"

struct BaseShader {
    GLuint program;
    GLuint u_MVP;
    GLuint u_lightdir;
    GLuint u_color;
};

int mkprog(const char* vertex_path, const char* fragment_path);
glm::mat4 mkproj(glm::vec3 cam_pos, float yaw, float pitch, float fov, float aspect_ratio);
void render(glm::mat4 projection, Scene* scene, BaseShader* shader, int depth);