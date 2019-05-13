#include "PlayerData.hpp"
#include "logger.hpp"
#include "sysexits.h"
#include "nlohmann\json.hpp"

#include <fstream>
#include <string>

#define META_CONF "../../networking/KillStreak/meta_data.json"

using json = nlohmann::json;
using namespace std;

// map string to archtype
unordered_map<string, ArcheType> archetype_map = {
	{"MAGE", MAGE},
	{"ASSASSIN", ASSASSIN},
	{"WARRIOR", WARRIOR}
};

// map string to skilltype
unordered_map<string, SkillType> skilltype_map = { 
	{"MELEE", MELEE},
	{"PROJECTILE", PROJECTILE},
	{"AOE", AOE},
	{"MINIMAP", MINIMAP },
	{"INVISIBLE", INVISIBLE},
	{"CHARGE", CHARGE}
};		

vector<int>* LeaderBoard::roundSummary() {
	// get the ranking result of this round
	vector<int> temp(currentKills);
	vector<int>* rankings = new vector<int>(PLAYERNUM);
	sort(temp.begin(), temp.end(), [](const int &i1, const int &i2) {return i1 > i2; });

	int rank = 1;
	int curr = temp[0] + 1;
	for (int score : temp) {
		if (score < curr) { rank++; curr = score; }
		else { continue; }

		for (int i = 0; i<PLAYERNUM; i++) {
			if (currentKills[i] == score) { (*rankings)[i] = rank; }
		}
	}

	// Update current points based on rankings
	for (int i = 0; i<PLAYERNUM; i++) currPoints[i] += prizes[(*rankings)[i]];
	// Update prizes
	for (int i = 0; i<PLAYERNUM; i++) prizes[i] *= prizeChange;
	// reset current kills
	currentKills.clear();

	return rankings;
}

void Skill::update(SkillType skillType, double cooldown, double range, double speed, double duration) {
	this->skillType = skillType;
	this->cooldown = cooldown;
	this->range = range;
	this->speed = speed;
	this->duration = duration;
}

double getDoubleField(INIReader & meta_data, string archetype, string field) {
	auto log = logger();
	double temp = (double)(meta_data.GetInteger(archetype, field, -1));
	if (temp == -1) {
		log->error("No {} for {}", field, archetype);
		exit(EX_CONFIG);
	}
	return temp;
}


// Parse all archtypes from config and upload values to skill_map for each corresponding type.
void Skill::load_archtype_data(unordered_map<ArcheType, vector<Skill>> &skill_map) {

	// open config for reading
	ifstream json_file(META_CONF);
	json jsonObjs = json::parse(json_file);

	for (auto cur_char : jsonObjs["ArcheTypes"])		  	// parse each archtype
	{
		string current_archetype = cur_char["ArcheType"];	 // mage, assassin, warrior? 

		for (auto skill : cur_char["Skills"])			// each skill set (MELEE, PROJECTILE, ETC)
		{

			Skill cur_skill		= Skill();

			// get skills from config
			string sk_t			= skill["skill_type"];
			string str_cooldown = skill["cooldown"];
			string str_range    = skill["range"];
			string str_speed	= skill["speed"];
			string str_duration = skill["duration"];

			// convert strings to doubles
			double cooldown = atof(str_cooldown.c_str());
			double range = atof(str_range.c_str());
			double speed = atof(str_speed.c_str());
			double duration = atof(str_duration.c_str());

			// get skilltype and archetype
			SkillType skill_type = skilltype_map[sk_t];
			ArcheType arche_type = archetype_map[current_archetype];

			// update skill with values and push to corresponding type skill map
			cur_skill.update(skill_type, cooldown, range, speed, duration);
			skill_map[arche_type].push_back(cur_skill);
		}

	}

}
