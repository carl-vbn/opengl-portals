float quad_vertices[] = {
    // Model space positions     UV
    -1.0f,  1.0f, 0.0f,          0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f,          0.0f, 0.0f,
    1.0f, -1.0f, 0.0f,          1.0f, 0.0f,
    1.0f,  1.0f, 0.0f,          1.0f, 1.0f
};

GLuint quad_indices[] = { 0, 1, 2, 0, 2, 3 };

GLfloat cube_vertices[] = {
    // Model space positions         Model space normals
    // Bottom face
    0.0f, 0.0f, 0.0f,                0.0f, -1.0f, 0.0f,
    1.0f, 0.0f, 0.0f,                0.0f, -1.0f, 0.0f,
    1.0f, 0.0f, 1.0f,                0.0f, -1.0f, 0.0f,
    0.0f, 0.0f, 1.0f,                0.0f, -1.0f, 0.0f,

    // Top face
    1.0f, 1.0f, 0.0f,                0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 0.0f,                0.0f, 1.0f, 0.0f,
    0.0f, 1.0f, 1.0f,                0.0f, 1.0f, 0.0f,
    1.0f, 1.0f, 1.0f,                0.0f, 1.0f, 0.0f,

    // Back face
    0.0f, 0.0f, 0.0f,                0.0f, 0.0f, -1.0f,
    0.0f, 1.0f, 0.0f,                0.0f, 0.0f, -1.0f,
    1.0f, 1.0f, 0.0f,                0.0f, 0.0f, -1.0f,
    1.0f, 0.0f, 0.0f,                0.0f, 0.0f, -1.0f,

    // Left face
    0.0f, 0.0f, 0.0f,                -1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 1.0f,                -1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 1.0f,                -1.0f, 0.0f, 0.0f,
    0.0f, 1.0f, 0.0f,                -1.0f, 0.0f, 0.0f,

    // Right face
    1.0f, 0.0f, 0.0f,                1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 0.0f,                1.0f, 0.0f, 0.0f,
    1.0f, 1.0f, 1.0f,                1.0f, 0.0f, 0.0f,
    1.0f, 0.0f, 1.0f,                1.0f, 0.0f, 0.0f,

    // Front face
    0.0f, 0.0f, 1.0f,                0.0f, 0.0f, 1.0f,
    1.0f, 0.0f, 1.0f,                0.0f, 0.0f, 1.0f,
    1.0f, 1.0f, 1.0f,                0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 1.0f,                0.0f, 0.0f, 1.0f
};

GLuint cube_indices[] = {
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