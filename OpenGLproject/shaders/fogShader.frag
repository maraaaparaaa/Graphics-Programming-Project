#version 410 core

in vec3 colour;
in vec2 passTexture;
in vec3 fragmentPosEyeSpace;

out vec4 fragmentColour;
uniform sampler2D diffuseTexture;

float computeFog()
{
 float fogDensity = 0.05f;
 float fragmentDistance = length(fragmentPosEyeSpace);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));
 fogFactor = 0.6f;

 return clamp(fogFactor, 0.0f, 1.0f);
}

void main() {

    vec4 fColor = texture(diffuseTexture, passTexture);
    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f,0.5f,0.5f,1.0f);
    fragmentColour = mix(fogColor, fColor, fogFactor);
}