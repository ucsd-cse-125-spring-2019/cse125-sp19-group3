#include "Model.h"
#include "../networking/KillStreak/Logger.hpp"
glm::mat4 aiM4x4toGlmMat4(aiMatrix4x4 m);
glm::mat4 aiM3x3toGlmMat4(aiMatrix3x3 m);

//static inline glm::mat4 mat4_cast(const aiMatrix4x4& m) { return glm::transpose(glm::make_mat4(&m.a1)); }

Model::Model(string const &path, bool animated, bool gamma)
{
	isAnimated = animated;
	gammaCorrection = gamma;
	loadModel(path);

	if(m_NumBones)
		BoneTransform(0.0f);

}

//TODO
//bool Model::isCollided(glm::vec3 myPos, Model * other, glm::vec3 otherPos) {
//	return bounding_sphere->isCollided(myPos, other->bounding_sphere, otherPos);
//}

// draws the model, and thus all its meshes
void Model::draw(Shader * shader, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx)
{
	glm::mat4 modelMtx = parentMtx * localMtx;
	shader->use();
	shader->setMat4("ModelMtx", modelMtx);
	shader->setMat4("ModelViewProjMtx", viewProjMtx * modelMtx);

	for (unsigned int i = 0; i < boneTransforms.size(); i++) {
		shader->setMat4(m_BoneInfo[i].BoneLocation, boneTransforms[i]);
	}

	for (unsigned int i = 0; i < meshes.size(); i++)
		meshes[i].draw(shader, viewProjMtx);
}

void Model::BoneTransform(float TimeInSeconds)
{
	glm::mat4 Identity = glm::mat4(1.0f);

	//unsigned int animationClipIndex = AnimationMode;
	//for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
	//	if (scene->mAnimations[i]->mName.data == AnimationName) {
	//		animationClipIndex = i;
	//		break;
	//	}
	//}
	float TicksPerSecond = scene->mAnimations[animationMode]->mTicksPerSecond != 0 ?
		scene->mAnimations[animationMode]->mTicksPerSecond : 25.0f;
	float TimeInTicks = TimeInSeconds * TicksPerSecond;
	float AnimationTime = fmod(TimeInTicks, scene->mAnimations[animationMode]->mDuration);

	ReadNodeHeirarchy(AnimationTime, scene->mRootNode, Identity);

	boneTransforms.resize(m_NumBones);

	for (unsigned int i = 0; i < m_NumBones; i++) {
		boneTransforms[i] = m_BoneInfo[i].FinalTransformation;
		auto m = boneTransforms[i];
		/*printf("\n\nbone transform is: %f, %f, %f, %f, \n%f, %f, %f, %f, \n%f, %f, %f, %f, \n%f, %f, %f, %f", m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);*/
	}
}


void Model::loadModel(string const &path)
{
	// read file via ASSIMP
	scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs); // | aiProcess_GenSmoothNormals);
	// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return;
	}
	// retrieve the directory path of the filepath
	directory = path.substr(0, path.find_last_of('/'));

	// store global inverse transform matrix
	if (scene) {
		globalInverseTransform = glm::inverse(aiM4x4toGlmMat4(scene->mRootNode->mTransformation));
	}

	unsigned int numVertices = 0;
	unsigned int numIndices = 0;

	// process each mesh located at the current node
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		// the node object only contains indices to index the actual objects in the scene. 
		// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
		meshesData.push_back(
			MeshData{ scene->mMeshes[i]->mMaterialIndex,
			scene->mMeshes[i]->mNumFaces * 3,
			numVertices,
			numIndices }
		);
		numVertices += scene->mMeshes[i]->mNumVertices;
		numIndices += meshesData[i].numIndices;
	}

	// process ASSIMP's root node recursively
	processNode(scene->mRootNode, scene);

	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		processMesh(vertices, indices, textures, i, scene->mMeshes[i], scene);
	}
	if(!isAnimated)
		normalize(vertices);
	/*std::vector<glm::vec3> verticePosVector;
	for (auto& vertex : vertices) {
		verticePosVector.push_back(vertex.Position);
	}
	const float * flat_array = &verticePosVector[0].x;
	bounding_sphere = new Sphere(flat_array, (unsigned)vertices.size(), sizeof(float[3]), 3, NULL, 500, 36, 18, true);*/
	meshes.push_back(Mesh(vertices, indices, textures));

	for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
		m_Animations.push_back(scene->mAnimations[i]->mName.data);
		m_AnimationMapping[scene->mAnimations[i]->mName.data] = i;
		printf(scene->mAnimations[i]->mName.data);
		m_ChannelMapping.push_back(map<string, unsigned int>());
		for (unsigned int j = 0; j < scene->mAnimations[i]->mNumChannels; j++) {
			m_ChannelMapping[i][scene->mAnimations[i]->mChannels[j]->mNodeName.data] = j;
		}
	}
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void Model::processNode(aiNode *node, const aiScene *scene)
{
	// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		processNode(node->mChildren[i], scene);
	}

}

