#include "CoreTypes.hpp"
#include <unordered_set>
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
protected:
	SkillType skillType;
	double range;
	double cooldown;
	double duration;
	double speed;
public:
	Skill() : skillType(DEFAULT_SKILLTYPE), range(-1), cooldown(-1), duration(-1), speed(-1) {}
	~Skill(){}
	void update(SkillType skillType, int range=0, int cooldown=0, int duration=0, int speed=0);
	static Skill getMelee(INIReader & meta_data, string archeType);
	static Skill getProjectile(INIReader & meta_data, string archeType);
	static Skill getAoe(INIReader & meta_data, string archetype);
	static Skill getMinimap(INIReader & meta_data, string archetype);
	static Skill getInvisible(INIReader & meta_data, string archetype);
	static Skill getCharge(INIReader & meta_data, string archetype);
};

class Arche {
public:
	Arche(){}
	~Arche(){}
	void addSkill(Skill s) { skills.push_back(s); }
	// virtual void useSkill(int skillIndex, Point finalLocation) {}
protected:
	vector<Skill> skills;
};

class Mage: public Arche {
public: 
	Mage() {}
	~Mage() {}
	// void useSkill(int skillIndex, Point finalLocation) override;
	void melee();
	void projectile(Point finalLocation);
	void aoe();
	void coneAoe(Point finalLocation);
};

class Assassin: public Arche {
public:
	Assassin() {}
	~Assassin() {}
	// void useSkill(int skillIndex, Point finalLocation) override;
	void melee();
	void projectile(Point finalLocation);
	void aoe();
	void minimap();
	void invisibility();
};

class Warrior: public Arche {
public: 
	Warrior() {}
	~Warrior() {}
	// void useSkill(int skillIndex, Point finalLocation) override;
	void melee();
	void projectile(Point finalLocation);
	void aoe();
	void coneAoe(Point finalLocation);
};

class PlayerMetadata {
public:
	PlayerMetadata(unsigned int clientId, std::string username, ArcheType type) : clientId(clientId), username(username), type(type) {
		switch (type) {
		case MAGE:
			arche = Mage();
			break;
		case ASSASSIN:
			arche = Assassin();
			break;
		case WARRIOR:
			arche = Warrior();
			break;
		default:
			arche = Mage();
			break;
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
	Arche arche;

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

