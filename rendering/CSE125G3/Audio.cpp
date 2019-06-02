#include "Audio.h"

using namespace std;

Audio::Audio() {
	loadWAVFiles();
	fireSound.setBuffer(fireBuffer);

	//**BGM**
	//if (!music.openFromFile("./audio/music.wav"))
	//	cout << "unable to load wav file " << "music.wav" << endl;
	//music.setLoop(true);
	//music.play();
}

Audio::~Audio() {
}

void Audio::loadWAVFiles() {
	if (!fireBuffer.loadFromFile("../../rendering/audio/fire_aoe.mp3"))
		cout << "unable to load wav file " << "fire.wav" << endl;
}

void Audio::play(glm::vec3 pos, unsigned int audio_id) {
	switch (audio_id) {
	case FIRE_AUDIO:
		fireSound.play();
		break;
	}
}

void Audio::initListener(glm::vec3 pos) {
	sf::Listener::setPosition(pos.x, pos.y, pos.z);
	sf::Listener::setDirection(-pos.x, -pos.y, -pos.z);
}