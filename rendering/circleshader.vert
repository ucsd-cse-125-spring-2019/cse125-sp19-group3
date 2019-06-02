#version 330 core

layout (location = 0) in vec3 position;

uniform mat4 ModelViewProjMtx = mat4(1);

void main() {
	gl_Position = ModelViewProjMtx * vec4(position, 1.0);
}
