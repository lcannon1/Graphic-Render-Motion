#version 410 core
// simple object fragment shader

// per-frame data, must match in C++ and any shaders that use it
layout(std140)                          // standard layout matching C++
uniform SceneData {                     // like a class name
    mat4 ProjFromWorld, WorldFromProj;  // viewing matrices
    vec4 LightDir;                      // light direction & ambient
};

// per-object data
layout(std140)
uniform ObjectData {
    mat4 WorldFromModel, ModelFromWorld;    // object matrices
    vec3 Ambient; float pad0;               // ambient color & padding
    vec3 Diffuse; float pad1;               // diffuse color & padding
    vec4 Specular;                          // specular color and exponent
};

// global per-object setting, outside of a uniform block
uniform sampler2D ColorTexture;

// input (must match vertex shader output)
in vec2 texcoord;  // texture coordinate
in vec3 normal;    // world-space normal
in vec4 position;  // world-space position

// output to frame buffer
out vec4 fragColor;

void main() {
    // lighting vectors
    vec3 N = normalize(normal);             // surface normal
    vec3 L = normalize(LightDir.xyz);       // light direction
    vec3 V = normalize(WorldFromProj[3].xyz * position.w - position.xyz * WorldFromProj[3].w);
    vec3 H = normalize(V+L);
    float N_dot_L = max(0., dot(N, L));
    float N_dot_H = max(0., dot(N, H));

    // ambient contribution
    vec3 ambCol = Ambient * LightDir.a;

    // diffuse or texture
    vec3 diffCol = Diffuse;
    if (textureSize(ColorTexture,0) != ivec2(1,1))
        diffCol *= texture(ColorTexture, texcoord).rgb;
    diffCol *= N_dot_L;

    // specular
    vec3 specCol = Specular.rgb * pow(N_dot_H, Specular.w) * N_dot_L;

    // final color
    fragColor = vec4(ambCol + diffCol + specCol, 1);
}