void Model::normalize(vector<Vertex>& vertices) {
	float min_x, max_x, min_y, max_y, min_z, max_z;
	min_x = 100000.0f;
	max_x = -100000.0f;
	min_y = 100000.0f;
	max_y = -100000.0f;
	min_z = 100000.0f;
	max_z = -100000.0f;
	for (int i = 0; i < vertices.size(); i++) {
		if (min_x > vertices[i].Position.x)
			min_x = vertices[i].Position.x;
		if (max_x < vertices[i].Position.x)
			max_x = vertices[i].Position.x;

		if (min_y > vertices[i].Position.y)
			min_y = vertices[i].Position.y;
		if (max_y < vertices[i].Position.y)
			max_y = vertices[i].Position.y;

		if (min_z > vertices[i].Position.z)
			min_z = vertices[i].Position.z;
		if (max_z < vertices[i].Position.z)
			max_z = vertices[i].Position.z;
	}
	float mid_x = (min_x + max_x) / 2;
	float mid_y = (min_y + max_y) / 2;
	float mid_z = (min_z + max_z) / 2;

	float range_x = max_x - min_x;
	float max = range_x;
	float range_y = max_y - min_y;
	float range_z = max_z - min_z;
	if (max < range_y)
		max = range_y;
	if (max < range_z)
		max = range_z;

	//printf("The model is centered at : %f %f %f \n", mid_x, mid_y, mid_z);
	for (int i = 0; i < vertices.size(); i++) {
		vertices[i].Position.x = (vertices[i].Position.x - mid_x);// *2 / max;
		vertices[i].Position.y = (vertices[i].Position.y - mid_y) + range_y/2;// *2 / max;
		vertices[i].Position.z = (vertices[i].Position.z - mid_z);// *2 / max;
	}
}


void Model::processMesh(vector<Vertex>& vertices, vector<unsigned int>& indices, vector<Texture>& textures, unsigned int meshIndex, aiMesh *mesh, const aiScene *scene)
{
	// Walk through each of the mesh's vertices
	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;
		glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
							// positions
		vector.x = mesh->mVertices[i].x;
		vector.y = mesh->mVertices[i].y;
		vector.z = mesh->mVertices[i].z;
		vertex.Position = vector;
		// normals
		vector.x = mesh->mNormals[i].x;
		vector.y = mesh->mNormals[i].y;
		vector.z = mesh->mNormals[i].z;
		vertex.Normal = vector;
		// texture coordinates
		if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
		{
			glm::vec2 vec;
			// a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
			// use models where a vertex can have multiple texture coordinates so we always take the first set (0).
			vec.x = mesh->mTextureCoords[0][i].x;
			vec.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = vec;
		}
		else
			vertex.TexCoords = glm::vec2(0.0f, 0.0f);
		vertices.push_back(vertex);
	}
	//printf("size of vertices is %d", vertices.size());
	//if(!isAnimated)
	//	normalize(vertices);
	// now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
	// process materials
	aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
	// we assume a convention for sampler names in the shaders. Each diffuse texture should be named
	// as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
	// Same applies to other texture as the following list summarizes:
	// diffuse: texture_diffuseN
	// specular: texture_specularN
	// normal: texture_normalN

	// 1. diffuse maps
	vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
	textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
	// 2. specular maps
	vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
	textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	// 3. normal maps
	std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
	textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
	// 4. height maps
	std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
	textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

	// process bones
	LoadBones(meshIndex, mesh, vertices);
}

// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
vector<Texture> Model::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
{
	vector<Texture> textures;
	for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString str;
		mat->GetTexture(type, i, &str);
		// check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
		bool skip = false;
		for (unsigned int j = 0; j < textures_loaded.size(); j++)
		{
			if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
			{
				textures.push_back(textures_loaded[j]);
				skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
				break;
			}
		}
		if (!skip)
		{   // if texture hasn't been loaded already, load it
			Texture texture;
			//texture.id = TextureFromFile(str.C_Str(), this->directory);
			texture.type = typeName;
			texture.path = str.C_Str();
			textures.push_back(texture);
			textures_loaded.push_back(texture);  // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
		}
	}
	return textures;
}

void Model::LoadBones(unsigned int meshIndex, const aiMesh* mesh, vector<Vertex> &vertices)
{
	for (unsigned int i = 0; i < mesh->mNumBones; i++) {
		unsigned int BoneIndex = 0;
		string BoneName(mesh->mBones[i]->mName.data);

		if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
			BoneIndex = m_BoneInfo.size();
			m_NumBones++;
			BoneInfo bi;
			m_BoneInfo.push_back(bi);
			m_BoneMapping[BoneName] = BoneIndex;
			m_BoneInfo[BoneIndex].BoneOffset = aiM4x4toGlmMat4(mesh->mBones[i]->mOffsetMatrix);
			m_BoneInfo[BoneIndex].BoneLocation = "BoneMtx[";
			m_BoneInfo[BoneIndex].BoneLocation.append(std::to_string(BoneIndex));
			m_BoneInfo[BoneIndex].BoneLocation.append("]");
			m_BoneInfo[BoneIndex].BoneName = BoneName;
		}
		else {
			BoneIndex = m_BoneMapping[BoneName];
		}

		for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
			unsigned int VertexID = meshesData[meshIndex].baseVertex + mesh->mBones[i]->mWeights[j].mVertexId;
			float Weight = mesh->mBones[i]->mWeights[j].mWeight;
			for (unsigned int k = 0; k < NUM_BONES_PER_VERTEX; k++) {
				if (vertices[VertexID].Weights[k] == 0.0) {
					vertices[VertexID].IDs[k] = BoneIndex;
					vertices[VertexID].Weights[k] = Weight;
					//printf("set vertex %d index %d with weight %f of bone %d\n", VertexID, k, Weight, BoneIndex);
					break;
				}
				if (k == NUM_BONES_PER_VERTEX - 1) {
					printf("not enough bone index space on vertex %d", VertexID);
				}
			}
		}
	}
}

glm::mat4 aiM4x4toGlmMat4(aiMatrix4x4 m) {
	return glm::transpose(glm::mat4(m[0][0], m[0][1], m[0][2], m[0][3],
		m[1][0], m[1][1], m[1][2], m[1][3],
		m[2][0], m[2][1], m[2][2], m[2][3],
		m[3][0], m[3][1], m[3][2], m[3][3]));
}

glm::mat4 aiM3x3toGlmMat4(aiMatrix3x3 m) {
	return glm::mat4(m[0][0], m[0][1], m[0][2], 0,
		m[1][0], m[1][1], m[1][2], 0,
		m[2][0], m[2][1], m[2][2], 0,
		0, 0, 0, 1);
}

