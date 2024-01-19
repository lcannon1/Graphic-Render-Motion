// draw a simple plan3 model

#include "Plane.hpp"
#include "GLapp.hpp"

#include <stdio.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

using namespace glm;  // avoid glm:: for all glm types and functions

// load the sphere data
Plane::Plane(vec3 size, const char *texturePPM) :
    Object(texturePPM)
{
    // build texture coordinate, normal, and vertex arrays
    uv = {vec2(0.f,0.f), vec2(1.f,0.f), vec2(0.f,1.f), vec2(1.f,1.f)};
    norm = {vec3(0.f,0.f,1.f), vec3(0.f,0.f,1.f), vec3(0.f,0.f,1.f), vec3(0.f,0.f,1.f)};
    vert = {
        vec3(-0.5f*size.x, -0.5f*size.y, 0.f),
        vec3( 0.5f*size.x, -0.5f*size.y, 0.f),
        vec3(-0.5f*size.x,  0.5f*size.y, 0.f),
        vec3( 0.5f*size.x,  0.5f*size.y, 0.f)
    };

    // build index array linking sets of three vertices into triangles
    indices = {0, 1, 3,   0, 3, 2};

    initGPUData();
}

// Object overload to draw external objects
Plane::Plane(std::vector<glm::vec3> vVert, std::vector<glm::vec3> vnVert,
    std::vector<glm::vec2> vtVert, std::vector<unsigned int> fVert, const char* texturePPM) :
    Object(texturePPM)
{
    // build texture coordinate, normal, and vertex arrays
    uv = vtVert;
    norm = vnVert;
    vert = vVert;

    // build index array linking sets of three vertices into triangles
    indices = fVert;

    // intersection precomputation
    for (int i = 0; i < vVert.size() - 2; i += 3) {
        int j = i + 1, k = i + 2;
        vec3 e0 = vVert[j] - vVert[k];
        vec3 e1 = vVert[k] - vVert[i];
        vec3 e2 = vVert[i] - vVert[j];
        vec3 currN = normalize(cross(e0, e1));
        N.push_back(currN);
        vec3 currNa = cross(currN, e0);         vec3 currNb = cross(currN, e1);
        currNa = currNa / dot(currNa, e2);      currNb = currNb / dot(currNb, e0);
        Na.push_back(currNa);                   Nb.push_back(currNb);
        float currCa = dot(currNa, vVert[j]); float currCb = dot(currNb, vVert[k]);
        Ca.push_back(currCa);                   Cb.push_back(currCb);
        V0_dot_N.push_back(dot(vVert[i], currN));
    }

    initGPUData();
}

const float
Plane::intersect(const vec3 rayStart, const vec3 rayDir, const float near) const
{
    // value outside range for movement bounding and z-axis adjustment
    float noIsect = 800;

    for (int i = 0; i < N.size(); i++) {
        // compute intersection point with plane
        float t = (V0_dot_N[i] - dot(N[i], rayStart)) / dot(N[i], rayDir);

        if (t < near || t > 750)
            return noIsect;  // not in ray bounds: no intersection

        vec3 P = rayStart + rayDir * t;

        // compute barycentric alpha
        float a = dot(Na[i], P) - Ca[i];
        if (a < 0 || a > 1)
            return noIsect;  // outside 1st edge or past 1st vertex

        // compute barycentric beta
        float b = dot(Nb[i], P) - Cb[i];
        if (b < 0 || 1 - a - b < 0)
            return noIsect;  // outside 2nd or 3rd edge

        return t;
    }
}
