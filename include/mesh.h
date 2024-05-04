#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#define BRUSH_VERTEX_COUNT 36

struct MeshObjectData {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};

struct Brush {
    glm::vec3 min;
    glm::vec3 max;

    glm::vec3 color;

    MeshObjectData* loaded_data;

    Brush(glm::vec3 min, glm::vec3 max, glm::vec3 color): min(min), max(max), color(color), loaded_data(NULL) {}
};

MeshObjectData* gen_meshobjdata(GLfloat* vertices, size_t vertex_array_size, GLuint* indices, size_t index_array_size);
void del_meshobjdata(MeshObjectData** data);
void load_brush(Brush* brush);
void unload_brush(Brush* brush);