#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#define CUBE_VERTEX_COUNT 36

#define POSITION 1
#define POSITION_NORMAL 2
#define POSITION_UV 3

struct MeshObjectData
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

MeshObjectData *gen_meshobjdata(GLfloat *vertices, size_t vertex_array_size, GLuint *indices, size_t index_array_size, uint8_t vertex_data_type);
void del_meshobjdata(MeshObjectData **data);

namespace primitives {
    extern MeshObjectData* quad;
    extern MeshObjectData* cube;
    extern MeshObjectData* line;

    void setup();
    void dispose();
}