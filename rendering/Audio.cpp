#include "Audio.h"

using namespace std;

Audio::Audio() {
	loadWAVFiles();
	assassin_projectile.setBuffer(assassin_projectile_Buffer);
	assassin_stealth.setBuffer(assassin_stealth_Buffer);
	assassin_teleport.setBuffer(assassin_teleport_Buffer);
	king_silence.setBuffer(king_silence_Buffer);
	fire_aoe.setBuffer(fire_aoe_Buffer);
	fire_cone_aoe.setBuffer(fire_cone_aoe_Buffer);
	fireball_1.setBuffer(fireball_1_Buffer);
	fireball_2.setBuffer(fireball_2_Buffer);
	fireball_3.setBuffer(fireball_3_Buffer);
	warrior_charge.setBuffer(warrior_charge_Buffer);
	warrior_projectile.setBuffer(warrior_projectile_Buffer);
	warrior_slam_aoe.setBuffer(warrior_slam_aoe_Buffer);
	skeleton_death_1.setBuffer(skeleton_death_1_Buffer);
	skeleton_death_2.setBuffer(skeleton_death_2_Buffer);
	skeleton_evade.setBuffer(skeleton_evade_Buffer);
	skeleton_evade_2.setBuffer(skeleton_evade_2_Buffer);
	cooldown_sound.setBuffer(cooldown_Buffer);
	buyItem_1.setBuffer(buyItem_1_Buffer);
	buyItem_2.setBuffer(buyItem_2_Buffer);
	equipItem_1.setBuffer(equipItem_1_Buffer);
	equipItem_2.setBuffer(equipItem_2_Buffer);
	button_press.setBuffer(button_press_Buffer);
	prepare_phase_music.setBuffer(prepare_phase_music_Buffer);
	kill_phase_music.setBuffer(kill_phase_music_Buffer);
	timer.setBuffer(timer_Buffer);
	//**BGM**
	//if (!music.openFromFile("./audio/music.wav"))
	//	cout << "unable to load wav file " << "music.wav" << endl;
	//music.setLoop(true);
	//music.play();
}

Audio::~Audio() {
}

void Audio::loadWAVFiles() {
	if (!assassin_projectile_Buffer.loadFromFile("../audio/assassin/assassin_projectile.wav"))
		cout << "unable to load wav file " << "assassin_projectile.wav" << endl;
	if (!assassin_stealth_Buffer.loadFromFile("../audio/assassin/assassin_stealth.wav"))
		cout << "unable to load wav file " << "assassin_stealth.wav" << endl;
	if (!assassin_teleport_Buffer.loadFromFile("../audio/assassin/assassin_teleport.wav"))
		cout << "unable to load wav file " << "assassin_teleport.wav" << endl;
	if (!king_silence_Buffer.loadFromFile("../audio/king/king_silence.wav"))
		cout << "unable to load wav file " << "king_silence.wav" << endl;
	if (!fire_aoe_Buffer.loadFromFile("../audio/mage/fire_aoe.wav"))
		cout << "unable to load wav file " << "fire_aoe.wav" << endl;
	if (!fire_cone_aoe_Buffer.loadFromFile("../audio/mage/fire_cone_aoe.wav"))
		cout << "unable to load wav file " << "fire_cone_aoe.wav" << endl;
	if (!fireball_1_Buffer.loadFromFile("../audio/mage/fireball_1.wav"))
		cout << "unable to load wav file " << "fireball_1.wav" << endl;
	if (!fireball_2_Buffer.loadFromFile("../audio/mage/fireball_2.wav"))
		cout << "unable to load wav file " << "fireball_2.wav" << endl;
	if (!fireball_3_Buffer.loadFromFile("../audio/mage/fireball_3.wav"))
		cout << "unable to load wav file " << "fireball_3.wav" << endl;
	if (!warrior_charge_Buffer.loadFromFile("../audio/warrior/warrior_charge.wav"))
		cout << "unable to load wav file " << "warrior_charge.wav" << endl;
	if (!warrior_projectile_Buffer.loadFromFile("../audio/warrior/warrior_projectile.wav"))
		cout << "unable to load wav file " << "warrior_projectile.wav" << endl;
	if (!warrior_slam_aoe_Buffer.loadFromFile("../audio/warrior/warrior_slam_aoe.wav"))
		cout << "unable to load wav file " << "warrior_slam_aoe.wav" << endl;
	if (!skeleton_death_1_Buffer.loadFromFile("../audio/model/skeleton_death1.wav"))
		cout << "unable to load wav file " << "skeleton_death1.wav" << endl;
	// Need to fix this soudn
	//if (!skeleton_death_2_Buffer.loadFromFile("../audio/model/skeleton_death2.wav"))
	//	cout << "unable to load wav file " << "skeleton_death2.wav" << endl;
	if (!skeleton_evade_Buffer.loadFromFile("../audio/model/skeleton_evade1.wav"))
		cout << "unable to load wav file " << "skeleton_evade1.wav" << endl;
	if (!skeleton_evade_2_Buffer.loadFromFile("../audio/model/skeleton_evade2.wav"))
		cout << "unable to load wav file " << "skeleton_evade2.wav" << endl;
	if (!buyItem_1_Buffer.loadFromFile("../audio/store/buyItem_1.wav"))
		cout << "unable to load wav file " << "buyItem_1.wav" << endl;
	if (!buyItem_2_Buffer.loadFromFile("../audio/store/buyItem_2.wav"))
		cout << "unable to load wav file " << "buyItem_2.wav" << endl;
	if (!equipItem_1_Buffer.loadFromFile("../audio/store/equipItem_1.wav"))
		cout << "unable to load wav file " << "equipItem_1.wav" << endl;
	if (!equipItem_2_Buffer.loadFromFile("../audio/store/equipItem_2.wav"))
		cout << "unable to load wav file " << "equipItem_2.wav" << endl;
	if (!equipItem_2_Buffer.loadFromFile("../audio/store/equipItem_2.wav"))
		cout << "unable to load wav file " << "equipItem_2.wav" << endl;
	if (!cooldown_Buffer.loadFromFile("../audio/model/cooldown_reset.wav"))
		cout << "unable to load wav file " << "cooldown_reset.wav" << endl;
	if (!button_press_Buffer.loadFromFile("../audio/store/button_click.wav"))
		cout << "unable to load wav file " << "timer_tick.wav" << endl;
	if (!prepare_phase_music_Buffer.loadFromFile("../audio/phases/prepare_phase.wav"))
		cout << "unable to load wav file " << "prepare_phase.wav" << endl;
	if (!kill_phase_music_Buffer.loadFromFile("../audio/phases/kill_phase.wav"))
		cout << "unable to load wav file " << "kill_phase.wav" << endl;
	if (!timer_Buffer.loadFromFile("../audio/phases/timer_tick.wav"))
		cout << "unable to load wav file " << "timer_tick.wav" << endl;
	

}

