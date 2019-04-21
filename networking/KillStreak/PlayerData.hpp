#include "CoreTypes.hpp";
#include <unordered_set>;
#include <string>;

class LeaderBoard {
	vector<int> currentKills;	// # of kills in each round of each player
	vector<int> currPoints;		// accumulative points of each player
};

class Skill {

};

class Arche {
protected:
	vector<Skill> skills;

};

class Player {
protected:
	int clientId;

	Arche arche;

	bool alive;
	Point currLocation;
	// Omitting Point desiredFinalLocation


	// TODO: currently not plan to implement gold, purchase, weapons
	int gold;
	int currKillStreak;		// to give out gold
	int currLoseStreak;		// to give out gold
	unordered_set<string> inventory;	// items from shop
	unordered_set<string> equipped;		// weapons, default to a melee and a proj
	int meleeLv;
	int projLv;

};