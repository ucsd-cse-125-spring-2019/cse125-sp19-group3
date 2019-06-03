#pragma once
#include <SFML/Audio.hpp>
#include <glm/glm.hpp>

#include <cstdio>
#include <iostream>

enum AUDIO_TYPE { ASSASSIN_PROJECTILE_AUDIO, ASSASSIN_STEALTH_AUDIO, ASSASSIN_TELEPORT_AUDIO, 
	KING_SILENCE_AUDIO, FIRE_AOE_AUDIO, FIRE_CONE_AOE_AUDIO, FIRE_BALL_1_AUDIO, FIREBALL_2_AUDIO, FIRE_BALL_3_AUDIO,
	WARRIOR_CHARGE_AUDIO, WARRIOR_PROJECTILE_AUDIO, WARRIOR_SLAM_AOE_AUDIO, SKELETON_DEATH_1_AUDIO, SKELETON_DEATH_2_AUDIO, 
	SKELETON_EVADE_AUDIO, SKELETON_EVADE_2_AUDIO, BUY_ITEM_1_AUDIO, BUY_ITEM_2_AUDIO, EQUIP_ITEM_1_AUDIO, EQUIP_ITEM_2_AUDIO};

class Audio {
public:
	Audio();
	~Audio();

	void play(glm::vec3 pos, unsigned int audio_id);

	void initListener(glm::vec3 pos);

private:
	//**ASSASSIN SKILLS**//
	sf::SoundBuffer assassin_projectile_Buffer;
	sf::SoundBuffer assassin_stealth_Buffer;
	sf::SoundBuffer assassin_teleport_Buffer;
	sf::Sound assassin_projectile;
	sf::Sound assassin_stealth;
	sf::Sound assassin_teleport;

	//**KING SKILLS**//
	sf::SoundBuffer king_silence_Buffer;
	sf::Sound king_silence;

	//**MAGE SKILLS**//
	sf::SoundBuffer fire_aoe_Buffer;
	sf::SoundBuffer fire_cone_aoe_Buffer;
	sf::SoundBuffer fireball_1_Buffer;
	sf::SoundBuffer fireball_2_Buffer;
	sf::SoundBuffer fireball_3_Buffer;
	sf::Sound fire_aoe;
	sf::Sound fire_cone_aoe;
	sf::Sound fireball_1;
	sf::Sound fireball_2;
	sf::Sound fireball_3;

	//**WARRIOR SKILLS**//
	sf::SoundBuffer warrior_charge_Buffer;
	sf::SoundBuffer warrior_projectile_Buffer;
	sf::SoundBuffer warrior_slam_aoe_Buffer;
	sf::Sound warrior_charge;
	sf::Sound warrior_projectile;
	sf::Sound warrior_slam_aoe;

	//**GENERAL MODEL SOUNDS**//
	sf::SoundBuffer skeleton_death_1_Buffer;
	sf::SoundBuffer skeleton_death_2_Buffer;
	sf::SoundBuffer skeleton_evade_Buffer;
	sf::SoundBuffer skeleton_evade_2_Buffer;
	sf::Sound skeleton_death_1;
	sf::Sound skeleton_death_2;
	sf::Sound skeleton_evade;
	sf::Sound skeleton_evade_2;

	//**STORE SOUNDS**//
	sf::SoundBuffer buyItem_1_Buffer;
	sf::SoundBuffer buyItem_2_Buffer;
	sf::SoundBuffer equipItem_1_Buffer;
	sf::SoundBuffer equipItem_2_Buffer;
	sf::Sound buyItem_1;
	sf::Sound buyItem_2;
	sf::Sound equipItem_1;
	sf::Sound equipItem_2;

	sf::Music music;
	void loadWAVFiles();
};