void Audio::play(glm::vec3 pos, unsigned int audio_id) {
	switch (audio_id) {
	case ASSASSIN_PROJECTILE_AUDIO:
		assassin_projectile.play();
		break;
	case ASSASSIN_STEALTH_AUDIO: 
		assassin_stealth.play();
		break;
	case ASSASSIN_TELEPORT_AUDIO:
		assassin_teleport.play();
		break;
	case KING_SILENCE_AUDIO:
		king_silence.play();
		break;
	case FIRE_AOE_AUDIO:
		fire_aoe.play();
		break;
	case FIRE_CONE_AOE_AUDIO:
		fire_cone_aoe.play();
		break;
	case FIRE_BALL_1_AUDIO:
		fireball_1.play();
		break;
	case FIREBALL_2_AUDIO:
		fireball_2.play();
		break;
	case FIRE_BALL_3_AUDIO:
		fireball_3.play();
		break;
	case WARRIOR_CHARGE_AUDIO:
		warrior_charge.play();
		break;
	case WARRIOR_PROJECTILE_AUDIO:
		warrior_projectile.play();
		break;
	case WARRIOR_SLAM_AOE_AUDIO:
		warrior_slam_aoe.play();
		break;
	case SKELETON_DEATH_1_AUDIO:
		skeleton_death_1.play();
		break;
	case SKELETON_DEATH_2_AUDIO:
		skeleton_death_2.play();
		break;
	case SKELETON_EVADE_AUDIO:
		skeleton_evade.play();
		break;
	case SKELETON_EVADE_2_AUDIO:
		skeleton_evade_2.play();
		break;
	case BUY_ITEM_1_AUDIO:
		buyItem_1.play();
		break;
	case BUY_ITEM_2_AUDIO:
		buyItem_2.play();
		break;
	case EQUIP_ITEM_1_AUDIO:
		equipItem_1.play();
		break;
	case EQUIP_ITEM_2_AUDIO:
		equipItem_2.play();
		break;
	case PREPARE_PHASE_MUSIC:
		prepare_phase_music.play();
		break;
	case KILL_PHASE_MUSIC:
		kill_phase_music.play();
		break;
	case COOLDOWN_RESET_AUDIO:
		cooldown_sound.play();
		break;
	case TIMER_AUDIO:
		timer.play();
		break;
	case GAME_OVER_AUDIO:
		game_over.play();
		break;
	case BUTTON_PRESS_AUDIO:
		button_press.play();
		break;
	}
	
}

void Audio::initListener(glm::vec3 pos) {
	sf::Listener::setPosition(pos.x, pos.y, pos.z);
	sf::Listener::setDirection(-pos.x, -pos.y, -pos.z);
}