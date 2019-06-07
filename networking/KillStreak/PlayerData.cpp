 #include "PlayerData.hpp"
#include "logger.hpp"
#include "sysexits.h"
#include "nlohmann\json.hpp"

#include <fstream>
#include <string>
#include <cmath>

#define META_CONF "../../networking/KillStreak/meta_data.json"
#define KILL_POINTS 3		// points awarded per kill
#define FIRST  1
#define SECOND 2
#define THIRD  3
#define FOURTH 4
#define ROUND_3 3
#define ROUND_4 4
#define ROUND_5 5

// COPIED FROM SERVERSCENE, CHANGE THIS IF YOU CHANGE THAT
#define EVADE				0
#define PROJECTILE			1
#define PYROBLAST			2
#define DRAGONS_BREATH		3
#define ASSASSIN_PROJECTILE	11
#define INVISIBILITY		12
#define SPRINT			    13
#define VISIBILITY          14
#define WHIRLWIND			22
#define CHARGE				23
#define ROYAL_CROSS			32
#define SUBJUGATION			33

using json = nlohmann::json;
using namespace std;

// map string to archtype
unordered_map<string, ArcheType> archetype_map = {
	{"MAGE", MAGE}, // FOR NOW
	{"ASSASSIN", ASSASSIN},
	{"WARRIOR", WARRIOR},
	{"KING", KING},
};	

/*
	Return player id of winner of previous round.
*/
vector<ArcheType> LeaderBoard::getRoundWinner(unordered_map<ArcheType, int>* selected_characters)
{
	vector<ArcheType> winners;
	// get max kills from list
	auto it = std::max_element(currentKills.begin(), currentKills.end());		
	int max_kills = *it;				

	// any players with max_kills is a winner; add their ArcheType to vector of winners
	for (int client_id = 0; client_id < GAME_SIZE; client_id++)
	{
		// current client_id is a winner; get their Archetype
		if (currentKills[client_id] == max_kills)
		{
			// get ArcheType
			unordered_map<ArcheType, int>::iterator a_it = selected_characters->begin();
			while (a_it != selected_characters->end())
			{
				ArcheType curr_type = a_it->first;
				int curr_id = a_it->second;
				if (client_id == curr_id)
				{
					winners.push_back(curr_type);
					break;
				}

				a_it++;
			}
		}
	}
	return winners;
}


/*
	End of round... award points to all players based on rank.
*/
void LeaderBoard::awardRoundPoints(int round_number)
{
	vector<int> rankings(GAME_SIZE,0);		// indexed by player_id, holds ranking of last round 
	vector<int> currKills = currentKills;	// make copy of rounds kill vector

	// get max kills from list
	auto it = std::max_element(currKills.begin(), currKills.end());		
	int max_kills = *it;				

	// assign ranking to each player (insert into rankings vector)
	int current_rank = 1;
	int total_assigned = 0;
	while (total_assigned < GAME_SIZE)
	{
		for (int i = 0; i < GAME_SIZE; i++)
		{
			if (currentKills[i] == max_kills)	// this player has the max kills
			{
				rankings[i] = current_rank;
				total_assigned++;
			}
		}

		// set all current max kills to -1
		replace(currKills.begin(), currKills.end(), max_kills, -1);	

		// get next max	& ranking
		it = std::max_element(currKills.begin(), currKills.end());	
		max_kills = *it;
		current_rank++;	
	}


	// award points based on ranking
	for (int client_id = 0; client_id < GAME_SIZE; client_id++)
	{
		int points = 0;
		int rank = rankings[client_id];

		// award initial points
		switch (rank)
		{
			case FIRST : points = 6; break;
			case SECOND: points = 4; break;
			case THIRD : points = 2; break;
			case FOURTH: points = 1; break;
			default	   : points = 0; break;
		}

		// multiply round points based on rank  
		if (round_number == ROUND_3 || round_number == ROUND_4) points *= 1.5;
		else if (round_number == ROUND_5) points += 2;

		currPoints[client_id] = currPoints[client_id] + points; // add to points vector on leaderboard
	}
}


// reset players kill streak in vector sent to all clients
void LeaderBoard::resetKillStreak(unsigned int player_id) 
{
	killStreaks[player_id] = 0;
}


// increment players killstreak in vector sent to all clients
void LeaderBoard::incKillStreak(unsigned int player_id)
{
	int curr_ks = killStreaks[player_id];
	killStreaks[player_id] = ++curr_ks;
}


