// data and functions to draw a plane
#pragma once

#include "Object.hpp"

// plane object
class Plane : public Object {
private:
    std::vector<glm::vec3> N, Na, Nb;         // edge normals
    std::vector<float> Ca, Cb;        // constants for barycentric computation
    std::vector<float> V0_dot_N;      // derived for intersection testing

public:
    // create plane from -size/2 to size/2
    Plane(glm::vec3 size, const char *texturePPM);

    // create objects from .obj files
    Plane(std::vector<glm::vec3> vVert, std::vector<glm::vec3> vnVert,
        std::vector<glm::vec2> vtVert, std::vector<unsigned int> fVert, const char* texturePPM);

public: // object functions
    const float intersect(const glm::vec3 rayStart, const glm::vec3 rayDir, const float near) const override;
};
