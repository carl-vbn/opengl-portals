#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mesh.h"
#include "renderer.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void process_input(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

int glfw_setup(GLFWwindow** window) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Portal", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, framebuffer_size_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    return 0;
}

int main()
{
    GLFWwindow* window;
    if (glfw_setup(&window) != 0) return -1;

    BaseShader shader;
    shader.program = mkprog("res/vertex.glsl", "res/fragment.glsl");
    shader.u_MVP = glGetUniformLocation(shader.program, "u_MVP");
    shader.u_color = glGetUniformLocation(shader.program, "u_color");
    shader.u_lightdir = glGetUniformLocation(shader.program, "u_lightdir");

    Scene scene;

    scene.geometry.push_back(new Brush(
        glm::vec3(-1.0f, -0.5f, -0.5f),
        glm::vec3(1.0f, 0.5f, 0.5f),
        glm::vec3(1.0f, 0.0f, 0.5f)
    ));

    scene.geometry.push_back(new Brush(
        glm::vec3(-0.5f, 0.5f, -0.5f),
        glm::vec3(0.5f, 0.8f, 0.5f),
        glm::vec3(0.0f, 1.0f, 0.5f)
    ));

    load_scene(&scene);

    glm::mat4 projection = mkproj(glm::vec3(0.0f, 0.0f, 1.0f), 0.0f, 0.0f, 45.0f, (float)SCR_WIDTH/SCR_HEIGHT);
    double previousTime = glfwGetTime();
    int frameCount = 0;

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        // FPS Counter
        double time = glfwGetTime();
        scene.time = time;
        frameCount++;
        if (time - previousTime >= 2.0)
        {
            std::stringstream titlestream;
            titlestream << "Portal [" << frameCount / 2.0f << " FPS]";
            glfwSetWindowTitle(window, titlestream.str().c_str());

            frameCount = 0;
            previousTime = time;
        }

        process_input(window);

        render(projection, &scene, &shader, 0);
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    unload_scene(&scene);
    glDeleteProgram(shader.program);

    glfwTerminate();
    return 0;
}

void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}