/*
	Award kill to player on rounds leaderboard by incrementing their kill score at index 'player_id'
	in the currentKills vector.
*/
void LeaderBoard::awardKillRound(unsigned int player_id)
{
	int kills = currentKills[player_id];
	currentKills[player_id] = ++kills;
}


// Award kill to player on global leaderboard
void LeaderBoard::awardKillGlobal(unsigned int player_id)
{
	int kills = globalKills[player_id];
	globalKills[player_id] = ++kills;
}

/*

	Award point to player by incrementing their point score at index 'player_id'
	in the currentPoints vector.
*/
void LeaderBoard::awardPoint(unsigned int player_id)
{
	int cur_score = currPoints[player_id];
	currPoints[player_id] = cur_score + KILL_POINTS;
}


// increment players death count
void LeaderBoard::incDeath(unsigned int player_id) 
{
	int currDeaths = currentDeaths[player_id];
	currentDeaths[player_id] = ++currDeaths;
}


/*
vector<int>* LeaderBoard::roundSummary() {
	// get the ranking result of this round
	vector<int> temp(currentKills);
	vector<int>* rankings = new vector<int>(GAME_SIZE);
	sort(temp.begin(), temp.end(), [](const int &i1, const int &i2) {return i1 > i2; });

	int rank = 1;
	int curr = temp[0] + 1;
	for (int score : temp) {
		if (score < curr) { rank++; curr = score; }
		else { continue; }

		for (int i = 0; i<GAME_SIZE; i++) {
			if (currentKills[i] == score) { (*rankings)[i] = rank; }
		}
	}

	// Update current points based on rankings
	for (int i = 0; i<GAME_SIZE; i++) currPoints[i] += prizes[(*rankings)[i]];
	// Update prizes
	for (int i = 0; i<GAME_SIZE; i++) prizes[i] *= prizeChange;
	// reset current kills
	currentKills.clear();

	return rankings;
}
*/


// Parse all archtypes from config and upload values to skill_map for each corresponding type.
void Skill::load_archtype_data(unordered_map<unsigned int, Skill> *skill_map,
	                           unordered_map<ArcheType, vector<unsigned int>> *archetype_skill_set) {

	// open config for reading
	ifstream json_file(META_CONF);
	json jsonObjs = json::parse(json_file);

	for (auto skill : jsonObjs["Skills"])		  	// parse each archtype
	{
		// get skills from config
		unsigned int skill_id       = skill["skill_id"];
		unsigned int initial_level  = skill["initial_level"];
		string skill_name			= skill["skill_name"];
		int cooldown              = skill["cooldown"]; // in milliseconds
		float range                 = skill["range"];
		float speed                 = skill["speed"];
		int duration              = skill["duration"]; // in milliseconds

		Skill curr_skill = Skill(skill_id, initial_level, skill_name, range, cooldown, duration, speed);
		skill_map->insert({ skill_id, curr_skill });
	}


	for (auto iter = jsonObjs["ArcheTypes"].begin(); iter != jsonObjs["ArcheTypes"].end(); iter++) {
		auto archetype_str = iter.key();
		auto skill_ids = iter.value();
		vector<unsigned int> available_skills;
		for (auto skill_id : skill_ids) {
			available_skills.push_back(skill_id);
		}
		archetype_skill_set->insert({ archetype_map[archetype_str], available_skills });
	}
}

