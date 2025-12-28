#version 410 core

// input from VAO
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in float aSize;
layout(location = 3) in float aLife;

out vec4 particleColor;
out float lifeValue;

uniform mat4 view;
uniform mat4 projection;


void main()
{
	vec4 viewPos = view * vec4(aPos, 1.0);
	
	//float distance = length(viewPos.xyz); // distance from camera

	gl_PointSize = aSize ;/// distance * 100.0; // size of the particle (in pixels)

	gl_Position = projection * viewPos;

	particleColor = aColor;
	lifeValue = aLife;    
}