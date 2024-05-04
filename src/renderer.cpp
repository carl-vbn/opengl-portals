#include "renderer.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

glm::vec3 Camera::GetForwardDirection() {
    glm::vec3 direction;
    direction.x = cos(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    direction.y = sin(glm::radians(this->pitch));
    direction.z = sin(glm::radians(this->yaw)) * cos(glm::radians(this->pitch));
    return direction;
}

glm::vec3 Camera::GetRightDirection() {
    return glm::cross(this->GetForwardDirection(), glm::vec3(0.0f, 1.0f, 0.0f));
}

// Build an OpenGL Shader progam from a vertex shader and a fragment shader
int mkprog(const char* vertex_path, const char* fragment_path) {
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

void render(glm::mat4 projection, Scene* scene, Camera* cam, StandardShader* shader, int depth) {
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(shader->program);

    glm::vec3 direction = cam->GetForwardDirection();
    // std::cout << direction.x << " " << direction.y << " " << direction.z << std::endl;
    glm::mat4 model = glm::rotate(glm::mat4(1.0f), glm::radians((float)scene->time * 100.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 view = glm::lookAt(
        cam->position, 
        cam->position + direction, 
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 mvp = projection * view * model;
    glUniformMatrix4fv(shader->u_MVP, 1, GL_FALSE, glm::value_ptr(mvp));
    glUniform3f(shader->u_lightdir, -0.801783726f, 0.534522484f, 0.267261242f);

    for (size_t i = 0; i<scene->geometry.size(); i++) {
        Brush* brush = scene->geometry[i];
        glBindVertexArray(brush->loaded_data->vao);
        glUniform3f(shader->u_color, brush->color.r, brush->color.g, brush->color.b);
        glDrawElements(GL_TRIANGLES, BRUSH_VERTEX_COUNT, GL_UNSIGNED_INT, 0);
    }
}