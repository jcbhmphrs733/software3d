#include "mesh/obj_loader.h"
#include <fstream>
#include <sstream>
#include <iostream>

Mesh ObjLoader::load(const std::string& filepath) {
    Mesh mesh;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "ObjLoader: failed to open file: " << filepath << "\n";
        return mesh;
    }

    // parsing will go here
    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string keyword;
        ss >> keyword;

        if (keyword == "v") {
            // parse a vertex position: "v x y z"
            Vec3 vertex;
            ss >> vertex.x >> vertex.y >> vertex.z;
            mesh.vertices.push_back(vertex);
        }
        else if (keyword == "f") {
            // parse a face: "f i0 i1 i2 ..." (OBJ indices are 1-based)
            std::vector<unsigned int> faceIndices;
            unsigned int index;
            while (ss >> index) {
                faceIndices.push_back(index - 1); // convert to 0-based
            }

            // triangulate: fan triangulation from first vertex
            // quad [A,B,C,D] -> triangles [A,B,C] and [A,C,D]
            for (size_t i = 1; i + 1 < faceIndices.size(); i++) {
                mesh.indices.push_back(faceIndices[0]);
                mesh.indices.push_back(faceIndices[i]);
                mesh.indices.push_back(faceIndices[i + 1]);
            }
        }
        // all other keywords (comments, normals, UVs, etc.) are ignored
    }

    return mesh;
}
