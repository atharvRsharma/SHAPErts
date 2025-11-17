#pragma once

#include <iostream>
#include <fstream> 
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include <vector>
#include "Primitives.h"




void parseObj(const std::string& filePath,
    std::vector<Primitives::Vertex>& out_vertices,
    std::vector<unsigned int>& out_indices);
