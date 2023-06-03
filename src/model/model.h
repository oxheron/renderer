#pragma once

// internal 
#include "mesh.h"

// std
#include <vector>
#include <memory>

class Model
{
    // All of the meshes in the current model
    std::vector<std::unique_ptr<Mesh>> meshes;
};