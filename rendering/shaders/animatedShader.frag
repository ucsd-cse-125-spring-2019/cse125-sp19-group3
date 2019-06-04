#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 texCoord;

uniform int UseTex = 0;
uniform float alpha = 1.0;
uniform sampler2D Texture;

uniform int isEvading = 0;

uniform vec3 ambientColor = vec3(0.21, 0.22, 0.28);
uniform vec3 lightDirection = normalize(vec3(5, 15, 10));
uniform vec3 lightColor = vec3(0.63, 0.68, 0.84);

uniform vec4 color = vec4(1);

out vec4 finalColor;

float getLightFactor(float intensity) {
	float factor = 0.1;
	if (intensity > 0.8) factor = 1.0;
	else if (intensity > 0.3) factor = 0.6;
	else if (intensity > 0.0) factor = 0.3;
	return factor;
}

void main() {

	float intensity = dot(lightDirection, fragNormal);
	float lightFactor = getLightFactor(intensity);
	vec3 diffuseColor = lightColor * max(0, intensity);

	if (UseTex == 0) {
		// Gamma correction
		finalColor = vec4(lightFactor * (ambientColor + diffuseColor) * vec3(color), color[3]);
	}
	else {
		if (isEvading == 1) {
			finalColor = vec4(lightFactor * (ambientColor + diffuseColor) * (vec3(texture(Texture, texCoord)) + vec3(0.8, 0.8, 0.8)), 1.0);
		} else {
		    finalColor = vec4(lightFactor * (ambientColor + diffuseColor) * (vec3(texture(Texture, texCoord)) + vec3(0.1, 0.1, 0.1)), 1.0);
		}
	}
}