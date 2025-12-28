#version 410 core

in vec4 color;
in float life;
in vec2 uv;

out vec4 fragmentColour;

void main()
{
    // Coordonate centrate (-0.5 la 0.5)
    vec2 coord = uv - vec2(0.5);
    float dist = length(coord);
    
    // Creem un gradient circular soft
    if(dist > 0.5)
        discard;
    
    // Gradient radial de la centru (1.0) spre margine (0.0)
    float alpha = 1.0 - (dist * 2.0);
    
    // Smoother falloff pentru foc mai realistic
    alpha = pow(alpha, 2.0);
    
    // Aplicăm fade-out pe durata vieții
    alpha *= life;
    
    // Multiplicăm cu alpha-ul din culoare pentru control suplimentar
    alpha *= color.a;
    
    // Output final cu culoarea originală și alpha calculat
    fragmentColour = vec4(color.rgb, alpha);
}