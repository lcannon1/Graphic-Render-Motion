// draw a simple plan3 model

#include "Triangle.hpp"
#include "GLapp.hpp"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

// load the sphere data
Triangle::Triangle(std::vector<glm::vec3> vVert, std::vector<glm::vec3> vnVert,
    std::vector<glm::vec2> vtVert, std::vector<unsigned int> fVert, const char* texturePPM) :
    Object(texturePPM)
{
    // build texture coordinate, normal, and vertex arrays
    uv = vtVert;
    norm = vnVert;
    vert = vVert;

    // build index array linking sets of three vertices into triangles
    indices = fVert;

    initGPUData();
}
