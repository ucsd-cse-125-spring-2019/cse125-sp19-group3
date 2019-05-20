#include "ServerScene.h"
#include "nlohmann\json.hpp"
#include <fstream>
#include "../networking/KillStreak/Logger.hpp"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
using json = nlohmann::json;

#define DEFAULT_X 666
#define DEFAULT_Z 666

// skill_id's
#define EVADE				0
#define PROJECTILE			1
#define PYROBLAST			2
#define DRAGONS_BREATH		3
#define ASSASSIN_PROJECTILE	11
#define INVISIBILITY		12
#define SIXTH_SENSE			13
#define WHIRLWIND			22
#define CHARGE				23
#define ROYAL_CROSS			32
#define SUBJUGATION			33
#define UNSILENCE           34


ServerScene::ServerScene()
{
	root = new Transform(0, glm::mat4(1.0f));
	serverSceneGraphMap.insert({ root->node_id, root });
	nodeIdCounter++;
	playerRoot = new Transform(nodeIdCounter, glm::mat4(1.0f));
	serverSceneGraphMap.insert({ playerRoot->node_id, playerRoot });
	root->addChild(nodeIdCounter);
	nodeIdCounter++;
	skillRoot = new Transform(nodeIdCounter, glm::mat4(1.0f));
	serverSceneGraphMap.insert({ skillRoot->node_id, skillRoot });
	root->addChild(nodeIdCounter);

	initModelPhysics();
}

ServerScene::~ServerScene() {
	delete root;
}

void processMesh(vector<Vertex>& vertices, vector<unsigned int>& indices, aiMesh *mesh)
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
	
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		// retrieve all indices of the face and store them in the indices vector
		for (unsigned int j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}
}
	// process materials
glm::vec3 findDimensions(vector<Vertex>& vertices) {
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
	return glm::vec3(range_x, range_y, range_z);
}


glm::vec3 findCubeDimensions(string const &path) {
	Assimp::Importer importer;
	auto scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs); // | aiProcess_GenSmoothNormals);
																				// check for errors
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
	{
		cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
		return glm::vec3(1.0f);
	}
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;

	for (unsigned int i = 0; i < scene->mNumMeshes; i++) {
		processMesh(vertices, indices, scene->mMeshes[i]);
	}
	return findDimensions(vertices);
	
}

void ServerScene::initModelPhysics() {
	ifstream json_file("../model_paths.json");
	json jsonObjs = json::parse(json_file);
	for (auto & obj : jsonObjs["data"]) {
		model_radius.insert({ (unsigned int)obj["model_id"], (float)obj["radius"] });
		if (!obj["animated"]) {
			glm::vec3 size = findCubeDimensions(obj["path"]);
			model_boundingbox.insert({ (unsigned int)obj["model_id"],  size });
		}
	}
}


void ServerScene::initEnv() {
	ifstream json_file("../env_model_locations.json");
	unsigned int envCounter = 4000000000;
	json jsonObjs = json::parse(json_file);
	for (auto & obj : jsonObjs["data"]) {
		Transform * envobj = new Transform(envCounter++, glm::translate(glm::mat4(1.0f), glm::vec3((float)(obj["translate"][0]), (float)(obj["translate"][1]), (float)(obj["translate"][2]))),
		glm::rotate(glm::mat4(1.0f), (float)obj["rotate"] / 180.0f * glm::pi<float>(), glm::vec3(0, 1, 0)),
		glm::scale(glm::mat4(1.0f), glm::vec3((float)obj["scale"], (float)obj["scale"], (float)obj["scale"])));
		envobj->initialRotation = (float)obj["rotate"];
		envobj->model_ids.insert((int)obj["model_id"]);
		env_objs.push_back(envobj);
	}
}

void ServerScene::addPlayer(unsigned int playerId, ArcheType modelType) {
	nodeIdCounter++;
	Transform * playerObj = new Transform(nodeIdCounter, glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0)),
		glm::rotate(glm::mat4(1.0f), -90/ 180.0f * glm::pi<float>(), glm::vec3(1, 0, 0)),
		glm::scale(glm::mat4(1.0f), glm::vec3(0.05f, 0.05f, 0.05f)));
	playerObj->model_ids.insert(modelType);
	playerRoot->addChild(nodeIdCounter);
	serverSceneGraphMap.insert({ nodeIdCounter, playerObj });

	// TODO: need to set model type based on player selection in lobby

	ScenePlayer player = ScenePlayer(playerId, nodeIdCounter, modelType, playerObj);
	//playerMap.insert(std::pair<unsigned int, Player *>(playerId, player));
	scenePlayers.insert({ playerId, player });
}

