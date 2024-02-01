# Graphic-Render-Motion
Clone repository for computer graphic rendering project with camera motion controls

# Contributions
Files created translate user input into camera movement. Implementation of user movement, including bounding to prevent camera from passing through surfaces.

# Credits
Initial Repository Creation:
Dr. Marc Olano
Associate Professor of Computer Science and Electrical Engineering
University of Maryland, Baltimore County

Source code summary:

GLapp.hpp/GLapp.cpp: Overall application data, initialization code, and well
GLFW callbacks, and main rendering loop.

Shader.hpp/Shader.cpp: Loading and compiling shaders.

Object.hpp/Object.cpp: Base class for objects, managing vertex and index
arrays, textures, and shaders.

Plane.hpp/Plane.cpp: Minimal two-triangle object with hard-coded data.

Sphere.hpp/Sphere/cpp: Parametric sphere object with per-frame position
updates.

config.h.in: Used by CMake to resolve data file paths.
