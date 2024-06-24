#include "renderer.h"

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtx/euler_angles.hpp>

#define LOAD_SHADERPRG(struct_instance, dirname) struct_instance.program = load_shader("res/shaders/" dirname "/vertex.glsl", "res/shaders/" dirname "/fragment.glsl")
#define LOCATE_UNIFORM(shader, uniform) shader.uniform = glGetUniformLocation(shader.program, #uniform)

namespace renderer {
    StandardShader standard_shader;
    ScreenShader screen_shader;
    PortalShader portal_shader;
    glm::mat4 projection;
    RenderTarget main_target;
    RenderTarget portal1_target, portal2_target;
    glm::mat4 debug_cube_transform(1.0f);
    bool debug_cube_xray = false;
    bool show_pcam_povs = false;

    // Build an OpenGL Shader progam from a vertex shader and a fragment shader
    int load_shader(const char* vertex_path, const char* fragment_path) {
        std::ifstream vertex(vertex_path);
        std::stringstream vertex_src;
        vertex_src << vertex.rdbuf();

        std::ifstream fragment(fragment_path);
        std::stringstream fragment_src;
        fragment_src << fragment.rdbuf();

        // Vertex shader
        unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
        
        {
            const std::string& tmp = vertex_src.str();   
            const char* cstr = tmp.c_str();
            glShaderSource(vertexShader, 1, &cstr, NULL);
        }

        // Vertex shader compilation
        glCompileShader(vertexShader);
        int success;
        char infoLog[512];
        glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        }
        
        // Fragment shader
        unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        
        {
            const std::string& tmp = fragment_src.str();   
            const char* cstr = tmp.c_str();
            glShaderSource(fragmentShader, 1, &cstr, NULL);
        }

