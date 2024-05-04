#include "mesh.h"

#include <stdexcept>
#include <iostream>

MeshObjectData* gen_meshobjdata(GLfloat* vertices, size_t vertex_array_size, GLuint* indices, size_t index_array_size, uint8_t vertex_data_type) {
    MeshObjectData* data = new MeshObjectData(); // Deleted in del_meshobjdata

    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_array_size, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_array_size, indices, GL_STATIC_DRAW);

    if (vertex_data_type == POSITION_NORMAL) {
        // aPos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // aNormal
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    } else if (vertex_data_type == POSITION_UV) {
        // aPos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        // aNormal
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0); 
    glBindVertexArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Not allowed to unbind EBO while VAO is bound

    data->vao = VAO;
    data->vbo = VBO;
    data->ebo = EBO;

    return data;
}

void del_meshobjdata(MeshObjectData** data) {
    glDeleteVertexArrays(1, &(*data)->vao);
    glDeleteBuffers(1, &(*data)->vbo);
    glDeleteBuffers(1, &(*data)->ebo);
    
    delete *data;
    *data = NULL;
}

// Creates and fills a VAO for the brush and sets the struct's loaded_data attribute.
void load_brush(Brush* brush) {
    if (brush->loaded_data != NULL) {
        throw std::runtime_error("attempted to load already loaded brush");
    }

    GLfloat vertices[] = {
        // Model space positions                     Model space normals
        // Bottom face
        brush->min.x, brush->min.y, brush->min.z,    0.0f, -1.0f, 0.0f,
        brush->max.x, brush->min.y, brush->min.z,    0.0f, -1.0f, 0.0f,
        brush->max.x, brush->min.y, brush->max.z,    0.0f, -1.0f, 0.0f,
        brush->min.x, brush->min.y, brush->max.z,    0.0f, -1.0f, 0.0f,

        // Top face
        brush->max.x, brush->max.y, brush->min.z,    0.0f, 1.0f, 0.0f,
        brush->min.x, brush->max.y, brush->min.z,    0.0f, 1.0f, 0.0f,
        brush->min.x, brush->max.y, brush->max.z,    0.0f, 1.0f, 0.0f,
        brush->max.x, brush->max.y, brush->max.z,    0.0f, 1.0f, 0.0f,

        // Back face
        brush->min.x, brush->min.y, brush->min.z,    0.0f, 0.0f, -1.0f,
        brush->min.x, brush->max.y, brush->min.z,    0.0f, 0.0f, -1.0f,
        brush->max.x, brush->max.y, brush->min.z,    0.0f, 0.0f, -1.0f,
        brush->max.x, brush->min.y, brush->min.z,    0.0f, 0.0f, -1.0f,

        // Left face
        brush->min.x, brush->min.y, brush->min.z,    -1.0f, 0.0f, 0.0f,
        brush->min.x, brush->min.y, brush->max.z,    -1.0f, 0.0f, 0.0f,
        brush->min.x, brush->max.y, brush->max.z,    -1.0f, 0.0f, 0.0f,
        brush->min.x, brush->max.y, brush->min.z,    -1.0f, 0.0f, 0.0f,

        // Right face
        brush->max.x, brush->min.y, brush->min.z,    1.0f, 0.0f, 0.0f,
        brush->max.x, brush->max.y, brush->min.z,    1.0f, 0.0f, 0.0f,
        brush->max.x, brush->max.y, brush->max.z,    1.0f, 0.0f, 0.0f,
        brush->max.x, brush->min.y, brush->max.z,    1.0f, 0.0f, 0.0f,

        // Front face
        brush->min.x, brush->min.y, brush->max.z,    0.0f, 0.0f, 1.0f,
        brush->max.x, brush->min.y, brush->max.z,    0.0f, 0.0f, 1.0f,
        brush->max.x, brush->max.y, brush->max.z,    0.0f, 0.0f, 1.0f,
        brush->min.x, brush->max.y, brush->max.z,    0.0f, 0.0f, 1.0f
    };

    GLuint indices[] = {
        // Bottom face
        0, 1, 2,
        2, 3, 0,

        // Top face
        4, 5, 6,
        6, 7, 4,

        // Back face
        8, 9, 10,
        10, 11, 8,

        // Left face
        12, 13, 14,
        14, 15, 12,

        // Right face
        16, 17, 18,
        18, 19, 16,

        // Front face
        20, 21, 22,
        22, 23, 20
    };

    brush->loaded_data = gen_meshobjdata(vertices, sizeof(vertices), indices, sizeof(indices), POSITION_NORMAL);
}

// Destroys a brush's MeshObjectData (VAO, VBO, EBO) and sets the struct's loaded_data property to NULL
void unload_brush(Brush* brush) {
    if (brush->loaded_data == NULL) {
        throw std::runtime_error("attempted to unload already unloaded brush");
    }

    del_meshobjdata(&brush->loaded_data);
}