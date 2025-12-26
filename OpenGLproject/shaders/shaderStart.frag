#version 410 core

in vec2 passTexture;

out vec4 fragmentColour;

uniform sampler2D diffuseTexture;

void main()
{
    fragmentColour = texture(diffuseTexture, passTexture);
}