void ServerScene::update()
{
	time += 1.0 / 60;
	auto skillIter = skills.begin();
	while (skillIter != skills.end()) {
		auto & skill = *skillIter;
		bool skillCollidedEnv = false;
		for (auto& envObj : env_objs) {
			glm::vec3 forwardVector = skill.direction*skill.speed;
			if (skill.node->isCollided(forwardVector, model_radius, serverSceneGraphMap, envObj, model_boundingbox, true)) {
				skillCollidedEnv = true;
			}
		}
		if (skill.outOfRange() || skillCollidedEnv) {
			serverSceneGraphMap.erase(skill.node->node_id);
			skillRoot->removeChild(skill.node->node_id);
			delete(skill.node);
			skillIter = skills.erase(skillIter);
		}
		else {
			skill.update();
			skillIter++;
		}
	}
	for (auto& element : scenePlayers) {
		checkAndHandlePlayerCollision(element.first);
		element.second.update();
	}
}

//TODO: Refactoring, moving collision check to player??? Also find radius for various objs.
void ServerScene::checkAndHandlePlayerCollision(unsigned int playerId) {
	ScenePlayer &player = scenePlayers[playerId];
	glm::vec3 forwardVector = (player.destination - player.currentPos)* player.speed;
	if(glm::length(forwardVector)>1)
		forwardVector = glm::normalize(player.destination - player.currentPos)* player.speed;
	for (auto& skill : skills) {
		if (skill.ownerId == playerId) {
			continue;
		}
		else if (player.playerRoot->isCollided(forwardVector, model_radius, serverSceneGraphMap, skill.node, model_boundingbox, false)) {
			player.setDestination(player.currentPos);
			printf("Player %d killed Player %d \n", skill.ownerId, playerId);
			return;
		}
	}
	for (auto& envObj : env_objs) {
		if (player.playerRoot->isCollided(forwardVector, model_radius, serverSceneGraphMap, envObj, model_boundingbox, true)) {
			player.setDestination(player.currentPos);
			break;
		}
	}

}

void ServerScene::handlePlayerMovement(unsigned int playerId, glm::vec3 destination)
{
	ScenePlayer &player = scenePlayers[playerId];
	player.setDestination(destination);
	float dotResult = glm::dot(glm::normalize(destination - player.currentPos), player.currentOri);

	if (abs(dotResult) < 1.0) {
		float angle = glm::acos(dotResult);
		//printf("rotate angle = %f", angle);
		glm::vec3 axis = glm::cross(player.currentOri, glm::normalize(destination - player.currentPos));
		if (glm::length(axis) != 0) {
			player.rotate(angle, axis);
		}
	}
}


/*
	Create a projectile and add it to the server scene. 
	For cases of non-default projectile like Pyroblast you must specify values x,z.

	x & z: Default values set as arguments, when either is non-default 'finalPoint' will be 
		updated adding x & y to the co-ordinate.
		--> See handlePyroBlast for non-default example
*/
void ServerScene::createSceneProjectile(unsigned int player_id, Point finalPoint, Point initPoint, Skill adjustedSkill,
	float x=DEFAULT_X, float z=DEFAULT_Z)
{
	// modify final point for non default projectile (e.g. Pyroblast)
	if ( x != DEFAULT_X || z != DEFAULT_Z ) finalPoint = initPoint + Point({x, 0.0f, z});

	nodeIdCounter++;
	SceneProjectile proj = SceneProjectile(nodeIdCounter, player_id, initPoint, finalPoint, skillRoot, adjustedSkill.speed, adjustedSkill.range);
	serverSceneGraphMap.insert({ nodeIdCounter, proj.node });
	skills.push_back(proj);
}


/*
	Handle Mage's pyroblast by creating projectiles in a circular motion around the player.
*/
void ServerScene::handlePyroBlast(unsigned int player_id, Point finalPoint, Point initPoint, Skill adjustedSkill)
{
	for (float z = -1; z < 2; z++) {
		for (float x = -1; x < 2; x++) {
			if (x == 0 && z == 0) continue;		// only shoot left and right from center

			// create projectile; check if want to make more for this coordinate
			createSceneProjectile(player_id, finalPoint, initPoint, adjustedSkill, x, z);
			if ((x == 0) && (abs(z) == 1)) // +/- 0.5 to x-axis
			{
				createSceneProjectile(player_id, finalPoint, initPoint, adjustedSkill, x - 0.5, z);
				createSceneProjectile(player_id, finalPoint, initPoint, adjustedSkill, x + 0.5, z);
			}
			else if (z == 0 && (abs(x) == 1)) // +/- 0.5 to z-axis
			{
				createSceneProjectile(player_id, finalPoint, initPoint, adjustedSkill, x, z - 0.5);
				createSceneProjectile(player_id, finalPoint, initPoint, adjustedSkill, x, z + 0.5);
			}
		}

	}

}


