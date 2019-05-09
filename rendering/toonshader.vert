#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;


uniform mat4 ModelMtx = mat4(1);
uniform mat4 ModelViewProjMtx = mat4(1);
uniform mat4 projection;
uniform vec3 viewPos;

out vec3 fragNormal;
out vec3 fragPos;
out vec3 world_pos;
out float edge;
void main()
{
    gl_Position = ModelViewProjMtx * vec4(position, 1.0);
	fragNormal =  normalize(normal);
	world_pos = vec3(mat4(ModelMtx) * vec4(position,1.0f));
	fragPos = vec3(ModelMtx * vec4(position, 1.0));
	//viewDir = normalize(viewPos - fragPos);
	edge = max(0, dot(fragNormal,normalize(viewPos-world_pos)));
}