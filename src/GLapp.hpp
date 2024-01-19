// collected state for access in callbacks
// 
#pragma once

#include <glm/glm.hpp>
#include <vector>

class GLapp {
public:
    struct GLFWwindow *win;      // graphics window from GLFW system

    // uniform buffer data about the scene
    // must be plain old data, matching layout in shaders
    // rearrange or pad as necessary for vec4 alignment
    struct SceneShaderData {
        glm::mat4 ProjFromWorld, WorldFromProj;  // viewing matrix & inverse
        glm::vec4 LightDir;         // xyz = light direction; w = ambient
    } sceneShaderData;
    unsigned int sceneUniformsID;

    // view info
    bool active;                // clicked into window
    int width, height;          // current window dimensions
    float distance;             // distance from 0,0,0
    float pan, tilt;            // horizontal and vertical Euler angles
    float panRate, tiltRate;    // keyboard orbiting rate in radians/sec
    float xRate, yRate;
    float near, far;

    // mouse state
    double mouseX, mouseY;      // location of mouse at last event

    // drawing state
    bool wireframe;

    // time (in seconds) of last frame
    double prevTime;

    std::vector<float> camPos;
    glm::mat4 eyePos;

    // objects to draw
    std::vector<class Object*> objects;

public:
    // initialize and destroy app data
    GLapp();
    ~GLapp();

    // update shader uniform state each frame
    void sceneUpdate(double dTime);

    // main rendering loop
    void render();
};
