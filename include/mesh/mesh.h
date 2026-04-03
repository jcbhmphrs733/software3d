#pragma once
#include <vector>
#include "math/vec2.h"
#include "math/vec3.h"

struct Mesh {
    std::vector<Vec3> vertices;          // 3D positions parsed from 'v' lines
    std::vector<unsigned int> indices;   // vertex indices parsed from 'f' lines, 3 per triangle
    std::vector<Vec3> faceNormals;       // one unit normal per triangle, computed after load
    std::vector<Vec3> vertexNormals;     // one averaged normal per vertex, computed after load
    std::vector<Vec2> uvs;              
    std::vector<unsigned int> uvIndices;    
};
