#version 330 core

// Interpolated values from the vertex shaders
in vec2 UV;

// Ouput data
out vec4 FragColor;

uniform sampler2D myTextureSampler;

void main(){
	// Output color = color of the texture at the specified UV
	vec4 texColor  = texture( myTextureSampler, UV );
	FragColor = texColor;
}