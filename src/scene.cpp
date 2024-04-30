#include "scene.h"

#include <vector>
#include <iostream>

// Load scene data to GPU
void load_scene(Scene* scene) {
    for (size_t i = 0; i<scene->geometry.size(); i++) {
        Brush* brush = scene->geometry[i];
        if (brush->loaded_data == NULL) {
            load_brush(brush);
        }
    }
}

// Unload scene data to GPU
void unload_scene(Scene* scene) {
    for (size_t i = 0; i<scene->geometry.size(); i++) {
        Brush* brush = scene->geometry[i];
        if (brush->loaded_data != NULL) {
            unload_brush(brush);
        }
    }
}