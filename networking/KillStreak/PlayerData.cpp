#include "PlayerData.hpp"
#include "logger.hpp"
#include "sysexits.h"

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

void Skill::update(SkillType skillType, int range, int cooldown, int duration, int speed) {
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

double getDoubleField(INIReader & meta_data, string archetype, string field) {
	auto log = logger();
	double temp = (double)(meta_data.GetInteger(archetype, field, -1));
	if (temp == -1) {
		log->error("No {} for {}", field, archetype);
		exit(EX_CONFIG);
	}
	return temp;
}

Skill getMelee(INIReader & meta_data, string archetype) {
	
	Skill temp = Skill();
	temp.update(SkillType::MELEE, getDoubleField(meta_data, archetype, "range"));
	return temp;
}

Skill getProjectile(INIReader & meta_data, string archetype) {
	Skill temp = Skill();
	double cooldown = getDoubleField(meta_data, archetype, "cooldown");
	double speed = getDoubleField(meta_data, archetype, "speed");
	temp.update(SkillType::PROJECTILE, 0, cooldown, 0, speed);
	return temp;
}

Skill getAoe(INIReader & meta_data, string archetype) {
	Skill temp = Skill();
	double range = getDoubleField(meta_data, archetype, "range");
	double duration = getDoubleField(meta_data, archetype, "duration");
	temp.update(SkillType::AOE, range, 0, duration, 0);
	return temp;
}

Skill getMinimap(INIReader & meta_data, string archetype) {
	Skill temp = Skill();
	double duration = getDoubleField(meta_data, archetype, "duration");
	temp.update(SkillType::MINIMAP, 0, 0, duration, 0);
	return temp;
}

Skill getInvisible(INIReader & meta_data, string archetype) {
	Skill temp = Skill();
	double speed = getDoubleField(meta_data, archetype, "speed");
	double duration = getDoubleField(meta_data, archetype, "duration");
	temp.update(SkillType::INVISIBLE, 0, 0, duration, speed);
	return temp;
}

Skill getCharge(INIReader & meta_data, string archetype) {
	Skill temp = Skill();
	double speed = getDoubleField(meta_data, archetype, "speed");
	double range = getDoubleField(meta_data, archetype, "range");
	temp.update(SkillType::CHARGE, range, 0, 0, speed);
	return temp;
}