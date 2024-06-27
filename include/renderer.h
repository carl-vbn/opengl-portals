#pragma once

#include "scene.h"

struct StandardShader {
    GLuint program;
    GLuint u_M;
    GLuint u_MVP;
    GLuint u_lightdir;
    GLuint u_color;
    GLuint u_highlightfrontface;
    GLuint u_slicepos;
    GLuint u_slicenormal;
};

struct ScreenShader {
    GLuint program;
    GLuint u_screentex;
    GLuint u_transform;
    GLuint u_aspectratio;
};

struct PortalShader {
    GLuint program;
    GLuint u_rendertex;
    GLuint u_MVP;
    GLuint u_color;
    GLuint u_outradius;
    GLuint u_inradius;
};

struct RenderTarget {
    GLuint fbo;
    GLuint rbo;
    GLuint texture;
};

namespace renderer {
    int setup(int scr_width, int scr_height, float fov);
    void update_screen_size(int scr_width, int scr_height, float fov);
    void dispose();
    int load_shader(const char* vertex_path, const char* fragment_path);
    int gen_rendertarget(RenderTarget* target, int width, int height, bool fpbuff=false);
    void del_rendertarget(RenderTarget* target);
    void render_scene(Scene* scene, glm::mat4 view, glm::mat4 projection, bool draw_portals, glm::vec3 slice_pos, glm::vec3 slice_normal);
    void render_screen(Scene* scene, Camera* cam);

    extern bool debug_cube_xray;
    extern bool show_pcam_povs;
}