#pragma once
#include <string>
#include "mesh/mesh.h"

class ObjLoader {
public:
    Mesh load(const std::string& filepath);
};
