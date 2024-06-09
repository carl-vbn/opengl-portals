#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "scene.h"
#include "mesh.h"
#include "renderer.h"

#define CAPTURE_CURSOR
#define MOVEMENT_SPEED 2.0f
#define MOUSE_X_SENSITIVITY 0.1f
#define MOUSE_Y_SENSITIVITY 0.1f

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void cursor_pos_callback(GLFWwindow* window, double xposIn, double yposIn);
void process_input(GLFWwindow* window, double deltaTime);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

Camera cam = Camera(glm::vec3(-5.0f, 10.0f, 2.0f), 0.0f, 0.0f);
Scene scene;

float vel_y = 0.0f;
float last_cursor_x = 0.0f;
float last_cursor_y = 0.0f;
bool focused = false;
bool pos_printed = false;
bool on_ground;

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
    glfwSetMouseButtonCallback(*window, mouse_button_callback);

#ifdef CAPTURE_CURSOR
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
#endif

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

    primitives::setup();
    renderer::setup(SCR_WIDTH, SCR_HEIGHT, glm::radians(45.0f));

    load_scene_file("res/scene.bin", &scene);

    double previousTime = glfwGetTime(); // Used for FPS counter, not refreshed every frame
    int frameCount = 0;

    while (!glfwWindowShouldClose(window))
    {
        // FPS Counter
        double time = glfwGetTime();
        double deltaTime = time - scene.time;
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

        process_input(window, deltaTime);

        renderer::render_screen(&scene, &cam, (float)SCR_WIDTH / SCR_HEIGHT);
 
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    renderer::dispose();
    primitives::dispose();

    glfwTerminate();
    return 0;
}

void process_input(GLFWwindow *window, double deltaTime)
{
    glm::vec3 translation = glm::vec3(0.0f);
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float speed_multiplier = deltaTime * MOVEMENT_SPEED * (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS ? 0.2f : 1.0f); 

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        translation += cam.GetPitchlessForwardDirection() * speed_multiplier;
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        translation -= cam.GetPitchlessForwardDirection() * speed_multiplier;
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        translation += cam.GetRightDirection() * speed_multiplier;
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        translation -= cam.GetRightDirection() * speed_multiplier;
    }

    if (on_ground && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        vel_y = 4.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        vel_y = -4.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        focused = false;
    }

    translation.y += vel_y * deltaTime;

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) { 
        if (!pos_printed) {
            PRINT_VEC3(cam.position);
            if (scene.portal1.open && is_in_portal(cam.position, &scene.portal1)) {
                std::cout << "In portal 1" << std::endl;
            }
            if (scene.portal2.open && is_in_portal(cam.position, &scene.portal2)) {
                std::cout << "In portal 2" << std::endl;
            }
            pos_printed = true;
        }
    } else {
        pos_printed = false;
    }

    scene_aware_movement(&cam, translation, &scene, &on_ground);
    update_cubes(&scene, deltaTime);

    if (on_ground) {
        vel_y = 0;
    } else {
        vel_y += GRAVITY * deltaTime;
    }

    renderer::debug_cube_xray = glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS;
    renderer::show_pcam_povs = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void cursor_pos_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    if (!focused) return;

    float xpos = static_cast<float>(xposIn) - SCR_WIDTH / 2.0f;
    float ypos = static_cast<float>(yposIn) - SCR_HEIGHT / 2.0f;

#ifndef CAPTURE_CURSOR
    if (last_cursor_x == 0.0f && last_cursor_y == 0.0f) {
        last_cursor_x = xpos;
        last_cursor_y = ypos;
    }
#endif

    float offsetx = xpos - last_cursor_x;
    float offsety = ypos - last_cursor_y;

    cam.yaw -= offsetx * MOUSE_X_SENSITIVITY;
    cam.pitch -= offsety * MOUSE_Y_SENSITIVITY;

#ifdef CAPTURE_CURSOR
    if (xpos != 0.0f || ypos != 0.0f) {
        glfwSetCursorPos(window, SCR_WIDTH / 2.0f, SCR_HEIGHT / 2.0f);
        xpos = 0.0f;
        ypos = 0.0;
    }
#endif

    last_cursor_x = xpos;
    last_cursor_y = ypos;

    // make sure that when pitch is out of bounds, screen doesn't get flipped
    if (cam.pitch > 89.0f)
        cam.pitch = 89.0f;
    if (cam.pitch < -89.0f)
        cam.pitch = -89.0f;
}

bool place_portal(Portal* portal, RaycastHitInfo* hit_info) {
    glm::vec3 A;
    glm::vec3 B;

    if (glm::abs(1.0f - glm::abs(glm::dot(hit_info->normal, glm::vec3(1,0,0)))) < 0.001) {
        // Normal is parallel to x-axis
        A = glm::vec3(0,1,0);
        B = glm::vec3(0,0,1);
    } else if (glm::abs(1.0f - glm::abs(glm::dot(hit_info->normal, glm::vec3(0,1,0)))) < 0.001) {
        // Normal is parallel to y-axis
        A = glm::vec3(1,0,0);
        B = glm::vec3(0,0,1);
    } else if (glm::abs(1.0f - glm::abs(glm::dot(hit_info->normal, glm::vec3(0,0,1)))) < 0.001) {
        // Normal is parallel to z-axis
        A = glm::vec3(1,0,0);
        B = glm::vec3(0,1,0);
    } else {
        assert(false);
    }

    glm::vec3 x = hit_info->intersection - hit_info->face_min;
    glm::vec3 x_max = hit_info->face_max - hit_info->face_min;
    float u = glm::dot(A,x);
    float v = glm::dot(B,x);

    float u_max = glm::dot(A, x_max);
    float v_max = glm::dot(B, x_max);

    if (u_max < portal->width * 2.0f || v_max < portal->height * 2.0f) {
        return false; // Face is too small
    }

    float u_corrected = glm::min(glm::max(u, portal->width), u_max - portal->width);
    float v_corrected = glm::min(glm::max(v, portal->height), v_max - portal->height);

    glm::vec3 corrected_pos = hit_info->face_min + A * u_corrected + B * v_corrected;

    portal->position = corrected_pos + hit_info->normal * 0.001f;
    portal->normal = hit_info->normal;
    portal->spawn_time = scene.time;
    portal->brush = hit_info->brush;
    portal->open = true;

    return true;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (action != GLFW_PRESS) return;

    if (!focused) {
        focused = true;
        last_cursor_x = 0.0f;
        last_cursor_y = 0.0f;

        return;
    }

    RaycastHitInfo hit_info;
    if (button == GLFW_MOUSE_BUTTON_1 && raycast(&cam, &scene, &hit_info)) {
        place_portal(&scene.portal1, &hit_info);
    }

    if (button == GLFW_MOUSE_BUTTON_2 && raycast(&cam, &scene, &hit_info)) {
        place_portal(&scene.portal2, &hit_info);
    }
}