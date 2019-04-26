#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoordIn;
layout (location = 3) in vec4 BoneIDs;
layout (location = 4) in vec4 Weights;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 texCoord;
out vec3 boneIDs;
out float val;

const int MAX_BONES = 100;

uniform mat4 ModelMtx = mat4(1);
uniform mat4 ModelViewProjMtx = mat4(1);
uniform mat4 BoneMtx[MAX_BONES];

void main() {
	mat4 BoneTransform = BoneMtx[int(BoneIDs[0])] * Weights[0];
    BoneTransform += BoneMtx[int(BoneIDs[1])] * Weights[1];
    BoneTransform += BoneMtx[int(BoneIDs[2])] * Weights[2];
    BoneTransform += BoneMtx[int(BoneIDs[3])] * Weights[3];

	vec4 PosL = BoneTransform * vec4(position, 1.0);
	//gl_Position = ModelViewProjMtx * PosL;
	gl_Position = ModelViewProjMtx * vec4(position, 1.0);
	//vec4 NormalL = BoneTransform * vec4(normal, 0.0);
	//fragPosition = vec3(ModelMtx * PosL);
	//fragNormal = vec3(ModelMtx * NormalL);
	fragPosition = vec3(ModelMtx * vec4(position, 1.0));
	fragNormal = BoneIDs[0] * vec3(ModelMtx * vec4(normal, 0.0));

	texCoord = texCoordIn;
	boneIDs = vec3(1.0f * BoneIDs[0], 1.0f * BoneIDs[1], 1.0f * BoneIDs[2]);
	val = length(boneIDs);
}
