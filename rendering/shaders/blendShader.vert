#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex;

uniform mat4 ModelViewProjMtx = mat4(1);

out vec2 texCoords;

void main()
{
    gl_Position = vec4(position, 1.0);
	gl_Position = ModelViewProjMtx * vec4(position, 1.0);
    texCoords = tex;
}  