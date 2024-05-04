#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#define BRUSH_VERTEX_COUNT 36

#define POSITION_NORMAL 1
#define POSITION_UV 2

struct MeshObjectData
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

struct Brush
{
    glm::vec3 min;
    glm::vec3 max;

    glm::vec3 color;

    Brush(glm::vec3 min, glm::vec3 max, glm::vec3 color) : min(min), max(max), color(color) {}
};

MeshObjectData *gen_meshobjdata(GLfloat *vertices, size_t vertex_array_size, GLuint *indices, size_t index_array_size, uint8_t vertex_data_type);
void del_meshobjdata(MeshObjectData **data);

namespace primitives {
    extern MeshObjectData* quad;
    extern MeshObjectData* cube;

    void setup();
    void dispose();
}