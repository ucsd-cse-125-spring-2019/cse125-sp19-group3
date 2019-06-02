#pragma once
#include <SFML/Audio.hpp>
#include <glm/glm.hpp>

#include <cstdio>
#include <iostream>

enum AUDIO_TYPE { FIRE_AUDIO };

class Audio {
public:
	Audio();
	~Audio();

	void play(glm::vec3 pos, unsigned int audio_id);

	void initListener(glm::vec3 pos);

private:
	sf::SoundBuffer fireBuffer;
	sf::Sound fireSound;

	sf::Music music;

	void loadWAVFiles();
};