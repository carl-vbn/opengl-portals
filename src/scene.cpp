#include "scene.h"

#include <fstream>
#include <vector>
#include <iostream>
#include <cstring>

#define ARRAY_TO_VEC3(arr) glm::vec3((arr)[0], (arr)[1], (arr)[2])
#define READ_INT32(value) file.read(reinterpret_cast<char*>((value)), sizeof(int));
#define READ_FLOAT(value) file.read(reinterpret_cast<char*>((value)), sizeof(float));
#define READ_FLOAT3(value) file.read(reinterpret_cast<char*>((value)), sizeof(float) * 3);

void load_scene_file(const char* path, Scene* scene) {
    std::ifstream file(path, std::ios::binary);

    int brush_count;
    READ_INT32(&brush_count);

    scene->geometry.clear();

    for (int i = 0; i<brush_count; i++) {
        float min_xyz[3];
        float max_xyz[3];
        float rgb[3];

        READ_FLOAT3(min_xyz);
        READ_FLOAT3(max_xyz);
        READ_FLOAT3(rgb);

        Brush brush = Brush(ARRAY_TO_VEC3(min_xyz), ARRAY_TO_VEC3(max_xyz), ARRAY_TO_VEC3(rgb));

        scene->geometry.push_back(brush);
    }
}