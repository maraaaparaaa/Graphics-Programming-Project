#version 410 core

// Quad vertices (-1 to 1)
layout(location = 0) in vec3 vertexPosition;

// Instance data
layout(location = 1) in vec3 particlePos;
layout(location = 2) in vec4 particleColor;
layout(location = 3) in float particleSize;
layout(location = 4) in float particleLife;

out vec4 color;
out float life;
out vec2 uv;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 cameraPos;

void main()
{
    // Particule billboard orientate spre camera
    vec3 cameraRight = vec3(view[0][0], view[1][0], view[2][0]);
    vec3 cameraUp    = vec3(view[0][1], view[1][1], view[2][1]);
    
    // Scalare corectă - particleSize este deja în world units
    vec3 worldPos = particlePos 
                  + cameraRight * vertexPosition.x * particleSize
                  + cameraUp    * vertexPosition.y * particleSize;
    
    gl_Position = projection * view * vec4(worldPos, 1.0);
    
    // Pass data to fragment shader
    color = particleColor;
    life = particleLife;
    uv = vertexPosition.xy * 0.5 + 0.5; // [-1,1] -> [0,1]
}