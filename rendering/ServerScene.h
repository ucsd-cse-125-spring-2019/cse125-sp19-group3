#ifndef _WINDOW_H_
#define _WINDOW_H_

#include <iostream>
#include <vector>
#include <GL/glew.h>
#include <queue> 

#include "Cube.h"
#include "shader.h"
#include "Model.h"
#include "Camera.h"
#include "Transform.h"
#include "../rendering/ScenePlayer.h"
#include "SceneProjectile.h"
#include "Audio.h"

// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "../shader.vert"
#define FRAGMENT_SHADER_PATH "../shader.frag"

class ScenePlayer;

class ServerScene {
public:
	int width;
	int height;

	// pointers to servers leaderBoard & player metadata map
	LeaderBoard* leaderBoard;		
	unordered_map<unsigned int, PlayerMetadata*>* playerMetadatas;	

	unordered_map<unsigned int, ScenePlayer> scenePlayers;

	//for player and projectiles
	unordered_map<unsigned int, float> model_radius;

	//for envs
	std::unordered_map<unsigned int, Point> model_boundingbox;
	std::vector<Transform *> env_objs;
	std::vector<SceneProjectile> skills;
	std::vector<int> soundsToPlay;
	bool warriorIsCharging = false;

  //TODO: fill in the four corner
	std::vector<glm::vec3> spawn_loc{
								  //top left
                                  glm::vec3(-0.169116f, 0.0f, -19.966980f),
								  //bottom left
                                  glm::vec3(-0.271529f, 0.0f, 75.787369f),
								  //bottom right
                                  glm::vec3(154.582565f, 0.0f, 75.737686f),
								  //top right
                                  glm::vec3(154.779358f, 0.0f, -18.759279f)
                                  };

	// constructor
	ServerScene(LeaderBoard* leaderBoard, unordered_map<unsigned int, PlayerMetadata*>* playerMetadatas, 
		unordered_map<unsigned int, Skill>* skill_map, unordered_map<ArcheType, vector<unsigned int>> *archetype_skillset);
	~ServerScene();

	void resetScene();
	void addPlayer(unsigned int playerId, ArcheType modelType);
	void update();
	void handlePlayerMovement(unsigned int player_id, glm::vec3 destination);

	void createSceneProjectile(unsigned int player_id,
								Point finalPoint,
								Point initPoint,
								Skill adjustedSkill,
								float x, float z);

	void handlePyroBlast(unsigned int player_id, Point finalPoint, Point initPoint, Skill adjustedSkill);

	void handleWhirlWind(unsigned int player_id, Point finalPoint, Point initPoint, Skill adjustedSkill);

	void handleRoyalCross(unsigned int player_id, Point finalPoint, Point initPoint, Skill adjustedSkill);

	void handlePlayerSkill(unsigned int player_id, 
		                   Point finalPoint, 
		                   int skill_id, 
		                   unordered_map<unsigned int, Skill> *skill_map,
		                   PlayerMetadata* playerMetadata);

	void handlePlayerDeath(ScenePlayer &player, unsigned int killer_id);
	void handlePlayerRespawn(unsigned int client_id);

	void checkAndHandlePlayerCollision(unsigned int playerId);
	/*unsigned int serializeInitScene(char* data, unsigned int playerId, unsigned int playerRootId);
	unsigned int serializeSceneGraph(char* data);
	std::pair<char *, unsigned int> serializeSceneGraph(Transform* t, char* data);*/
	Transform * getRoot();
	unordered_map<unsigned int, Transform *> serverSceneGraphMap;
	unordered_map<ArcheType, vector<unsigned int>> *archetype_skillset;
	void initEnv();
	void initModelPhysics();
	unordered_map<unsigned int, Skill>* skill_map;

private:
	Transform * root;
	Transform * playerRoot;
	Transform * skillRoot;

	double time = 0.0;
	unsigned int nodeIdCounter = 0;
};

#endif