Skill Skill::calculateSkillBasedOnLevel(Skill &baseSkill, unsigned int level) {
	/*auto range = baseSkill.range * pow(1.2, level);
	auto cooldown = (int)(baseSkill.cooldown * pow(0.8, level));
	auto duration = (int)(baseSkill.duration * pow(1.2, level));
	auto speed = baseSkill.speed * pow(1.2, level);*/

	Skill adjustedSkill = baseSkill;
	switch (baseSkill.skill_id) {
	
	// duration-based skills (evade, sprint, invisibility, silence)
	case EVADE: 
	{
		// only change cooldown for evade
		auto newCooldown = baseSkill.cooldown - 2000 * (level - 1);
		adjustedSkill.cooldown = newCooldown;
		break;
	}
	case SPRINT:
	{
		// only change duration for sprint, max level duration == cooldown
		auto newDuration = baseSkill.duration + 5000 * (level - 1);
		adjustedSkill.duration = newDuration;
		break;
	}
	case INVISIBILITY:
	{
		auto newDuration = baseSkill.duration + 1000 * (level - 1);
		adjustedSkill.duration = newDuration;
		break;
	}
	case SUBJUGATION:
	{
		auto newDuration = baseSkill.duration + 1000 * (level - 1);
		adjustedSkill.duration = newDuration;
		break;
	}
	// aoe skills (pyroblast, whirlwind, royal cross)
	case PYROBLAST:
	{
		auto newRange = baseSkill.range + 5 * (level - 1);
		auto newCooldown = baseSkill.cooldown - 1000 * (level - 1);
		adjustedSkill.range = newRange;
		adjustedSkill.cooldown = newCooldown;
		break;
	}
	case WHIRLWIND:
	{
		auto newCooldown = baseSkill.cooldown - 1500 * (level - 1);
		adjustedSkill.cooldown = newCooldown;
		break;
	}
	case ROYAL_CROSS:
	{
		auto newCooldown = baseSkill.cooldown - 1500 * (level - 1);
		adjustedSkill.cooldown = newCooldown;
		break;
	}
	// point skills (meteor, charge, projectile, assassin projectile)
	case DRAGONS_BREATH:
	{
		auto newCooldown = baseSkill.cooldown - 1000 * (level - 1);
		adjustedSkill.cooldown = newCooldown;
		auto newSpeed = baseSkill.speed + 1.5 * (level - 1);
		adjustedSkill.speed = newSpeed;
		break;
	}
	case CHARGE:
	{
		auto newCooldown = baseSkill.cooldown - 2000 * (level - 1);
		auto newRange = baseSkill.range + 3 * (level - 1);
		adjustedSkill.cooldown = newCooldown;
		adjustedSkill.range = newRange;
		break;
	}
	case PROJECTILE:
	{
		auto newCooldown = baseSkill.cooldown - 2000 * (level - 1);
		auto newRange = baseSkill.range + 4 * (level - 1);
		auto newSpeed = baseSkill.speed + 0.1 * (level - 1);
		adjustedSkill.cooldown = newCooldown;
		adjustedSkill.range = newRange;
		adjustedSkill.speed = newSpeed;
		break;
	}
	case ASSASSIN_PROJECTILE:
	{
		auto newCooldown = baseSkill.cooldown - 1000 * (level - 1);
		auto newSpeed = baseSkill.speed + 0.1 * (level - 1);
		adjustedSkill.cooldown = newCooldown;
		adjustedSkill.speed = newSpeed;
		break;
	}

	default:
		break;
	}

	return adjustedSkill;
}






/*
	Mainly for testing purposes, print each value in current kills vector
	of leaderboard.
*/
void LeaderBoard::printCurrentKills()
{
	int cur_player = 0;
	vector<int>::iterator it = currentKills.begin();
	while (it != currentKills.end())
	{
		logger()->debug("Player {}: {} kills", cur_player, *it);
		it++; cur_player++;
	}
}

/*
	Mainly for testing purposes, print each value in current kills vector
	of leaderboard.
*/
void LeaderBoard::printCurrPoints()
{
	int cur_player = 0;
	vector<int>::iterator it = currPoints.begin();
	while (it != currPoints.end())
	{
		logger()->debug("Player {}: {} points", cur_player, *it);
		it++; cur_player++;
	}
}


/*
	Mainly for testing purposes, print each value in killstreaks vector
	of leaderboard.
*/
void LeaderBoard::printCurrentKillStreaks()
{
	int cur_player = 0;
	vector<int>::iterator it = killStreaks.begin();
	while (it != killStreaks.end())
	{
		logger()->debug("Player {}: {} Killstreak", cur_player, *it);
		it++; cur_player++;
	}
}


/*
	Mainly for testing purposes, print each value in deathcount vector
	of leaderboard.
*/
void LeaderBoard::printDeathCount()
{
	int cur_player = 0;
	vector<int>::iterator it = currentDeaths.begin();
	while (it != currentDeaths.end())
	{
		logger()->debug("Player {}: {} deaths", cur_player, *it);
		it++; cur_player++;
	}
}

/*
	Mainly for testing purposes, print each value in current kills vector
	of leaderboard.
*/
void LeaderBoard::printPrizes()
{
	int cur_player = 0;
	vector<int>::iterator it = prizes.begin();
	while (it != prizes.end())
	{
		logger()->debug("Player {}: {} prizes", cur_player, *it);
		it++; cur_player++;
	}
}




