#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 TexCoord;

uniform int UseTex = 0;
uniform sampler2D Texture;

uniform vec3 AmbientColor = vec3(0.01, 0.02, 0.22);
uniform vec3 LightDirection=normalize(vec3(5, 15, 10));
uniform vec3 LightColor=vec3(0.63, 0.68, 0.84);
uniform vec3 DiffuseColor = vec3(0.2, 0.2, 0.2);

out vec4 finalColor;

float getLightFactor(float intensity) {
	float factor = 0.1;
	if (intensity > 0.8) factor = 1.0;
	else if (intensity > 0.3) factor = 0.6;
	else if (intensity > 0.0) factor = 0.3;
	return factor;
}

void main() {
	if (UseTex == 0) {
		// Compute irradiance (sum of ambient & direct lighting)
		float intensity = dot(LightDirection, fragNormal);
		float lightFactor = getLightFactor(intensity);

		vec3 irradiance = AmbientColor + LightColor * max(0, intensity);

		// Diffuse reflectance
		vec3 reflectance = irradiance * DiffuseColor;

		// Gamma correction
		finalColor = vec4(lightFactor * sqrt(irradiance), 1);
		//finalColor = vec4(normalize(fragNormal), 1);
	}
	else {
		finalColor = texture(Texture, TexCoord);
	}
}