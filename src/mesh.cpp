#include "mesh.h"

#include <stdexcept>
#include <iostream>

MeshObjectData* gen_meshobjdata(GLfloat* vertices, size_t vertex_array_size, GLuint* indices, size_t index_array_size) {
    MeshObjectData* data = new MeshObjectData();

    unsigned int VAO, VBO, EBO;

    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertex_array_size, vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, index_array_size, indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

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
    
    *data = NULL;
}

// Creates and fills a VAO for the brush and sets the struct's loaded_data attribute.
void load_brush(Brush* brush) {
    if (brush->loaded_data != NULL) {
        throw std::runtime_error("attempted to load already loaded brush");
    }

    GLfloat vertices[] = {
        // Bottom face vertices
        brush->min.x, brush->min.y, brush->min.z, 
        brush->max.x, brush->min.y, brush->min.z, 
        brush->max.x, brush->min.y, brush->max.z,
        brush->min.x, brush->min.y, brush->max.z, 

        // Top face vertices
        brush->max.x, brush->max.y, brush->min.z, 
        brush->min.x, brush->max.y, brush->min.z, 
        brush->min.x, brush->max.y, brush->max.z,
        brush->max.x, brush->max.y, brush->max.z, 
    };

    GLuint indices[] = {
        // Bottom face
        0, 1, 2,
        2, 3, 0,

        // Top face
        4, 5, 6,
        6, 7, 4,

        // Back face
        0, 5, 4,
        4, 1, 0,

        // Left face
        0, 3, 6,
        6, 5, 0,

        // Right face
        1, 4, 7,
        7, 2, 1,

        // Front face
        3, 2, 7,
        7, 6, 3
    };

    brush->loaded_data = gen_meshobjdata(vertices, sizeof(vertices), indices, sizeof(indices));
}

// Destroys a brush's MeshObjectData (VAO, VBO, EBO) and sets the struct's loaded_data property to NULL
void unload_brush(Brush* brush) {
    if (brush->loaded_data == NULL) {
        throw std::runtime_error("attempted to unload already unloaded brush");
    }

    del_meshobjdata(&brush->loaded_data);
}