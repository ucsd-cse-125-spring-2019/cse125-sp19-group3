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
#include "ScenePlayer.h"
#include "SceneProjectile.h"

// On some systems you need to change this to the absolute path
#define VERTEX_SHADER_PATH "../shader.vert"
#define FRAGMENT_SHADER_PATH "../shader.frag"

class ServerScene {
public:
	int width;
	int height;

	LeaderBoard* leaderBoard;		// pointer to servers leaderBoard

	unordered_map<unsigned int, ScenePlayer> scenePlayers;

	//for player and projectiles
	unordered_map<unsigned int, float> model_radius;

	//for envs
	std::unordered_map<unsigned int, Point> model_boundingbox;
	std::vector<Transform *> env_objs;
	std::vector<SceneProjectile> skills;

	// constructor
	ServerScene(LeaderBoard* leaderBoard);
	~ServerScene();

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
		                   unsigned int skill_id, 
		                   unordered_map<unsigned int, Skill> *skill_map,
		                   PlayerMetadata &playerMetadata);

	void handlePlayerDeath(ScenePlayer &player, unsigned int killer_id);

	void checkAndHandlePlayerCollision(unsigned int playerId);
	/*unsigned int serializeInitScene(char* data, unsigned int playerId, unsigned int playerRootId);
	unsigned int serializeSceneGraph(char* data);
	std::pair<char *, unsigned int> serializeSceneGraph(Transform* t, char* data);*/
	Transform * getRoot();
	unordered_map<unsigned int, Transform *> serverSceneGraphMap;
	void initEnv();
	void initModelPhysics();

private:
	Transform * root;
	Transform * playerRoot;
	Transform * skillRoot;

	double time = 0.0;
	unsigned int nodeIdCounter = 0;
};

#endif
