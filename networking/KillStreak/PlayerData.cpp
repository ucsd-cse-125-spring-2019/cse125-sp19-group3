#include "PlayerData.hpp"

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