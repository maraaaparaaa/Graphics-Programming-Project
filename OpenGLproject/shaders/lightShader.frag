#version 410 core

in vec2 passTexture;
in vec3 normalEye;
in vec3 fragPosEye;

out vec4 fragmentColour;

uniform vec3 lightDirEye; 
uniform vec3 lightColor;
uniform sampler2D diffuseTexture;
uniform float objectLightMultiplier;
uniform float shininess;
uniform float specularStrength;

void main()
{

    vec3 ambient = 0.2 * lightColor; // minimal light everywhere
    
    vec3 N = normalize(normalEye);
    vec3 L = normalize(-lightDirEye);
    vec3 V = normalize(-fragPosEye); // to the camera
    vec3 R = reflect(-L, N);

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;
    
    float spec = pow(max(dot(V, R), 0.0), shininess); 
    vec3 specular = specularStrength * spec * lightColor;
    
    vec3 textColor = texture(diffuseTexture, passTexture).rgb;
    vec3 result = (ambient + diffuse + specular) * textColor 
						 * objectLightMultiplier;
    fragmentColour = vec4(result, 1.0);
}
