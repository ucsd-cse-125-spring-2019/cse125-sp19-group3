 #include "PlayerData.hpp"
#include "logger.hpp"
#include "sysexits.h"
#include "nlohmann\json.hpp"

#include <fstream>
#include <string>
#include <cmath>

#define META_CONF "../../networking/KillStreak/meta_data.json"

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
	Award point to player by incrementing their score at index 'player_id'
	in the currpoints vector.
*/
void LeaderBoard::awardPoint(unsigned int player_id)
{

}

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
		float cooldown              = skill["cooldown"];
		float range                 = skill["range"];
		float speed                 = skill["speed"];
		float duration              = skill["duration"];

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
	auto range = baseSkill.range * pow(1.2, level);
	auto cooldown = baseSkill.cooldown * pow(0.7, level);
	auto duration = baseSkill.duration;
	auto speed = baseSkill.speed * pow(1.1, level);
	return Skill(baseSkill.skill_id,
		level,
		baseSkill.skillName,
		range,
		cooldown,
		duration, // worry about this for animation vs invisibility + evade
		speed);
}
