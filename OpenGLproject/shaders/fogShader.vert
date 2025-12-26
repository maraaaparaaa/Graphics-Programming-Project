#version 410 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 textcoord;

out vec3 colour;
out vec2 passTexture;
out vec3 fragmentPosEyeSpace;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {

    vec4 posEye = view * model * vec4(vertexPosition, 1.0);
    fragmentPosEyeSpace = posEye.xyz;

    colour = vertexNormal;
    passTexture = textcoord;

    gl_Position = projection * posEye;
}
