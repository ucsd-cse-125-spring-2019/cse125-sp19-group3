#ifndef _MODEL_H_
#define _MODEL_H_

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "shader.h"
#include "../networking/KillStreak/CoreTypes.hpp"
//#include "Sphere.h"


#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>
#include <cmath>
using namespace std;

typedef struct{
	unsigned int texIndex;
	unsigned int numIndices;
	unsigned int baseVertex;
	unsigned int baseIndex;
} MeshData;



//unsigned int TextureFromFile(const char *path, const string &directory, bool gamma = false);

class Model
{
public:
	/*  Model Data */
	vector<Texture> textures_loaded;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
	vector<MeshData> meshesData;
	vector<Mesh> meshes;
	string directory;
	bool isAnimated;
	vector<string> m_Animations;
	unsigned int curr_mode = spawn;
	unsigned int prev_movementMode = idle;
	int movementMode = idle;
	int animationMode = -1;
	float animationTime;
	vector<vector<float>> animation_frames;
	glm::mat4 localMtx = glm::mat4(1.0f);

	//Sphere * bounding_sphere;
	/*  Functions   */
	// constructor, expects a filepath to a 3D model.
	Model(string const &path, string const &texPath, bool animated=false);

	// draws the model, and thus all its meshes
	void draw(Shader * shader, const glm::mat4 &parentMtx, const glm::mat4 &viewProjMtx);

	void BoneTransform(float TimeInSeconds);

	//TODO: collision detection. when doing collision detection, pass in location data(could get from translate matrix)
	//bool isCollided(glm::vec3 myPos, Model * other, glm::vec3 otherPos);

private:
	unsigned int textureId;

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
	void loadModel(string const &path);

	// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
	void processNode(aiNode *node, const aiScene *scene);

	void normalize(vector<Vertex>& vertices);

	void processMesh(vector<Vertex>& vertices, vector<unsigned int>& indices, vector<Texture>& textures, unsigned int meshIndex, aiMesh *mesh, const aiScene *scene);

	// checks all material textures of a given type and loads the textures if they're not loaded yet.
	// the required info is returned as a Texture struct.
	vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
	
	void LoadBones(unsigned int meshIndex, const aiMesh* mesh, vector<Vertex> &vertices);

	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const glm::mat4& ParentTransform);

	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	unsigned int FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);

	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	unsigned int FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	
	void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);

	unsigned int FindPosition(float AnimationTime, const aiNodeAnim* pNodeAnim);
};

#endif