#version 330 core

uniform vec3 LightDir=normalize(vec3(1,0,1));
uniform vec3 LightColor=vec3(0.6, 0.2, 0.3);
uniform int UseTex = 0;

in vec3 fragNormal;
in vec3 fragPos;
in float edge;
in vec3 world_pos;
out vec4 color;


void main()
{
	float intensity;
	vec4 colorResult;
	intensity = dot(LightDir,fragNormal);
	if(UseTex == 1){
		colorResult = vec4(LightColor,1.0);
	}
	else if(edge<0.1){
		colorResult = vec4(0,0,0,1.0);
	}
	else {
		if (intensity > 0.95)
			colorResult = vec4(LightColor,1.0);
		else if (intensity > 0.7)
			colorResult = vec4(LightColor*0.85,1.0);
		else if (intensity > 0.5)
			colorResult = vec4(LightColor*0.65,1.0);
		else if (intensity > 0.2)
			colorResult = vec4(LightColor*0.4,1.0);
		else 
			colorResult = vec4(LightColor*0.1,1.0);
	}
	color = colorResult;
}