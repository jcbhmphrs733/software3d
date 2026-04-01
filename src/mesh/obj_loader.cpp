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
        else if (keyword == "vt") {
            // parse a UV coordinate: "vt u v"
            Vec2 uv;
            ss >> uv.x >> uv.y;
            mesh.uvs.push_back(uv);
        }
        else if (keyword == "f") {
            // parse a face: supports "f v", "f v/vt", "f v//vn", "f v/vt/vn"
            std::vector<unsigned int> faceIndices;
            std::vector<unsigned int> uvFaceIndices;
            std::string token;
            while (ss >> token) {
                // vertex index: part before the first '/'
                unsigned int vIdx = (unsigned int)std::stoi(token.substr(0, token.find('/')));
                faceIndices.push_back(vIdx - 1); // convert to 0-based

                // UV index: part between first and second '/'
                unsigned int uvIdx = 0;
                size_t firstSlash = token.find('/');
                if (firstSlash != std::string::npos && firstSlash + 1 < token.size() && token[firstSlash + 1] != '/') {
                    size_t secondSlash = token.find('/', firstSlash + 1);
                    uvIdx = (unsigned int)std::stoi(token.substr(firstSlash + 1, secondSlash - firstSlash - 1)) - 1;
                }
                uvFaceIndices.push_back(uvIdx);
            }

            // triangulate: fan triangulation from first vertex
            // quad [A,B,C,D] -> triangles [A,B,C] and [A,C,D]
            for (size_t i = 1; i + 1 < faceIndices.size(); i++) {
                mesh.indices.push_back(faceIndices[0]);
                mesh.indices.push_back(faceIndices[i]);
                mesh.indices.push_back(faceIndices[i + 1]);

                mesh.uvIndices.push_back(uvFaceIndices[0]);
                mesh.uvIndices.push_back(uvFaceIndices[i]);
                mesh.uvIndices.push_back(uvFaceIndices[i + 1]);
            }
        }
        // all other keywords (comments, normals, etc.) are ignored
    }

    return mesh;
}