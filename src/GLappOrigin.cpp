//
// Simple GL example
//


#include "GLapp.hpp"
#include "Sphere.hpp"
#include "Plane.hpp"
#include "Triangle.hpp"

#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <assert.h>

#include <string>
#include <cstring>
#include <fstream>
#include <vector>
#include <map>
#include <iostream>

#ifndef F_PI
#define F_PI 3.1415926f
#endif

using namespace glm;  // avoid glm:: for all glm types and functions

///////
// GLFW callbacks must use extern "C"
extern "C" {
    // called for GLFW error
    void error(int error, const char *description) {
        fprintf(stderr, "GLFW error %d: %s\n", error, description);
    }

    // called whenever the window size changes
    void reshape(GLFWwindow *win, int width, int height) {
        // save window dimensions
        GLapp *app = (GLapp*)glfwGetWindowUserPointer(win);
        glfwGetFramebufferSize(win, &app->width, &app->height);

        // viewport size matches window size
        glViewport(0, 0, app->width, app->height);
    }

    // called when mouse button is pressed
    void mousePress(GLFWwindow *win, int button, int action, int mods) {
        if (button != GLFW_MOUSE_BUTTON_LEFT) return;

        // disable cursor and grab focus
        GLapp *app = (GLapp*)glfwGetWindowUserPointer(win);
        app->active = true;
        glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        glfwGetCursorPos(win, &app->mouseX, &app->mouseY);
    }

    // called when mouse is moved
    void mouseMove(GLFWwindow *win, double x, double y) {
        GLapp *app = (GLapp*)glfwGetWindowUserPointer(win);
        if (!app->active) return;

        // rotation angle, scaled so across the window = one rotation
        app->pan += float(F_PI * float(x - app->mouseX) / app->width);
        app->tilt += float(0.5f*F_PI * float(y - app->mouseY) / app->height);

        // remember location so next update will be relative to this one
        app->mouseX = x;
        app->mouseY = y;
    }

    // called on any keypress
    void keyPress(GLFWwindow *win, int key, int scancode, int action, int mods) {
        GLapp *app = (GLapp*)glfwGetWindowUserPointer(win);

        if (action == GLFW_PRESS) {
            switch (key) {
            case 'A':                   // rotate left
                app->panRate = -F_PI;  // half a rotation/sec
                return;

            case 'D':                   // rotate right
                app->panRate = F_PI;   // half a rotation/sec
                return;

            case 'W':                   // rotate up
                app->tiltRate = 0.5f * F_PI; // 1/4 rotation/sec
                return;

            case 'S':                   // rotate down
                app->tiltRate = -0.5f * F_PI; // 1/4 rotation/sec
                return;

            case 'R':                   // reload shaders
                for (auto object : app->objects)
                    object->updateShaders();
                return;

            case 'I':                   // cycle through ambient intensity
                app->sceneShaderData.LightDir.a += 0.2f;
                if (app->sceneShaderData.LightDir.a > 1.f)
                    app->sceneShaderData.LightDir.a = 0.f;
                return;

            case 'L':                   // toggle lines or solid
                app->wireframe = !app->wireframe;
                glPolygonMode(GL_FRONT_AND_BACK, app->wireframe ? GL_LINE : GL_FILL);
                return;

            case GLFW_KEY_ESCAPE:                    // Escape
                if (app->active) {                   //  1st press, release mouse
                    app->active = false;
                    glfwSetInputMode(win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
                else                                 //  2nd press, exit                 
                    glfwSetWindowShouldClose(win, true);
                return;
            }
        }

        if (action == GLFW_RELEASE) {
            switch (key) {
            case 'A': case 'D':         // stop panning
                app->panRate = 0;
                return;
            case 'W': case 'S':         // stop tilting
                app->tiltRate = 0;
                return;
            }
        }
    }
}

// initialize GLFW - windows and interaction
GLapp::GLapp()
{
    // member data initialization
    active = false;                             // not tracking mouse input
    width = 843; height = 480;                  // window size
    distance = 500.f; pan = 0.f; tilt = -1.4f, near = 1.0f, far = 1000.0f;  // view
    panRate = tiltRate = 0.f;                   // keyboard view control
    mouseX = mouseY = 0.f;                      // mouse view controls
    wireframe = false;                          // solid drawing

    // set error callback before init
    glfwSetErrorCallback(error);
    int ok = glfwInit();
    assert(ok);

    // OpenGL version: YOU MAY NEED TO ADJUST VERSION OR OPTIONS!
    // When figuring out the settings that will work for you, make
    // sure you can see error messages on console output.
    //
    // My driver needs FORWARD_COMPAT, but others will need to comment that out.
    // Likely changes for other versions:
    //   All versions: change VERSION_MAJOR and VERSION_MINOR
    //   OpenGL 3.0 (2008): does not support features we need
    //   OpenGL 3.1 (2009):
    //     comment out GLFW_OPENGL_PROFILE line
    //     Use "140" for the "#version" line in the .vert and .frag files
    //   OpenGL 3.2 (2009): Use "150 core" for the "#version" line in the .vert and .frag files
    //   OpenGL 3.3 (2010): Use "330 core" for the "#version" line in the .vert and .frag files
    //   Any of 4.0 or later:
    //     Similar to 3.3: #version line in shaders uses <MAJOR><MINOR>0
    //     For example, 4.6 is "#version 460 core" 
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);

    // ask for a window with dimensions 843 x 480 (HD 480p)
    win = glfwCreateWindow(width, height, "Simple OpenGL Application", 0, 0);
    assert(win);

    glfwMakeContextCurrent(win);

    // GLEW handles OpenGL shared library access
    glewExperimental = true;
    glewInit();

    // set callback functions to be called by GLFW
    glfwSetWindowUserPointer(win, this);
    glfwSetFramebufferSizeCallback(win, reshape);
    glfwSetKeyCallback(win, keyPress);
    glfwSetMouseButtonCallback(win, mousePress);
    glfwSetCursorPosCallback(win, mouseMove);

    // tell OpenGL to enable z-buffer for overlapping surfaces
    glEnable(GL_DEPTH_TEST);

    // initialize buffer for scene shader data
    glGenBuffers(1, &sceneUniformsID);
    glBindBuffer(GL_UNIFORM_BUFFER, sceneUniformsID);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(SceneShaderData), 0, GL_STREAM_DRAW);

    // initialize scene data
    sceneShaderData.LightDir = vec4(-1,-2,2,0);
}