void Model::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
{
	string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = scene->mAnimations[animationMode];

	glm::mat4 NodeTransformation = aiM4x4toGlmMat4(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = pAnimation->mChannels[m_ChannelMapping[animationMode][NodeName]];

	if (pNodeAnim) {
		// Interpolate scaling and generate scaling transformation matrix
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), glm::vec3(Scaling.x, Scaling.y, Scaling.z));

		// Interpolate rotation and generate rotation transformation matrix
		aiQuaternion RotationQ;
		CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
		glm::mat4 RotationM = glm::mat4_cast(glm::quat(RotationQ.w, RotationQ.x, RotationQ.y, RotationQ.z));
		//glm::mat4 RotationM = aiM4x4toGlmMat4(aiMatrix4x4(RotationQ.GetMatrix()));

		// Interpolate translation and generate translation transformation matrix
		aiVector3D Translation;
		CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
		glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), glm::vec3(Translation.x, Translation.y, Translation.z));

		// Combine the above transformations
		NodeTransformation = TranslationM * 
			//glm::mat4(glm::mat3(mat4_cast(pNode->mTransformation))) * 
			RotationM * 
			ScalingM;
	}

	glm::mat4 WorldTransformation = ParentTransform * NodeTransformation;

	if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
		unsigned int BoneIndex = m_BoneMapping[NodeName];
		m_BoneInfo[BoneIndex].FinalTransformation = globalInverseTransform * WorldTransformation *
			m_BoneInfo[BoneIndex].BoneOffset;
	}

	for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], WorldTransformation);
	}
}

void Model::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	AnimationTime = fmod(AnimationTime, ((float)pNodeAnim->mScalingKeys[pNodeAnim->mNumScalingKeys - 1].mTime - (float)pNodeAnim->mScalingKeys[0].mTime))
		+ (float)pNodeAnim->mScalingKeys[0].mTime;

	unsigned int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	unsigned int NextScalingIndex = (ScalingIndex + 1);
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float DeltaTime = pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime;
	float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& StartScalingV = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& EndScalingV = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	Out = Factor * StartScalingV + (1 - Factor) * EndScalingV;
	//Out = Out.Normalize();
}

unsigned int Model::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);
}

void Model::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	AnimationTime = fmod(AnimationTime, ((float)pNodeAnim->mRotationKeys[pNodeAnim->mNumRotationKeys - 1].mTime - (float)pNodeAnim->mRotationKeys[0].mTime))
		+ (float)pNodeAnim->mRotationKeys[0].mTime;

	unsigned int RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	unsigned int NextRotationIndex = (RotationIndex + 1);
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float DeltaTime = pNodeAnim->mRotationKeys[NextRotationIndex].mTime - pNodeAnim->mRotationKeys[RotationIndex].mTime;
	float Factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiQuaternion& StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
	Out = Out.Normalize();
}

unsigned int Model::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);
}

void Model::CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	// we need at least two values to interpolate...
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	AnimationTime = fmod(AnimationTime, ((float)pNodeAnim->mPositionKeys[pNodeAnim->mNumPositionKeys - 1].mTime - (float)pNodeAnim->mPositionKeys[0].mTime))
		+ (float)pNodeAnim->mPositionKeys[0].mTime;

	unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
	unsigned int NextPositionIndex = (PositionIndex + 1);
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float DeltaTime = pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime;
	float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
	assert(Factor >= 0.0f && Factor <= 1.0f);
	const aiVector3D& StartPositionV = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& EndPositionV = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	Out = Factor * StartPositionV + (1 - Factor) * EndPositionV;
	//if (Out.SquareLength() != 0) Out = Out.Normalize();
}

unsigned int Model::FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumPositionKeys > 0);

	for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);
}

//unsigned int TextureFromFile(const char *path, const string &directory, bool gamma)
//{
//	string filename = string(path);
//	filename = directory + '/' + filename;
//
//	unsigned int textureID;
//	glGenTextures(1, &textureID);
//
//	int width, height, nrComponents;
//	unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
//	if (data)
//	{
//		GLenum format;
//		if (nrComponents == 1)
//			format = GL_RED;
//		else if (nrComponents == 3)
//			format = GL_RGB;
//		else if (nrComponents == 4)
//			format = GL_RGBA;
//
//		glBindTexture(GL_TEXTURE_2D, textureID);
//		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
//		glGenerateMipmap(GL_TEXTURE_2D);
//
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
//		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//
//		stbi_image_free(data);
//	}
//	else
//	{
//		std::cout << "Texture failed to load at path: " << path << std::endl;
//		stbi_image_free(data);
//	}
//
//	return textureID;
//}