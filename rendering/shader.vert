#version 330 core

layout (location=0) in vec3 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec2 texCoordIn;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 texCoord;

uniform mat4 ModelMtx = mat4(1);
uniform mat4 ModelViewProjMtx = mat4(1);

void main() {
	gl_Position = ModelViewProjMtx * vec4(position,1);

	fragPosition = vec3(ModelMtx * vec4(position,1));
	fragNormal = vec3(ModelMtx * vec4(normal,0));
	texCoord = texCoordIn;
}
