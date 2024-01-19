// data and functions to draw a plane
#pragma once

#include "Object.hpp"

#include <vector>

// plane object
class Triangle : public Object {
public:
    // create plane from -size/2 to size/2
    Triangle(std::vector<glm::vec3> vVert, std::vector<glm::vec3> vnVert,
        std::vector<glm::vec2> vtVert, std::vector<unsigned int> fVert, const char* texturePPM);
};
