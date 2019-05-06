#version 330 core

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 TexCoord;
in vec4 boneIDs;
in vec4 boneIDs_2;
in vec4 pos;

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
		//finalColor = vec4(sqrt(reflectance), 1);
		//finalColor = vec4(normalize(2.0f * fragNormal), 1);

		//finalColor = pos;
		//finalColor = vec4(sqrt(pow(boneIDs.x, 2) + pow(boneIDs.y, 2) + pow(boneIDs.z, 2)), 0, 0, 1);
		finalColor = vec4(normalize(vec3(boneIDs.xyz)), 1.0f);
	}
	else {
		finalColor = texture(Texture, TexCoord);
	}
}