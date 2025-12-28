#version 410 core

in vec4 particleColor;
in float lifeValue;

out vec4 fragmentColour; // final output : color of the pixel

void main()
{
    // gl_PointCoord = pixel local coordinates inside a point from (0,0) to (1,1) - vec2
	// center of the rasterized point = (0.5,0.5)
	vec2 coord = gl_PointCoord - vec2(0.5); // moves the origin from corner to center
	float dist = length(coord); //distance from the center - length of the vector coord = (x,y)
	
	if (dist > 0.5) {
     		discard; // discard the pixel
	}
	

	// radial gradient - lighter in the center
	float alpha = 1.0 - (dist * 2.0); // 1.0 in the center, 0.0 at margins
	alpha = pow(alpha, 3.0); //smoother transition - tranzitia liniara reprezinta o trecere dura
//pe cand aplicarea pow determina o tranzitie mai lenta intre culori ( cauta grafic pe net )
	
	float coreIntensity = 1.0 - smoothstep(0.0, 0.1, dist);
        alpha = mix(alpha, alpha * 1.5, coreIntensity * 0.4);

	//final alpha depends on life too - transparenta in funtie de viata particulei
	alpha *= lifeValue;

	vec3 finalColor = particleColor.rgb * 0.6;

	//final color
	fragmentColour = vec4(finalColor, particleColor.a * alpha);


}