///////
// Clean up any context data
GLapp::~GLapp() 
{
    for (auto obj: objects)
        delete obj;
    glfwDestroyWindow(win);
    glfwTerminate();
}

// call before drawing each frame to update per-frame scene state
void GLapp::sceneUpdate(double dTime)
{
    pan += float(panRate * dTime);
    tilt += float(tiltRate * dTime);

    sceneShaderData.ProjFromWorld = 
        perspective(F_PI/4.f, (float)width/height, near, far)
        * translate(mat4(1), vec3(0,0,-distance))
        * rotate(mat4(1), tilt, vec3(1,0,0))
        * rotate(mat4(1), pan, vec3(0,0,1));
    sceneShaderData.WorldFromProj = inverse(sceneShaderData.ProjFromWorld);

    glBindBuffer(GL_UNIFORM_BUFFER, sceneUniformsID);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(SceneShaderData), &sceneShaderData);
}

// render a frame
void GLapp::render()
{
    // consistent time for drawing this frame
    double currTime = glfwGetTime();
    double dTime = currTime - prevTime;

    // clear old screen contents to a sky blue
    glClearColor(0.5, 0.7, 0.9, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // draw all objects
    sceneUpdate(dTime);
    for (auto object : objects)
        object->draw(this, currTime);

    // show what we drew
    glfwSwapBuffers(win);
    prevTime = currTime;
}

int main(int argc, char *argv[])
{
    // initialize windows and OpenGL
    GLapp app;

    for (int i = 1; i < argc; i++) {
        std::ifstream ifile;
        std::ifstream mtlFile;
        std::string fileName, token, mtlToken, lib, mtl, filePath = "../data/", pathBuild = "";
        std::vector<vec3> vVert, vnVert, vnDraw;
        std::vector<vec2> vtVert, vtDraw;
        vec2 viewDist(1000.0f, 0.0f);
        std::vector<unsigned int> fVert, vtIndex, vnIndex;
        std::map<std::string, vec3> ka;
        vec3* currKa = &ka[""];
        std::map<std::string, vec3> kd;
        vec3* currKd = &kd[""];
        std::map<std::string, vec3> ks;
        vec3* currKs = &ks[""];
        std::map<std::string, float> ns;
        float* currNs = &ns[""];
        std::map<std::string, std::string> mtlName;
        std::string* currMtl = &mtlName[""];

        fileName = argv[i];
        ifile.open(filePath + fileName);

        // Get file path for mtl and ppm files
        for (int i = 0; i < fileName.length(); i++) {
            if (fileName[i] != '/') {
                pathBuild += fileName[i];
            }
            else {
                pathBuild += fileName[i];
                filePath += pathBuild;
                pathBuild = "";
            }
        }

        // File Parsing
        if (ifile.is_open()) {
            ifile >> token;
            while (!ifile.eof()) {

                // Open mtl file and retrieve texture data
                if (token == "mtllib") {
                    ifile >> lib;
                    mtlFile.open(filePath + lib);
                    if (mtlFile.is_open()) {
                        mtlFile >> mtlToken;
                        while (!mtlFile.eof()) {

                            // Map texture data to mtl label
                            if (mtlToken == "newmtl") {
                                mtlFile >> mtl;
                                currKa = &ka[mtl];
                                currKd = &kd[mtl];
                                currKs = &ks[mtl];
                                currNs = &ns[mtl];
                                currMtl = &mtlName[mtl];
                            }
                            else if (mtlToken == "Ka")
                                mtlFile >> (*currKa)[0] >> (*currKa)[1] >> (*currKa)[2];
                            else if (mtlToken == "Kd")
                                mtlFile >> (*currKd)[0] >> (*currKd)[1] >> (*currKd)[2];
                            else if (mtlToken == "Ks")
                                mtlFile >> (*currKs)[0] >> (*currKs)[1] >> (*currKs)[2];
                            else if (mtlToken == "Ns")
                                mtlFile >> *currNs;
                            else if (mtlToken == "map_Kd") {
                                mtlFile >> mtlToken;
                                *currMtl = filePath + mtlToken;
                            }
                            mtlFile >> mtlToken;
                        }
                    }
                }

                else if (token == "usemtl") {

                    // Initialize vnDraw and vtDraw for object creation
                    if (vnDraw.size() < vVert.size()) {
                        while (vnDraw.size() != vVert.size()) {
                            vnDraw.push_back(vec3(0, 0, 0));
                        }
                    }
                    if (vtDraw.size() < vVert.size()) {
                        while (vtDraw.size() != vVert.size()) {
                            vtDraw.push_back(vec2(0, 0));
                        }
                    }

                    // Check if end of mtl instance
                    if (fVert.size() > 0) {

                        // Match texture coords and normals to vertex positions
                        for (int i = 0; i < fVert.size(); i++) {
                            vnDraw[fVert[i]] = vnVert[vnIndex[i]];
                            vtDraw[fVert[i]] = vtVert[vtIndex[i]];
                        }

                        // Pass in object data and create object
                        const char* mtlPPM = mtlName[mtl].c_str();
                        app.objects.push_back(new Plane(vVert, vnDraw, vtDraw, fVert, mtlPPM));

                        // Empty object arrays to prepare for new object
                        fVert = {};
                        vnIndex = {};
                        vtIndex = {};
                    }
                    ifile >> mtl;
                }

                else if (token == "v") { // v
                    float a, b, c;
                    ifile >> token; a = std::stof(token);
                    ifile >> token; b = std::stof(token);
                    ifile >> token; c = std::stof(token);
                    vec3* vert3 = new vec3(a, b, c);
                    vVert.push_back(*vert3);

                    // View Handeling
                    vec3 distVec(a, b, c);
                    for (int i = 0; i < 3; i++) {

                        // Check for lower bound of view
                        if (distVec[i] < viewDist[0]) {
                            viewDist[0] = distVec[i];
                            if (viewDist[1] != 0) {
                                app.distance = 2 * viewDist[1] - viewDist[0];
                                app.far = viewDist[1] + app.distance;
                                app.near = (viewDist[0] < 0) ? 2 * viewDist[0] + app.distance : app.distance - 2 * viewDist[0];
                            }
                        }

                        // Check for upper bound of view
                        if (distVec[i] > viewDist[1]) {
                            viewDist[1] = distVec[i];
                            if (viewDist[0] != 1000) {
                                app.distance = 2 * viewDist[1] - viewDist[0];
                                app.far = viewDist[1] + app.distance;
                                app.near = (viewDist[0] < 0) ? 2 * viewDist[0] + app.distance : app.distance - 2 * viewDist[0];
                            }
                        }
                    }
                }
                else if (token == "vt") { // uv
                    float a, b;
                    ifile >> token; a = std::stof(token);
                    ifile >> token; b = std::stof(token);
                    vec2* vert2 = new vec2(a, b);
                    vtVert.push_back(*vert2);
                }
                else if (token == "vn") { // norm
                    float a, b, c;
                    ifile >> token; a = std::stof(token);
                    ifile >> token; b = std::stof(token);
                    ifile >> token; c = std::stof(token);
                    vec3* vertNorm = new vec3(a, b, c);
                    vnVert.push_back(*vertNorm);
                }
                else if (token == "f") { // indices
                    for (int j = 0; j < 3; j++) {
                        ifile >> token;
                        unsigned int face = 0;

                        // Separate polygon face data into arrays
                        for (int i = 0; i < token.length(); i++) {

                            // Separate digits of f
                            if (token[i] != '/') {
                                if (face == 0)
                                    face = atof(&token[i]);
                            }
                            else {

                                // Check which array to fill
                                if (vtIndex.size() < fVert.size()) {

                                    // Input texture data
                                    vtIndex.push_back(face - 1);
                                    face = 0;
                                }
                                else if (vnIndex.size() < fVert.size()) {

                                    // Input normal data
                                    vnIndex.push_back(face - 1);
                                    face = 0;
                                }
                                else {

                                    // Input vertex position data
                                    fVert.push_back(face - 1);
                                    face = 0;
                                }
                            }
                            if (i == (token.length() - 1)) {

                                // Check which array to fill
                                if (vtIndex.size() < fVert.size()) {

                                    // Input texture data
                                    vtIndex.push_back(face - 1);
                                    face = 0;
                                }
                                else if (vnIndex.size() < fVert.size()) {

                                    // Input normal data
                                    vnIndex.push_back(face - 1);
                                    face = 0;
                                }
                                else {

                                    // Input vertex position data
                                    fVert.push_back(face - 1);
                                    face = 0;
                                }
                            }
                        }
                    }
                }
                ifile >> token;
            }

            // Initialize vnDraw and vtDraw for object creation
            if (vnDraw.size() < vVert.size()) {
                while (vnDraw.size() != vVert.size()) {
                    vnDraw.push_back(vec3(0, 0, 0));
                }
            }
            if (vtDraw.size() < vVert.size()) {
                while (vtDraw.size() != vVert.size()) {
                    vtDraw.push_back(vec2(0, 0));
                }
            }

            // Check if object data is present
            if (fVert.size() > 0) {

                // Match texture coords and normals to vertex positions
                for (int i = 0; i < fVert.size(); i++) {
                    vnDraw[fVert[i]] = vnVert[vnIndex[i]];
                    vtDraw[fVert[i]] = vtVert[vtIndex[i]];
                }
            }

            // Pass in object data and create object
            const char* mtlPPM = mtlName[mtl].c_str();
            app.objects.push_back(new Plane(vVert, vnDraw, vtDraw, fVert, mtlPPM));
        }
    }

    // set up initial viewport
    reshape(app.win, app.width, app.height);

    // each frame: render then check for events
    while (!glfwWindowShouldClose(app.win)) {
        app.render();
        glfwPollEvents();
    }

    return 0;
}
