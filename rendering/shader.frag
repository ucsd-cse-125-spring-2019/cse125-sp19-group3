#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 TexCoord;

uniform int UseTex = 0;
uniform sampler2D Texture;

uniform vec3 AmbientColor = vec3(0.2, 0.1, 0.3);
uniform vec3 LightDirection=normalize(vec3(1,5,2));
uniform vec3 LightColor=vec3(0.9, 0.1, 0);
uniform vec3 DiffuseColor = vec3(0.5);

out vec4 finalColor;

void main() {
	if (UseTex == 0) {
		// Compute irradiance (sum of ambient & direct lighting)
		vec3 irradiance = AmbientColor + LightColor * max(0,dot(LightDirection,fragNormal));

		// Diffuse reflectance
		vec3 reflectance = irradiance * DiffuseColor;

		// Gamma correction
		finalColor = vec4(sqrt(reflectance), 1);
		//finalColor = vec4(normalize(2.0f * fragNormal), 1);
	}
	else {
		finalColor = texture(Texture, TexCoord);
	}
}