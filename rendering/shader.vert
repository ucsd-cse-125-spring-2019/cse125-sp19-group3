#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texCoordIn;
layout (location = 3) in ivec4 BoneIDs;
layout (location = 4) in ivec4 BoneIDs_2;
layout (location = 5) in vec4 Weights;
layout (location = 6) in vec4 Weights_2;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 texCoord;
out vec4 boneIDs;
out vec4 boneIDs_2;
out vec4 pos;

const int MAX_BONES = 100;

uniform mat4 ModelMtx = mat4(1);
uniform mat4 ModelViewProjMtx = mat4(1);
uniform mat4 BoneMtx[MAX_BONES];

void main() {
	mat4 BoneTransform = BoneMtx[BoneIDs[0]] * Weights[0];
		BoneTransform += BoneMtx[BoneIDs[1]] * Weights[1];
		BoneTransform += BoneMtx[BoneIDs[2]] * Weights[2];
		BoneTransform += BoneMtx[BoneIDs[3]] * Weights[3];
		BoneTransform += BoneMtx[BoneIDs_2[0]] * Weights_2[0];
		BoneTransform += BoneMtx[BoneIDs_2[1]] * Weights_2[1];
		BoneTransform += BoneMtx[BoneIDs_2[2]] * Weights_2[2];
		BoneTransform += BoneMtx[BoneIDs_2[3]] * Weights_2[3];

		//if (Weights[0] == 0) {
		//    BoneTransform = mat4(1);
		//}


	vec4 PosL = BoneTransform * vec4(position, 1.0);
	vec4 NormalL = BoneTransform * vec4(normal, 0.0);

	gl_Position = ModelViewProjMtx * PosL;
	//gl_Position = ModelViewProjMtx * vec4(position, 1.0);

	fragPosition = vec3(ModelMtx * PosL);
	fragNormal = vec3(ModelMtx * NormalL);
	//fragPosition = vec3(ModelMtx * vec4(position, 1.0));
	//fragNormal = vec3(ModelMtx * vec4(normal, 0.0));

	texCoord = texCoordIn;
	boneIDs = vec4(1.0f * BoneIDs[0], 1.0f * BoneIDs[1], 1.0f * BoneIDs[2], 1.0f * BoneIDs[3]);
	//boneIDs = Weights;
	boneIDs_2 = vec4(1.0f * BoneIDs_2[0], 1.0f * BoneIDs_2[1], 1.0f * BoneIDs_2[2], 1.0f * BoneIDs_2[3]);
	//boneIDs_2 = Weights_2;
	//pos = vec4(normalize(vec3(PosL.x, PosL.y, PosL.z)), 1.0);
	pos = vec4(normalize(position), 1.0);
}
