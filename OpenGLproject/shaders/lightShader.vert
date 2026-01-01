#version 410 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;   
layout(location = 2) in vec2 textcoord;

out vec3 fragPosEye; // fragment position in eye space
out vec3 normalEye; // normal in eye space
out vec2 passTexture;
out vec4 fragPosLightSpace; // for shadow mapping

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
//uniform mat3 normalMatrix;
uniform mat4 lightSpaceMatrix;

void main()
{
    // world space position
    vec4 worldPos = model * vec4(vertexPosition, 1.0);

    // view space position
    vec4 posEye = view * worldPos;
    fragPosEye = posEye.xyz;
    
    fragPosLightSpace = lightSpaceMatrix * worldPos;

    normalEye = mat3(transpose(inverse(view * model))) * vertexNormal;

    passTexture = textcoord;
    gl_Position = projection * posEye;
    
}