        // Fragment shader compilation
        glCompileShader(fragmentShader);
        glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        }

        // Linking
        unsigned int shaderProgram = glCreateProgram();
        glAttachShader(shaderProgram, vertexShader);
        glAttachShader(shaderProgram, fragmentShader);
        glLinkProgram(shaderProgram);

        glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);

        return shaderProgram;
    }

    int gen_rendertarget(RenderTarget* target, int width, int height, bool fpbuff) {
        glGenFramebuffers(1, &target->fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, target->fbo);

        glGenTextures(1, &target->texture);
        glBindTexture(GL_TEXTURE_2D, target->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, fpbuff ? GL_RGBA16F : GL_RGB, width, height, 0, GL_RGB, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target->texture, 0);

        glGenRenderbuffers(1, &target->rbo);
        glBindRenderbuffer(GL_RENDERBUFFER, target->rbo); 
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);  
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, target->rbo);
        if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Renderbuffer failure" << std::endl;
            return 1;
        }

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Framebuffer failure" << std::endl;
            return 1;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return 0;
    }

    void del_rendertarget(RenderTarget* target) {
        glDeleteFramebuffers(1, &target->fbo);
        glDeleteRenderbuffers(1, &target->rbo);
        glDeleteTextures(1, &target->texture);
    }

    int setup(int scr_width, int scr_height, float fov) {
        LOAD_SHADERPRG(standard_shader, "standard");
        LOCATE_UNIFORM(standard_shader, u_M);
        LOCATE_UNIFORM(standard_shader, u_MVP);
        LOCATE_UNIFORM(standard_shader, u_color);
        LOCATE_UNIFORM(standard_shader, u_lightdir);
        LOCATE_UNIFORM(standard_shader, u_highlightfrontface);
        LOCATE_UNIFORM(standard_shader, u_slicepos);
        LOCATE_UNIFORM(standard_shader, u_slicenormal);

        LOAD_SHADERPRG(screen_shader, "screen");
        LOCATE_UNIFORM(screen_shader, u_screentex);
        LOCATE_UNIFORM(screen_shader, u_transform);
        LOCATE_UNIFORM(screen_shader, u_aspectratio);

        LOAD_SHADERPRG(portal_shader, "portal");
        LOCATE_UNIFORM(portal_shader, u_rendertex);
        LOCATE_UNIFORM(portal_shader, u_color);
        LOCATE_UNIFORM(portal_shader, u_MVP);
        LOCATE_UNIFORM(portal_shader, u_outradius);
        LOCATE_UNIFORM(portal_shader, u_inradius);

        projection = glm::perspective(fov, (float)scr_width/scr_height, 0.1f, 100.0f);

        gen_rendertarget(&main_target, scr_width, scr_height);
        gen_rendertarget(&portal1_target, scr_width, scr_height);
        gen_rendertarget(&portal2_target, scr_width, scr_height);
        
        glEnable(GL_CULL_FACE);

        return 0;
    }

    void dispose() {
        glDeleteProgram(standard_shader.program);
        glDeleteProgram(screen_shader.program);
        del_rendertarget(&main_target);
        del_rendertarget(&portal1_target);
        del_rendertarget(&portal2_target);
    }

    void render_portal(Portal* portal, Scene* scene, glm::mat4 view, glm::vec3 color) {
        glm::mat4 model = glm::scale(glm::translate(glm::mat4(1.0f), portal->position - portal->normal * PORTAL_THICKNESS) * portal_rotation(portal), glm::vec3(portal->width, portal->height, PORTAL_THICKNESS));
        glm::mat4 mvp = projection * view * model;
        glUniformMatrix4fv(portal_shader.u_MVP, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform3f(portal_shader.u_color, color.r, color.g, color.b);

        float age = (float)scene->time - portal->spawn_time;
        if (age > 1.0f) {
            glUniform1f(portal_shader.u_outradius, 1.0f);
            glUniform1f(portal_shader.u_inradius, 0.8f);
        } else {
            glUniform1f(portal_shader.u_outradius, 1.0f - glm::exp(-5.0f * age));
            glUniform1f(portal_shader.u_inradius, 0.8f / (glm::exp(6.90675f - 16.0f * age) + 1.0f));
        }
        
        glDrawElements(GL_TRIANGLES, CUBE_VERTEX_COUNT, GL_UNSIGNED_INT, 0);
    }

    // Render the specified scene from the specified POV
    void render_scene(Scene* scene, glm::mat4 view, glm::mat4 projection, bool draw_portals=true, glm::vec3 slice_pos=glm::vec3(0.0f), glm::vec3 slice_normal=glm::vec3(0.0f)) {
        glm::mat4 model;
        glm::mat4 mvp;

        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Draw brushes
        glUseProgram(standard_shader.program);
        glBindVertexArray(primitives::cube->vao);

        glUniform3f(standard_shader.u_lightdir, scene->light_dir.x, scene->light_dir.y, scene->light_dir.z);
        glUniform1i(standard_shader.u_highlightfrontface, 0);
        glUniform3f(standard_shader.u_slicepos, slice_pos.x, slice_pos.y, slice_pos.z);
        glUniform3f(standard_shader.u_slicenormal, slice_normal.x, slice_normal.y, slice_normal.z);
        for (size_t i = 0; i<scene->geometry.size(); i++) {
            Brush* brush = &scene->geometry[i];
            glm::vec3 half_size = (brush->max - brush->min) / 2.0f;
            model = glm::mat4(1.0f);
            model = glm::translate(model, brush->min + half_size);
            model = glm::scale(model, half_size);
            mvp = projection * view * model;
            glUniformMatrix4fv(standard_shader.u_M, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(standard_shader.u_MVP, 1, GL_FALSE, glm::value_ptr(mvp));
            glUniform3f(standard_shader.u_color, brush->color.r, brush->color.g, brush->color.b);
            glDrawElements(GL_TRIANGLES, CUBE_VERTEX_COUNT, GL_UNSIGNED_INT, 0);
        }

        // Draw cubes
        for (size_t i = 0; i<scene->cubes.size(); i++) {
            Cube* cube = &scene->cubes[i];
            model = glm::mat4(1.0f);
            model = glm::translate(model, cube->position);
            model = glm::scale(model, glm::vec3(cube->size));
            mvp = projection * view * model;
            glUniformMatrix4fv(standard_shader.u_M, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(standard_shader.u_MVP, 1, GL_FALSE, glm::value_ptr(mvp));
            glUniform3f(standard_shader.u_color, cube->color.r, cube->color.g, cube->color.b);
            glDrawElements(GL_TRIANGLES, CUBE_VERTEX_COUNT, GL_UNSIGNED_INT, 0);
        }

        // Draw portals
        if (draw_portals) {
            glDisable(GL_CULL_FACE);
            glUseProgram(portal_shader.program);
            
            if (scene->portal1.open) {
                glBindTexture(GL_TEXTURE_2D, scene->portal2.open ? portal1_target.texture : 0);
                render_portal(&scene->portal1, scene, view, glm::vec3(0.0f, 1.0f, 1.0f));
            }

            if (scene->portal2.open) {
                glBindTexture(GL_TEXTURE_2D, scene->portal1.open ? portal2_target.texture : 0);
                render_portal(&scene->portal2, scene, view, glm::vec3(1.0f, 1.0f, 0.0f));
            }
            glEnable(GL_CULL_FACE);
        }

        // Draw debug cube
        // glUseProgram(standard_shader.program);
        // if (debug_cube_xray) glClear(GL_DEPTH_BUFFER_BIT);
        // mvp = projection * view * debug_cube_transform;
        // glUniformMatrix4fv(standard_shader.u_MVP, 1, GL_FALSE, glm::value_ptr(mvp));
        // glUniform3f(standard_shader.u_color, 1.0f, 0.0f, 0.0f);
        // glUniform1i(standard_shader.u_highlightfrontface, 1);
        // glDrawElements(GL_TRIANGLES, CUBE_VERTEX_COUNT, GL_UNSIGNED_INT, 0);
    }

    // Render everything to the screen (this includes the FBO pass)
    void render_screen(Scene* scene, Camera* cam, float aspect_ratio) {        
        if (scene->portal1.open && scene->portal2.open) {
            // First portal target
            Camera p1cam = Camera(pcam_transform(cam, &scene->portal1, &scene->portal2));
            debug_cube_transform = p1cam.GetTransform();
            
            glBindFramebuffer(GL_FRAMEBUFFER, portal1_target.fbo);
            glEnable(GL_DEPTH_TEST);
            render_scene(scene, p1cam.GetView(), projection, false, scene->portal2.position, scene->portal2.normal);

            // Second portal target
            Camera p2cam = Camera(pcam_transform(cam, &scene->portal2, &scene->portal1));
            
            glBindFramebuffer(GL_FRAMEBUFFER, portal2_target.fbo);
            glEnable(GL_DEPTH_TEST);
            render_scene(scene, p2cam.GetView(), projection, false, scene->portal1.position, scene->portal1.normal);
        }

        // Main target
        glBindFramebuffer(GL_FRAMEBUFFER, main_target.fbo);
        glEnable(GL_DEPTH_TEST);
        render_scene(scene, cam->GetView(), projection);

        // Draw to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(screen_shader.program);
        glUniform1f(screen_shader.u_aspectratio, aspect_ratio);
        glBindVertexArray(primitives::quad->vao);

        // Main camera
        glBindTexture(GL_TEXTURE_2D, main_target.texture);
        glUniformMatrix4fv(screen_shader.u_transform, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        if (show_pcam_povs) {
            // P1 Camera
            glBindTexture(GL_TEXTURE_2D, portal1_target.texture);
            glUniformMatrix4fv(screen_shader.u_transform, 1, GL_FALSE, glm::value_ptr(
                glm::scale(
                    glm::translate(
                        glm::mat4(1.0f),
                        glm::vec3(0.75f, 0.75f, 0.0f)
                    ),
                    glm::vec3(0.25f)
                )
            ));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

            // P2 Camera
            glBindTexture(GL_TEXTURE_2D, portal2_target.texture);
            glUniformMatrix4fv(screen_shader.u_transform, 1, GL_FALSE, glm::value_ptr(
                glm::scale(
                    glm::translate(
                        glm::mat4(1.0f),
                        glm::vec3(0.75f, 0.25f, 0.0f)
                    ),
                    glm::vec3(0.25f)
                )
            ));
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }
}