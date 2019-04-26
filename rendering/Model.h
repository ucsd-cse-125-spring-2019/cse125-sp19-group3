#ifndef _MODEL_H_
#define _MODEL_H_

#define GLFW_INCLUDE_GLEXT
#ifdef __APPLE__
#define GLFW_INCLUDE_GLCOREARB
#else
#include <GL/glew.h>
#endif
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "shader.h"
//#include "stb_image.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
using namespace std;

//unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model
{
public:
	/*  Model Data */
	vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	vector<Mesh> meshes;
	string directory;
	bool gammaCorrection;

	/*  Functions   */
	// constructor, expects a filepath to a 3D model.
	Model(string const &path, bool gamma = false) : gammaCorrection(gamma)
	{
		loadModel(path);
		BoneTransform("Root|Idle", 0.0f);
	}

	// draws the model, and thus all its meshes
	void draw(Shader * shader, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx)
	{
		glm::mat4 modelMtx = parentMtx * localMtx;
		shader->use();
		shader->setMat4("ModelMtx", modelMtx);
		shader->setMat4("ModelViewProjMtx", viewProjMtx * modelMtx);
		shader->setNMat4("BoneMtx", boneTransforms[0], boneTransforms.size());
		for (unsigned int i = 0; i < meshes.size(); i++)
			meshes[i].draw(shader, viewProjMtx);
	}

	void BoneTransform(string AnimationName, float TimeInSeconds)
	{
		glm::mat4 Identity = glm::mat4(1.0f);

		float TicksPerSecond = scene->mAnimations[0]->mTicksPerSecond != 0 ?
			scene->mAnimations[0]->mTicksPerSecond : 25.0f;
		float TimeInTicks = TimeInSeconds * TicksPerSecond;
		float AnimationTime = fmod(TimeInTicks, scene->mAnimations[0]->mDuration);

		ReadNodeHeirarchy(AnimationName, AnimationTime, scene->mRootNode, Identity);

		boneTransforms.resize(m_NumBones);

		for (unsigned int i = 0; i < m_NumBones; i++) {
			boneTransforms[i] = m_BoneInfo[i].FinalTransformation;
		}
	}

private:

	glm::mat4 localMtx = glm::mat4(1.0f);
	glm::mat4 globalInverseTransform;

	Assimp::Importer importer;
	const aiScene* scene;
	map<string, unsigned int> m_BoneMapping;
	unsigned int m_NumBones = 0;
	vector<BoneInfo> m_BoneInfo;
	map<string, unsigned int> m_AnimationMapping;
	vector<map<string, unsigned int>> m_ChannelMapping;

	vector<glm::mat4> boneTransforms;

	/*  Functions   */
	// loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
	void loadModel(string const &path)
	{
		// read file via ASSIMP
		scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
		// check for errors
		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
		{
			cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
			return;
		}
		// retrieve the directory path of the filepath
		directory = path.substr(0, path.find_last_of('/'));

		// process ASSIMP's root node recursively
		processNode(scene->mRootNode, scene);

		for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
			m_AnimationMapping[scene->mAnimations[i]->mName.data] = i;
			printf(scene->mAnimations[i]->mName.data);
			m_ChannelMapping.push_back(map<string, unsigned int>());
			for (unsigned int j = 0; j < scene->mAnimations[i]->mNumChannels; j++) {
				m_ChannelMapping[i][scene->mAnimations[i]->mChannels[j]->mNodeName.data] = j;
			}
		}
	}

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene)
	{
		// store global inverse transform matrix
		globalInverseTransform = glm::inverse(toGlmMat4(node->mTransformation));

		// process each mesh located at the current node
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			// the node object only contains indices to index the actual objects in the scene. 
			// the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			meshes.push_back(processMesh(mesh, scene));
		}
		// after we've processed all of the meshes (if any) we then recursively process each of the children nodes
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			processNode(node->mChildren[i], scene);
		}

	}

	Mesh processMesh(aiMesh *mesh, const aiScene *scene)
	{
		// data to fill
		vector<Vertex> vertices;
		vector<unsigned int> indices;
		vector<Texture> textures;

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
		LoadBones(mesh, vertices);

		// return a mesh object created from the extracted mesh data
		return Mesh(vertices, indices, textures);
	}

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
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

	void LoadBones(const aiMesh* mesh, vector<Vertex> &vertices)
	{
		for (unsigned int i = 0; i < mesh->mNumBones; i++) {
			unsigned int BoneIndex = 0;
			string BoneName(mesh->mBones[i]->mName.data);

			if (m_BoneMapping.find(BoneName) == m_BoneMapping.end()) {
				BoneIndex = m_NumBones;
				m_NumBones++;
				BoneInfo bi;
				m_BoneInfo.push_back(bi);
			}
			else {
				BoneIndex = m_BoneMapping[BoneName];
			}

			m_BoneMapping[BoneName] = BoneIndex;
			m_BoneInfo[BoneIndex].BoneOffset = toGlmMat4(mesh->mBones[i]->mOffsetMatrix);

			for (unsigned int j = 0; j < mesh->mBones[i]->mNumWeights; j++) {
				unsigned int VertexID = mesh->mBones[i]->mWeights[j].mVertexId;
				float Weight = mesh->mBones[i]->mWeights[j].mWeight;
				for (unsigned int i = 0; i < NUM_BONES_PER_VERTEX; i++) {
					if (vertices[VertexID].Weights[i] == 0.0) {
						vertices[VertexID].IDs[i] = BoneIndex;
						vertices[VertexID].Weights[i] = Weight;
						break;
					}
				}
			}
		}
	}

	glm::mat4 toGlmMat4(aiMatrix4x4 m) {
		return glm::mat4(m[0][0], m[0][1], m[0][2], m[0][3],
			m[1][0], m[1][1], m[1][2], m[1][3],
			m[2][0], m[2][1], m[2][2], m[2][3],
			m[3][0], m[3][1], m[3][2], m[3][3]);
	}

	glm::mat4 toGlmMat4(aiMatrix3x3 m) {
		return glm::mat4(m[0][0], m[0][1], m[0][2], 0,
			m[1][0], m[1][1], m[1][2], 0,
			m[2][0], m[2][1], m[2][2], 0,
			0, 0, 0, 1);
	}

	

	void ReadNodeHeirarchy(string AnimationName, float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform)
	{
		string NodeName(pNode->mName.data);

		const aiAnimation* pAnimation = scene->mAnimations[m_AnimationMapping[AnimationName]];

		glm::mat4 NodeTransformation = toGlmMat4(pNode->mTransformation);

		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[m_ChannelMapping[m_AnimationMapping[AnimationName]][NodeName]];

		if (pNodeAnim) {
			// Interpolate scaling and generate scaling transformation matrix
			aiVector3D Scaling;
			CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
			glm::mat4 ScalingM = glm::scale(glm::mat4(1.0f), glm::vec3(Scaling.x, Scaling.y, Scaling.z));

			// Interpolate rotation and generate rotation transformation matrix
			aiQuaternion RotationQ;
			CalcInterpolatedRotation(RotationQ, AnimationTime, pNodeAnim);
			glm::mat4 RotationM = toGlmMat4(RotationQ.GetMatrix());

			// Interpolate translation and generate translation transformation matrix
			aiVector3D Translation;
			CalcInterpolatedPosition(Translation, AnimationTime, pNodeAnim);
			glm::mat4 TranslationM = glm::translate(glm::mat4(1.0f), glm::vec3(Translation.x, Translation.y, Translation.z));

			// Combine the above transformations
			NodeTransformation = TranslationM * RotationM * ScalingM;
		}

		glm::mat4 WorldTransformation = ParentTransform * NodeTransformation;

		if (m_BoneMapping.find(NodeName) != m_BoneMapping.end()) {
			unsigned int BoneIndex = m_BoneMapping[NodeName];
			m_BoneInfo[BoneIndex].FinalTransformation = globalInverseTransform * WorldTransformation *
				m_BoneInfo[BoneIndex].BoneOffset;
		}

		for (unsigned int i = 0; i < pNode->mNumChildren; i++) {
			ReadNodeHeirarchy(AnimationName, AnimationTime, pNode->mChildren[i], WorldTransformation);
		}
	}

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumScalingKeys == 1) {
			Out = pNodeAnim->mScalingKeys[0].mValue;
			return;
		}

		unsigned int ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
		unsigned int NextScalingIndex = (ScalingIndex + 1);
		assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
		float DeltaTime = pNodeAnim->mScalingKeys[NextScalingIndex].mTime - pNodeAnim->mScalingKeys[ScalingIndex].mTime;
		float Factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& StartScalingV = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
		const aiVector3D& EndScalingV = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
		Out = Factor * StartScalingV + (1 - Factor) * EndScalingV;
		Out = Out.Normalize();
	}

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumScalingKeys > 0);

		for (unsigned int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);
	}

	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumRotationKeys == 1) {
			Out = pNodeAnim->mRotationKeys[0].mValue;
			return;
		}

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

	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumRotationKeys > 0);

		for (unsigned int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);
	}
	
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		// we need at least two values to interpolate...
		if (pNodeAnim->mNumPositionKeys == 1) {
			Out = pNodeAnim->mPositionKeys[0].mValue;
			return;
		}

		/*if (AnimationTime == 0) {
			Out = pNodeAnim->mPositionKeys[0].mValue;
			return;
		}*/

		unsigned int PositionIndex = FindPosition(AnimationTime, pNodeAnim);
		unsigned int NextPositionIndex = (PositionIndex + 1);
		assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
		float DeltaTime = pNodeAnim->mPositionKeys[NextPositionIndex].mTime - pNodeAnim->mPositionKeys[PositionIndex].mTime;
		float Factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / DeltaTime;
		assert(Factor >= 0.0f && Factor <= 1.0f);
		const aiVector3D& StartPositionV = pNodeAnim->mPositionKeys[PositionIndex].mValue;
		const aiVector3D& EndPositionV = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
		Out = Factor * StartPositionV + (1 - Factor) * EndPositionV;
		if (Out.SquareLength() != 0) Out = Out.Normalize();
	}

	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim)
	{
		assert(pNodeAnim->mNumPositionKeys > 0);

		for (unsigned int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++) {
			if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
				return i;
			}
		}

		assert(0);
	}
};

#endif