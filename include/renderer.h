#pragma once

#include <glm/gtc/matrix_transform.hpp>

#include "scene.h"

struct StandardShader {
    GLuint program;
    GLuint u_MVP;
    GLuint u_lightdir;
    GLuint u_color;
};

struct ScreenShader {
    GLuint program;
    GLuint u_screentex;
};

struct PortalShader {
    GLuint program;
    GLuint u_rendertex;
    GLuint u_MVP;
};

struct Camera {
    glm::vec3 position;
    float yaw;
    float pitch;

    Camera(glm::vec3 position, float yaw, float pitch): position(position), yaw(yaw), pitch(pitch) {} 

    glm::vec3 GetForwardDirection();
    glm::vec3 GetRightDirection();
};

struct RenderTarget {
    GLuint fbo;
    GLuint rbo;
    GLuint texture;
};

namespace renderer {
    int setup(int scr_width, int scr_height, float fov);
    void dispose();
    int load_shader(const char* vertex_path, const char* fragment_path);
    int gen_rendertarget(RenderTarget* target, int width, int height);
    void del_rendertarget(RenderTarget* target);
    void render_scene(Scene* scene, Camera* cam, glm::mat4 projection);
    void render_screen(Scene* scene, Camera* cam);
}