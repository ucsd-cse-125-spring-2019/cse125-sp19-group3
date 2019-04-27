#include "CoreTypes.hpp"
#include <unordered_set>
#include <string>
#include <algorithm>    // std::sort
#include <vector>

// #define PLAYERNUM 4;

using namespace std;

static int PLAYERNUM = 4;

class LeaderBoard {
public:
	LeaderBoard(vector<int> initial_prize, int change) : prizeChange(change) { prizes = initial_prize;}
	LeaderBoard() : currentKills(vector<int>(PLAYERNUM, 0)), currPoints(vector<int>(PLAYERNUM, 0)) {}
	~LeaderBoard() {}

	vector<int> roundSummary (); // Update vectors for the round and return the ranking of each player
	vector<int> getCurrPoints() {return currPoints;}

protected:
	vector<int> currentKills;	// # of kills in each round of each player
	vector<int> currPoints;		// accumulative points of each player
	vector<int> prizes;							// points added to player each round based on ranking
	float prizeChange;							// prizes increases per round (1.2)
};

typedef enum{AOE, MINIMAP, INVISIBLE, CHARGE, DEFAULT_SKILLTYPE} SkillType;
class Skill {
protected:
	SkillType skillType;
	int range;
	int cooldown;
	int duration;
	int speed;
public:
	Skill() : skillType(DEFAULT_SKILLTYPE), range(-1), cooldown(-1), duration(-1), speed(-1) {}
	~Skill(){}
	void update(SkillType skillType, int range=0, int cooldown=0, int duration=0, int speed=0);
};

typedef enum{MAGE, ASSASSIN, WARRIOR} ArcheType;
class Arche {
public:
	Arche(){}
	~Arche(){}
	void addSkill(Skill s) { skills.push_back(s); }
	virtual void useSkill(int skillIndex, Point finalLocation);
protected:
	vector<Skill> skills;
};

class Mage: Arche {
	Mage() {}
	~Mage() {}
	void useSkill(int skillIndex, Point finalLocation);
};

class Assassin: Arche {
	Assassin() {}
	~Assassin() {}
	void useSkill(int skillIndex, Point finalLocation);
};

class Warrior: Arche {
	Warrior() {}
	~Warrior() {}
	void useSkill(int skillIndex, Point finalLocation);
};

class Player {
public:
	Player() {}
	~Player() {}

protected:
	int clientId;

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
	unordered_set<string> equipped;		// weapons, default to a melee and a proj
	int meleeLv;
	int projLv;

};

vector<int> LeaderBoard::roundSummary() {
	// get the ranking result of this round
	vector<int> temp(currentKills);
	vector<int> rankings(PLAYERNUM);
	sort(temp.begin(), temp.end(), [](const int &i1, const int &i2) {return i1 > i2; });

	int rank = 1;
	int curr = temp[0]+1;
	for (int score : temp) {
		if (score < curr) { rank++; curr = score;}
		else { continue; }

		for (int i=0; i<PLAYERNUM; i++) {
			if (currentKills[i] == score) { rankings[i] = rank; }
		}
	}

	// Update current points based on rankings
	for (int i=0; i<PLAYERNUM; i++) currPoints[i] += prizes[rankings[i]];
	// Update prizes
	for (int i=0; i<PLAYERNUM; i++) prizes[i] *= prizeChange;
	// reset current kills
	currentKills.clear();

	return rankings;
}

void Skill::update(SkillType skillType, int range=0, int cooldown=0, int duration=0, int speed=0) {
	this->skillType = skillType;
	this->cooldown = cooldown;
	switch (skillType) {
	case AOE:
		this->range = range;
		this->duration = duration;
		break;
	case MINIMAP:
		this->duration = duration;
		break;
	case INVISIBLE:
		this->duration = duration;
		this->speed = speed;
		break;
	case CHARGE:
		this->range = range;
		this->speed = speed;
		break;
	}
}