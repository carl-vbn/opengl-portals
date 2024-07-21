#include "mesh.h"

#include <stdexcept>
#include <iostream>

#include "primitive_mesh_data.h"

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

    if (vertex_data_type == POSITION) {
        // aPos
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    } else if (vertex_data_type == POSITION_NORMAL) {
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

MeshObjectData* primitives::quad;
MeshObjectData* primitives::cube;
MeshObjectData* primitives::line;

void primitives::setup() {
    quad = gen_meshobjdata(quad_vertices, sizeof(quad_vertices), quad_indices, sizeof(quad_indices), POSITION_UV);
    cube = gen_meshobjdata(cube_vertices, sizeof(cube_vertices), cube_indices, sizeof(cube_indices), POSITION_NORMAL);
    line = gen_meshobjdata(line_vertices, sizeof(line_vertices), line_indices, sizeof(line_indices), POSITION);
}

void primitives::dispose() {
    del_meshobjdata(&quad);
    del_meshobjdata(&cube);
}