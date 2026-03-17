#pragma once
#include <vector>
#include "math/vec3.h"

struct Mesh {
    std::vector<Vec3> vertices;          // 3D positions parsed from 'v' lines
    std::vector<unsigned int> indices;   // vertex indices parsed from 'f' lines, 3 per triangle
};
