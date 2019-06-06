#pragma once
#include "CoreTypes.hpp"
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <algorithm>    // std::sort
#include <vector>
#include <list>
#include "INIReader.h"

using namespace std;

class LeaderBoard {
public:

	// default constructor
	LeaderBoard() : currentKills(vector<int>(GAME_SIZE, 0)), currentDeaths(vector<int>(GAME_SIZE,0)),
					killStreaks(vector<int>(GAME_SIZE, 0)), globalKills(vector<int>(GAME_SIZE,0)),
					currPoints(vector<int>(GAME_SIZE, 0)), currGold(vector<int>(GAME_SIZE,0)),
					prizes(vector<int>(GAME_SIZE, 0)), deaths_this_tick(0) {}

	~LeaderBoard() {}

	void awardKillRound(unsigned int player_id);		// award point to player_id on rounds leaderboard
	void awardKillGlobal(unsigned int player_id);		// award point to player_id on global leaderboard
	void resetKillStreak(unsigned int player_id);		// reset players kill streak
	void incKillStreak(unsigned int player_id);			// increment players killstreak
	void incDeath(unsigned int player_id);				// increment players death count
	void awardPoint(unsigned int player_id);			// award point to player_id
	void awardRoundPoints(int round_number);			// award points to all players based on rank

	// return winner (player id) of last round
//	vector<ArcheType> getRoundWinner(unordered_map<unsigned int, PlayerMetadata*>* playerMetadatas);


	// for testing
	void printCurrentKillStreaks();	
	void printCurrentKills();	
	void printCurrPoints();	
	void printDeathCount();	
	void printPrizes();	


	// RESET THESE VECTORS THREE VECTORS AFTER EVERY ROUND
	vector<int> currentKills;	// current rounds kills for each player
	vector<int> currentDeaths;  // current rounds deaths for each player
	vector<int> killStreaks;	// current rounds killstreaks for each player

	vector<int> globalKills;	// total # of kills in all rounds
	vector<int> currPoints;		// accumulative points of each player
	vector<int> currGold;		// accumulative gold of each player

	vector<int> prizes;			// points added to player each round based on ranking
	list<int> kill_map;			// map of who killed who on every server tick
	int deaths_this_tick;		// total deaths that occured this tick
};

class Skill {
public:
	string skillName;
	unsigned int level;
	unsigned int skill_id;
	float range;
	int cooldown; // in milliseconds
	int duration; // in milliseconds
	float speed;
	Skill(unsigned int skill_id, unsigned int initialLevel, string skillName, float range, int cooldown, int duration, float speed) {
		this->skillName = skillName;
		this->level = initialLevel;
		this->skill_id = skill_id;
		this->range = range;
		this->cooldown = cooldown;
		this->duration = duration;
		this->speed = speed;
	};
	Skill() : skillName("Default Skill"), range(-1), cooldown(-1), duration(-1), speed(-1) {}
	~Skill(){}
	static void load_archtype_data(unordered_map<unsigned int, Skill> *skill_map, 
		                           unordered_map<ArcheType, vector<unsigned int>> *archetype_skill_set);
	static Skill calculateSkillBasedOnLevel(Skill &baseSkill, unsigned int level);
};

