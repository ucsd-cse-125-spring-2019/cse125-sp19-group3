#include "CoreTypes.hpp"
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <algorithm>    // std::sort
#include <vector>
#include "INIReader.h"

using namespace std;

static int PLAYERNUM = 4;

class LeaderBoard {
public:
	LeaderBoard(vector<int> initial_prize, int change) : prizeChange(change) { prizes = initial_prize;}
	LeaderBoard() : currentKills(vector<int>(PLAYERNUM, 0)), currPoints(vector<int>(PLAYERNUM, 0)) {}
	~LeaderBoard() {}

	vector<int>* roundSummary (); // Update vectors for the round and return the ranking of each player
	vector<int> getCurrPoints() {return currPoints;}

protected:
	vector<int> currentKills;	// # of kills in each round of each player
	vector<int> currPoints;		// accumulative points of each player
	vector<int> prizes;							// points added to player each round based on ranking
	float prizeChange;							// prizes increases per round (1.2)
};

typedef enum{MELEE, PROJECTILE, AOE, MINIMAP, INVISIBLE, CHARGE, DEFAULT_SKILLTYPE} SkillType;
class Skill {
public:
	string skillName;
	unsigned int initialLevel;
	unsigned int skill_id;
	float range;
	float cooldown;
	float duration;
	float speed;
	Skill(unsigned int skill_id, unsigned int initialLevel, string skillName, float range, float cooldown, float duration, float speed) {
		this->skillName = skillName;
		this->initialLevel = initialLevel;
		this->skill_id = skill_id;
		this->range = range;
		this->cooldown = cooldown;
		this->duration = duration;
		this->speed = speed;
	};
	Skill() : skillName("Default Skill"), range(-1), cooldown(-1), duration(-1), speed(-1) {}
	~Skill(){}
	static void load_archtype_data(unordered_map<unsigned int, Skill> &skill_map, 
		                           unordered_map<ArcheType, vector<unsigned int>> &archetype_skill_set);
	static Skill calculateSkillBasedOnLevel(Skill &baseSkill, unsigned int level);
};

/*
	Stores players metadata (id, name, type, gold, killstreak, death status).
*/
class PlayerMetadata {
public:
	PlayerMetadata(unsigned int clientId, 
		           std::string username, 
		           ArcheType type, 
		           unordered_map<unsigned int, Skill> &skill_map, 
		           unordered_map<ArcheType, vector<unsigned int>> &archetype_skillsets) : clientId(clientId), username(username), type(type) {
		for (auto skill_id : archetype_skillsets[type]) {
			auto initialLevel = skill_map[skill_id].initialLevel;
			skillLevels.insert({ skill_id, initialLevel });
		}
		alive = true;
		gold = 0;
		currKillStreak = 0;
		currLoseStreak = 0;
		
	}
	PlayerMetadata() {}
	~PlayerMetadata() {}

	unsigned int clientId;
	std::string username;
	ArcheType type;
	unordered_map<unsigned int, unsigned int> skillLevels;

	bool alive;
	Point currLocation;
	// Omitting Point desiredFinalLocation

	// TODO: currently not plan to implement gold, purchase
	// TODO: how to implement weapons
	int gold;
	int currKillStreak;		// to give out gold
	int currLoseStreak;		// to give out gold
	unordered_set<string> inventory;	// items from shop
};