/*
	Handle Warriors's whirlwind by creating projectiles in a circular motion around the player.

	NOTE: Currently just delegates to handlePyroBlast incase we decide to have different logic. 
*/
void ServerScene::handleWhirlWind(unsigned int player_id, Point finalPoint, Point initPoint, Skill adjustedSkill)
{
	handlePyroBlast(player_id, finalPoint, initPoint, adjustedSkill);
}


/*
	Handle King's royal cross by creating projectiles towards the top/bottom & sides of player.
*/
void ServerScene::handleRoyalCross(unsigned int player_id, Point finalPoint, Point initPoint, Skill adjustedSkill)
{
	for (float z = -1; z < 2; z++) {
		for (float x = -1; x < 2; x++) {

			if (abs(z) == 1 && x == 0) {	  // shoot up & down
				createSceneProjectile(player_id, finalPoint, initPoint, adjustedSkill, x, z);
			}

			else if (abs(x) == 1 && z == 0) { // shoot left & right
				createSceneProjectile(player_id, finalPoint, initPoint, adjustedSkill, x, z);
			}

		}
	}

}


/*
	Handle incoming player skill received from client's incoming packet. Match skill_id and handle skill accordingly.
*/
void ServerScene::handlePlayerSkill(unsigned int player_id, Point finalPoint,
	unsigned int skill_id, unordered_map<unsigned int, Skill> *skill_map, PlayerMetadata &playerMetadata)
{
	// special case of unsilence
	if (skill_id == UNSILENCE) {
		logger()->debug("everyone is unsilenced");
		for (auto& player : scenePlayers) {
			player.second.isSilenced = false;
		}
		return;
	}

	// don't handle class skills if the player is silenced
	if (scenePlayers[player_id].isSilenced) {
		if (skill_id != PROJECTILE && skill_id != EVADE) {
			return;
		}
	}
	// get skill & adjust accordinglyt o level
	auto level = playerMetadata.skillLevels[skill_id];
	unordered_map<unsigned int, Skill>::iterator s_it = skill_map->find(skill_id);
	Skill cur_skill = s_it->second;
	Skill adjustedSkill = Skill::calculateSkillBasedOnLevel(cur_skill, level);
	Point initPoint = scenePlayers[player_id].currentPos;

	skill_id = (skill_id % 10 == 1) ? 1 : skill_id;		// check if projectile; update skill_id if true
	switch (skill_id)
	{
		case PROJECTILE:
		{
			createSceneProjectile(player_id, finalPoint, initPoint, adjustedSkill);
			break;
		}
		case PYROBLAST:
		{
			handlePyroBlast(player_id, finalPoint, initPoint, adjustedSkill);
			break;
		}
		case DRAGONS_BREATH:
		{

			// TODO: Possibly modfiy 'createSceneProjectile' to handle this specific case, will need to update dirAOE.node-> scale...
			// can define a default value that will state what to do
			nodeIdCounter++;
			initPoint = scenePlayers[player_id].currentPos + Point({ 0.0f, 5.0f, 0.0f });
			SceneProjectile dirAOE = SceneProjectile(nodeIdCounter, player_id, initPoint, finalPoint, skillRoot, adjustedSkill.speed, adjustedSkill.range);
			dirAOE.node->scale = glm::scale(glm::mat4(1.0f), Point(0.08f, 0.08f, 0.08f));
			serverSceneGraphMap.insert({ nodeIdCounter, dirAOE.node });
			skills.push_back(dirAOE);
			break;
		}
		case WHIRLWIND:
			handleWhirlWind(player_id, finalPoint, initPoint, adjustedSkill);
			break;
		case ROYAL_CROSS:
			handleRoyalCross(player_id, finalPoint, initPoint, adjustedSkill);
			break;
		case INVISIBILITY:
		{
			// be a little sneaky: every time i fire this, just flip the invisibility.
			// client must send another skill packet after duration is over.
			int node_id = scenePlayers[player_id].root_id;
			serverSceneGraphMap[node_id]->enabled = !(serverSceneGraphMap[node_id]->enabled);
			break;
		}
		case SUBJUGATION:
		{
			// TODO: delay this skill by a bit to let animations/particle fx to play
			// and then do hitbox calculations

			auto& king = scenePlayers[player_id];

			// grab all players who are in the range of the skill. @roy not sure if this is the right way to handle hit detection??
			for (auto& player : scenePlayers) {
				if (player_id == player.first) {
					continue;
				}
				if (glm::length(king.currentPos - player.second.currentPos) <= adjustedSkill.range) {
					player.second.isSilenced = true;
					logger()->debug("Player {} (model: {}) was silenced", player.first, player.second.modelType);
				}
			}
			break;
		}
		default:
		{
			logger()->error("Skill_id {} not found!", skill_id);
			break;
		}

	}
} 


Transform * ServerScene::getRoot() {
	return root;
}