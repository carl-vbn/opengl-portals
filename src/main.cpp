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

#define MOVEMENT_SPEED 0.05f

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_pos_callback(GLFWwindow* window, double xposIn, double yposIn);
void process_input(GLFWwindow *window);

const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

Camera cam = Camera(glm::vec3(0.0f, 0.0f, 10.0f), -90.0f, 0.0f);

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
    glfwSetCursorPosCallback(*window, cursor_pos_callback);
    // glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetCursorPos(*window, SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);

    return 0;
}

int main()
{
    GLFWwindow* window;
    if (glfw_setup(&window) != 0) return -1;

    primitives::setup();
    renderer::setup(SCR_WIDTH, SCR_HEIGHT, glm::radians(45.0f));

    Scene scene;

    // Brushes deleted below
    scene.geometry.push_back(new Brush(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f, 0.0f, 0.5f)
    ));

    scene.geometry.push_back(new Brush(
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(2.0f, 2.0f, 2.0f),
        glm::vec3(0.0f, 1.0f, 0.5f)
    ));

    scene.geometry.push_back(new Brush(
        glm::vec3(-6.5f, -1.5f, -0.2f),
        glm::vec3(-3.5f, 1.5f, -0.01f),
        glm::vec3(0.0f, 0.0f, 0.0f)
    ));

    scene.portals.push_back(new Portal(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f),
        1.0f,
        1.0f
    ));

    double previousTime = glfwGetTime();
    int frameCount = 0;


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

        renderer::render_screen(&scene, &cam);
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (size_t i = 0; i<scene.geometry.size(); i++) delete scene.geometry[i];
    for (size_t i = 0; i<scene.portals.size(); i++) delete scene.portals[i];

    renderer::dispose();
    primitives::dispose();

    glfwTerminate();
    return 0;
}

void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        cam.position += cam.GetForwardDirection() * MOVEMENT_SPEED;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        cam.position -= cam.GetForwardDirection() * MOVEMENT_SPEED;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        cam.position += cam.GetRightDirection() * MOVEMENT_SPEED;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        cam.position -= cam.GetRightDirection() * MOVEMENT_SPEED;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        cam.position.y += MOVEMENT_SPEED;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        cam.position.y -= MOVEMENT_SPEED;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

float lastX;
float lastY;
bool firstMouse = true;
void cursor_pos_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f; // change this value to your liking
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    cam.yaw += xoffset;
    cam.pitch += yoffset;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (cam.pitch > 89.0f)
        cam.pitch = 89.0f;
    if (cam.pitch < -89.0f)
        cam.pitch = -89.0f;
}