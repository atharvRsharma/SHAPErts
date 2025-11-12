#include "FileParser.h"
#include <unordered_map>
#include <tuple>
#include <sstream>
#include <fstream>
#include <iostream>

// helper key type for the map
struct IndexTriplet {
    int v, vt, vn;
    bool operator==(IndexTriplet const& o) const {
        return v == o.v && vt == o.vt && vn == o.vn;
    }
};

struct IndexTripletHash {
    std::size_t operator()(IndexTriplet const& k) const noexcept {
        // Simple combined hash
        // use 31/131 multipliers to spread bits
        return ((std::size_t)k.v * 73856093u) ^ ((std::size_t)k.vt * 19349663u) ^ ((std::size_t)k.vn * 83492791u);
    }
};

// Modified signature: also fill out_indices
void parseObj(const std::string& filePath,
    std::vector<Primitives::Vertex>& out_vertices,
    std::vector<unsigned int>& out_indices)
{
    std::vector<glm::vec3> temp_positions;
    std::vector<glm::vec2> temp_tex_coords;
    std::vector<glm::vec3> temp_normals;

    out_vertices.clear();
    out_indices.clear();

    std::ifstream fileStream(filePath);
    if (!fileStream.is_open()) {
        std::cerr << "file path err: " << filePath << std::endl;
        return;
    }

    // Map from (v,vt,vn) to new vertex index
    std::unordered_map<IndexTriplet, unsigned int, IndexTripletHash> indexMap;
    indexMap.reserve(1024);

    std::string line;
    while (std::getline(fileStream, line)) {
        if (line.empty()) continue;

        std::stringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            glm::vec3 position;
            ss >> position.x >> position.y >> position.z;
            temp_positions.push_back(position);
        }
        else if (prefix == "vt") {
            glm::vec2 tex_coords;
            ss >> tex_coords.x >> tex_coords.y;
            temp_tex_coords.push_back(tex_coords);
        }
        else if (prefix == "vn") {
            glm::vec3 normals;
            ss >> normals.x >> normals.y >> normals.z;
            temp_normals.push_back(normals);
        }
        else if (prefix == "f") {
            // We'll parse each vertex of the face (supports triangles; can be extended for n-gons)
            // OBJ uses 1-based indices. We'll assume faces are v/vt/vn. For robustness you can add parsing for missing vt or vn.
            std::string vertStr;
            std::vector<unsigned int> faceIndices; // indices for this face (triangulate if necessary)

            // read whole remainder and split by whitespace
            std::vector<std::string> vertexTokens;
            while (ss >> vertStr) vertexTokens.push_back(vertStr);

            // If face has >3 vertices (quad/ngon), fan triangulate
            for (size_t i = 0; i < vertexTokens.size(); ++i) {
                // parse "v/vt/vn"
                int vi = 0, vti = 0, vni = 0;
                char c1 = 0, c2 = 0;
                std::stringstream vss(vertexTokens[i]);
                // attempt to parse ints separated by '/'
                // This is a bit forgiving: it will handle v//vn (empty vt) poorly; extend if you need that case.
                vss >> vi >> c1 >> vti >> c2 >> vni;

                IndexTriplet key{ vi, vti, vni };

                auto it = indexMap.find(key);
                if (it != indexMap.end()) {
                    faceIndices.push_back(it->second);
                }
                else {
                    // create new vertex
                    Primitives::Vertex vx;
                    // guard against out-of-range indices
                    if (vi > 0 && static_cast<size_t>(vi) <= temp_positions.size())
                        vx.position = temp_positions[vi - 1];
                    else vx.position = glm::vec3(0.0f);

                    if (vti > 0 && static_cast<size_t>(vti) <= temp_tex_coords.size())
                        vx.texCoord = temp_tex_coords[vti - 1];
                    else vx.texCoord = glm::vec2(0.0f);

                    if (vni > 0 && static_cast<size_t>(vni) <= temp_normals.size())
                        vx.normal = temp_normals[vni - 1];
                    else vx.normal = glm::vec3(0.0f);

                    unsigned int newIndex = static_cast<unsigned int>(out_vertices.size());
                    out_vertices.push_back(vx);
                    indexMap.emplace(key, newIndex);
                    faceIndices.push_back(newIndex);
                }
            } // end per-face-vertex parse

            // triangulate faceIndices if it's >3 (fan triangulation)
            if (faceIndices.size() >= 3) {
                for (size_t k = 1; k + 1 < faceIndices.size(); ++k) {
                    out_indices.push_back(faceIndices[0]);
                    out_indices.push_back(faceIndices[k]);
                    out_indices.push_back(faceIndices[k + 1]);
                }
            }
        }
    } // end file read

    fileStream.close();
}
