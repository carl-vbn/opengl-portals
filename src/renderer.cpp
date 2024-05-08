#include "renderer.h"

#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

Camera::Camera(glm::mat4 transform) {
    position = glm::vec3(transform[3]);

    float pitch_radians, yaw_radians, roll_radians;
    glm::extractEulerAngleXYZ(transform, pitch_radians, yaw_radians, roll_radians);

    if (roll_radians > 0.01 || roll_radians < -0.01) {
        std::cout << roll_radians << std::endl;
        // pitch_radians -= glm::pi<float>();
    }

    this->pitch = glm::degrees(pitch_radians);
    this->yaw = glm::degrees(yaw_radians);
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

namespace renderer {
    StandardShader standard_shader;
    ScreenShader screen_shader;
    PortalShader portal_shader;
    glm::mat4 projection;
    RenderTarget main_target;
    RenderTarget portal1_target, portal2_target;
    glm::vec3 debug_cube_pos(0.0f, 0.0f, 0.0f);
    bool debug_cube_xray = false;

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

    int gen_rendertarget(RenderTarget* target, int width, int height) {
        glGenFramebuffers(1, &target->fbo);
        glBindFramebuffer(GL_FRAMEBUFFER, target->fbo);

        glGenTextures(1, &target->texture);
        glBindTexture(GL_TEXTURE_2D, target->texture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
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
        standard_shader.program = load_shader("res/shaders/standard/vertex.glsl", "res/shaders/standard/fragment.glsl");
        standard_shader.u_MVP = glGetUniformLocation(standard_shader.program, "u_MVP");
        standard_shader.u_color = glGetUniformLocation(standard_shader.program, "u_color");
        standard_shader.u_lightdir = glGetUniformLocation(standard_shader.program, "u_lightdir");

        screen_shader.program = load_shader("res/shaders/screen/vertex.glsl", "res/shaders/screen/fragment.glsl");
        screen_shader.u_screentex = glGetUniformLocation(screen_shader.program, "u_screentex");

        portal_shader.program = load_shader("res/shaders/portal/vertex.glsl", "res/shaders/portal/fragment.glsl");
        portal_shader.u_rendertex = glGetUniformLocation(portal_shader.program, "u_rendertex");
        portal_shader.u_MVP = glGetUniformLocation(portal_shader.program, "u_MVP");

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

    // Render the specified scene from the specified POV
    void render_scene(Scene* scene, glm::mat4 view, glm::mat4 projection) {
        glm::mat4 model;
        glm::mat4 mvp;

        glClearColor(0.1f, 0.1f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(standard_shader.program);
        glBindVertexArray(primitives::cube->vao);

        glUniform3f(standard_shader.u_lightdir, scene->light_dir.x, scene->light_dir.y, scene->light_dir.z);
        for (size_t i = 0; i<scene->geometry.size(); i++) {
            Brush* brush = &scene->geometry[i];
            model = glm::mat4(1.0f);
            model = glm::translate(model, brush->min);
            model = glm::scale(model, brush->max - brush->min);
            mvp = projection * view * model;
            glUniformMatrix4fv(standard_shader.u_MVP, 1, GL_FALSE, glm::value_ptr(mvp));
            glUniform3f(standard_shader.u_color, brush->color.r, brush->color.g, brush->color.b);
            glDrawElements(GL_TRIANGLES, BRUSH_VERTEX_COUNT, GL_UNSIGNED_INT, 0);
        }

        // Draw portals
        glUseProgram(portal_shader.program);
        glBindVertexArray(primitives::quad->vao);
        
        model = glm::translate(glm::mat4(1.0f), scene->portal1.position);
        mvp = projection * view * model;
        glUniformMatrix4fv(portal_shader.u_MVP, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindTexture(GL_TEXTURE_2D, portal1_target.texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        model = glm::translate(glm::mat4(1.0f), scene->portal2.position);
        mvp = projection * view * model;
        glUniformMatrix4fv(portal_shader.u_MVP, 1, GL_FALSE, glm::value_ptr(mvp));
        glBindTexture(GL_TEXTURE_2D, portal2_target.texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Draw debug cube
        glUseProgram(standard_shader.program);
        glBindVertexArray(primitives::cube->vao);
        if (debug_cube_xray) glClear(GL_DEPTH_BUFFER_BIT);
        model = glm::mat4(1.0f);
        model = glm::translate(model, debug_cube_pos - glm::vec3(0.5f));
        mvp = projection * view * model;
        glUniformMatrix4fv(standard_shader.u_MVP, 1, GL_FALSE, glm::value_ptr(mvp));
        glUniform3f(standard_shader.u_color, 1.0f, 0.0f, 0.0f);
        glDrawElements(GL_TRIANGLES, BRUSH_VERTEX_COUNT, GL_UNSIGNED_INT, 0);
    }

    // Render everything to the screen (this includes the FBO pass)
    void render_screen(Scene* scene, Camera* cam) {
        // Test
        glm::mat4 cam_transform = cam->GetLocalToWorldMatrix();
        Camera cam_copy(cam_transform);
        // std::cout << cam_copy.yaw-cam->yaw << " " << cam_copy.pitch-cam->pitch << std::endl;
        std::cout << cam->yaw << " " << cam->pitch << std::endl;

        // First portal target
        glm::mat4 p1model = glm::translate(glm::mat4(1.0f), scene->portal1.position);
        glm::mat4 p2model = glm::rotate(glm::translate(glm::mat4(1.0f), scene->portal2.position), glm::pi<float>(), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 cam_model = cam->GetLocalToWorldMatrix();
        glm::mat4 p1cam_transform = p2model * cam_model * glm::inverse(p1model);

        debug_cube_pos = glm::vec3(p1cam_transform[3]);
        // Camera p1cam(p1cam_transform);
        // // std::cout << p1cam.yaw << " " << p1cam.pitch << std::endl;
        
        // glBindFramebuffer(GL_FRAMEBUFFER, portal1_target.fbo);
        // glEnable(GL_DEPTH_TEST);
        // render_scene(scene, p1cam.GetView(), projection);

        // Main target
        glBindFramebuffer(GL_FRAMEBUFFER, main_target.fbo);
        glEnable(GL_DEPTH_TEST);
        render_scene(scene, cam->GetView(), projection);

        // Draw to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glDisable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT);
        glUseProgram(screen_shader.program);
        glBindVertexArray(primitives::quad->vao);
        glBindTexture(GL_TEXTURE_2D, main_target.texture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }
}