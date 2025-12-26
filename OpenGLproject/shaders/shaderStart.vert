#version 410 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;   // nu e folosit, dar îl păstrăm
layout(location = 2) in vec2 textcoord;

out vec2 passTexture;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    passTexture = textcoord;
    gl_Position = projection * view * model * vec4(vertexPosition, 1.0);
}
