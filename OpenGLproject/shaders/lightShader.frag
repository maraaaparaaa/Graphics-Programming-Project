#version 410 core

in vec2 passTexture;
in vec3 normalEye;
in vec3 fragPosEye;

out vec4 fragmentColour;

// directional light
uniform vec3 lightDirEye; 
uniform vec3 lightColor;

// material
uniform sampler2D diffuseTexture;
uniform float objectLightMultiplier;
uniform float shininess;
uniform float specularStrength;

// point light 
uniform vec3 lightPosEye;
uniform vec3 pointLightColor;

uniform float time;

// attenuation for point light
float constant = 1.0f;
float linear = 0.0001f;
float quadratic = 0.0000025f;

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

    //point light

    vec3 lightDirN = normalize(lightPosEye - fragPosEye.xyz);
    // compute distance to light
    float dist = length(lightPosEye - fragPosEye.xyz);
    // compute attenuation
    float att =  1.0f / (constant + linear * dist + quadratic * (dist * dist)); 
    
    // natural pulse
    float flicker1 = sin(time * 2.5 + 0.3) * 0.5 + 0.5;
    float flicker2 = sin(time * 6.7 + 1.2) * 0.3 + 0.7;
    float flicker3 = sin(time * 11.3 + 2.1) * 0.2 + 0.8;
    float flicker4 = sin(time * 17.8 + 0.7) * 0.1 + 0.9;  //trembling
    
    float flickerIntensity = flicker1 * flicker2 * flicker3 * flicker4;
    flickerIntensity = 0.65 + flickerIntensity * 0.35;

    float colorShift = sin(time * 1.8) * 0.15 + 0.85;  // 0.7 - 1.0
    
    vec3 fireColorVariation = vec3(
        1.0,                                    // constant red
        0.5 + colorShift * 0.3,                // green-yellow-orange
        0.2 * colorShift                        // minimal blue
    );

    vec3 pointLightColor_dynamic = pointLightColor * flickerIntensity * fireColorVariation;

    float diff_point = max(dot(N, lightDirN), 0.0);
    vec3 diffuse_point = diff_point * pointLightColor_dynamic;
    
    vec3 R_point = reflect(-lightDirN, N);
    float spec_point = pow(max(dot(V, R_point), 0.0), shininess);
    vec3 specular_point = specularStrength * spec_point * pointLightColor_dynamic;

    vec3 ambient_point = 0.2 * pointLightColor_dynamic;

    diffuse_point *= att;
    specular_point *= att;
    ambient_point *= att;

    vec3 result = ambient * textColor + ambient * textColor; 
    result += (diffuse + specular) * textColor; // directional
    result += (diffuse_point + specular_point) * textColor; // point light
    result *= objectLightMultiplier;

    fragmentColour = vec4(result, 1.0);
}
