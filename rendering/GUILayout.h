#pragma once
#include "../../NuklearInit.h"
#define EVADE_INDEX 0
#define PROJ_INDEX 1
#define OMNI_SKILL_INDEX 2
#define DIR_SKILL_INDEX 3

#define KILLED_TEXT_NUM 4
const char * intToCharArray(int i) {
	string s = std::to_string(i);
	const int n = s.length();
	return s.c_str();
}

// order leaderboard by kills, most first
static void ui_leaderboard(struct nk_context *ctx, struct media *media,
	LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes ) {

	// order kills, usernames, and archetypes by client with most kills first...
	vector<int> kills;							
	vector<string> ordered_usernames;		
	vector<ArcheType> ordered_types;	
	vector<int> curKills = leaderBoard->currentKills;

	// make parallel arrays 'kills' & 'ordered_usernames' having same index for players based on number of kills
	for ( int i = 0; i < GAME_SIZE; i++)
	{
		// find max element in list; get total kills for that player
		auto it = std::max_element(curKills.begin(), curKills.end());
		int index = it - curKills.begin();
		int numKills = *it;

		// add next client with most kills username, kills & archetype
		ordered_usernames.push_back(usernames[index]);
		ordered_types.push_back(archetypes[index]);
		kills.push_back(numKills);

		*it = -1;		// reset current max to -1
	}

	if (nk_begin(ctx, "Leaderboard", nk_rect(10, 10, 300, 300),
		NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{
		static const float lbratio[] = { 0.2f, 0.15f, 0.40f,0.25f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row(ctx, NK_DYNAMIC, 45, 4, lbratio);
		nk_label(ctx, "Rank", NK_TEXT_LEFT);
		nk_spacing(ctx, 1);
		// username & points
		nk_label(ctx, "Name", NK_TEXT_LEFT);
		nk_label(ctx, "Kills", NK_TEXT_LEFT);
		for (int i = 0; i < GAME_SIZE; i++) {

			const char * player_id;
			string s = std::to_string(i + 1);
			player_id = s.c_str();
			const char * player_point;

			// total kills for player
			string point_s = std::to_string(kills[i]);
			player_point = point_s.c_str();

			nk_layout_row(ctx, NK_DYNAMIC, 45, 4, lbratio);
			nk_text(ctx, player_id, strlen(player_id), NK_TEXT_LEFT);

			switch (ordered_types[i])	// archetype icon on leaderboard
			{
				case MAGE	 : nk_image(ctx, media->mage);	  break;
				case ASSASSIN: nk_image(ctx, media->assasin); break;
				case WARRIOR : nk_image(ctx, media->warrior); break;
				case KING	 : nk_image(ctx, media->king);	  break;
			}
			
			// username & points
			nk_text(ctx, ordered_usernames[i].c_str(), strlen(ordered_usernames[i].c_str()), NK_TEXT_LEFT);
			nk_text(ctx, player_point, strlen(player_point), NK_TEXT_LEFT);
		}

	}
	nk_end(ctx);
}

static void ui_killphase_header(struct nk_context *ctx, struct media *media, int width, int height, int roundnum, int gold, int victory_points) {
	struct nk_style *s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(0, 0, 0, 0));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
	if (nk_begin(ctx, "kill_header", nk_rect(width * 0.85, 10, width * 0.15, width * 0.09+65),
		NK_WINDOW_NO_SCROLLBAR))
	{
		static const float kill_ratio[] = { 0.3f,0.3f, 0.4f };  /* 0.3 + 0.4 + 0.3 = 1 */
		string roundStr = "ROUND: " + std::to_string(roundnum);
		const char * round_char = roundStr.c_str();

		string goldStr = std::to_string(gold);
		string vicPtsStr = std::to_string(victory_points);
		const char * gold_char = goldStr.c_str();
		const char * vic_char = vicPtsStr.c_str();
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row_dynamic(ctx, 65, 1);
		nk_label(ctx, round_char, NK_TEXT_RIGHT | NK_TEXT_ALIGN_CENTERED);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));

		nk_layout_row(ctx, NK_DYNAMIC, width * 0.07,3, kill_ratio);

		nk_spacing(ctx, 1);
		if (nk_group_begin(ctx, "icons", NK_WINDOW_NO_SCROLLBAR)) { // column 1
			nk_layout_row_static(ctx, 48, 48, 1);
			nk_image(ctx, media->gold);
			nk_layout_row_static(ctx, 20, 1, 1);
			nk_layout_row_static(ctx, 48, 48, 1);
			nk_image(ctx, media->points);
		}
		nk_group_end(ctx);

		if (nk_group_begin(ctx, "nums", NK_WINDOW_NO_SCROLLBAR)) { // column 1
			nk_layout_row_static(ctx, 48, 48, 1);
			nk_label(ctx, gold_char, NK_TEXT_RIGHT | NK_TEXT_ALIGN_CENTERED);
			nk_layout_row_static(ctx, 20, 1, 1);
			nk_layout_row_static(ctx, 48, 48, 1);
			nk_text(ctx, vic_char, strlen(vic_char), NK_TEXT_RIGHT | NK_TEXT_ALIGN_CENTERED);
		}
		nk_group_end(ctx);
	}
	nk_end(ctx);
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
}

static void ui_skills(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player, vector<nanoseconds> skill_timers) {
	/*
	Q --> Directional Skill (with the exception of King)
	W --> Omni Directional Skill
	E --> Evade (omni)
	R --> Projectile (directional)

	*******    In meta_data.json    ******
	Skills MUST be in the order of: evade (0), projectile (1), omni (2), directional (3)
	*/
	static const char *key_bindings[] = { "Q","W","E","R" };
	static const unsigned int sequential_bindings[] = { DIR_SKILL_INDEX , OMNI_SKILL_INDEX , EVADE_INDEX , PROJ_INDEX };
	static int op = HUMAN;
	struct nk_style *s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(0, 0, 0, 0));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(0, 0, 0, 0)));
	nk_style_set_font(ctx, &(media->font_64->handle));
	if (nk_begin(ctx, "skills", nk_rect(width*0.3,  height*0.84, width*0.4, height*0.16),
		NK_WINDOW_NO_SCROLLBAR))
	{
		static const float ratio[] = { 0.143f,0.143f, 0.143f,0.143f, 0.143f,0.143f, 0.142f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row(ctx, NK_DYNAMIC, height *0.16, 7, ratio);
		ArcheType type = player->modelType; 
		for (int i = 0; i < 4; i++) {
			if (nk_group_begin(ctx, key_bindings[i], NK_WINDOW_NO_SCROLLBAR)) { // column 1
				nk_layout_row_dynamic(ctx, width *0.05, 1); // nested row
				std::chrono::nanoseconds nanoSecs = skill_timers[sequential_bindings[i]];
				if (nanoSecs > std::chrono::seconds::zero()){
					auto timeExpr = chrono::duration_cast<chrono::seconds>(nanoSecs);
					//to_string
					string result_string = to_string(timeExpr.count());
					const char * cooldown_char_array = result_string.c_str();
					nk_text(ctx, cooldown_char_array, strlen(cooldown_char_array), NK_TEXT_ALIGN_CENTERED);
				}
				else {
					if (type == WARRIOR) {
						if(i < 2 && player->isSilenced)
							nk_image(ctx, media->warrior_silenced[i]);
						else
							nk_image(ctx, media->warrior_skills[i]);
					}
					else if (type == MAGE) {
						if (i < 2 && player->isSilenced)
							nk_image(ctx, media->mage_silenced[i]);
						else
							nk_image(ctx, media->mage_skills[i]);
					}
					else if (type == ASSASSIN) {
						if (i < 2 && player->isSilenced)
							nk_image(ctx, media->assassin_silenced[i]);
						else
							nk_image(ctx, media->assassin_skills[i]);
					}
					else {
						if (i < 2 && player->isSilenced)
							nk_image(ctx, media->king_silenced[i]);
						else
							nk_image(ctx, media->king_skills[i]);
					}
				}
					nk_layout_row_dynamic(ctx, 24, 1);
					nk_text(ctx, key_bindings[i], strlen(key_bindings[i]), NK_TEXT_ALIGN_CENTERED);
				nk_group_end(ctx);
			}
			nk_spacing(ctx, 1);
		}

	}
	nk_end(ctx);
	nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
}


static void ui_deadscreen(struct nk_context *ctx, struct media *media, int width, int height, int killTextDeterminant) {
	int deathTextDisplay = killTextDeterminant% KILLED_TEXT_NUM;
	//add a full screen layout that is grey with transparency 0.5
	struct nk_style *s = &ctx->style;
	nk_style_push_color(ctx, &s->window.background, nk_rgba(20, 20, 20, 140));
	nk_style_push_style_item(ctx, &s->window.fixed_background, nk_style_item_color(nk_rgba(20, 20, 20, 140)));
	if (nk_begin(ctx, "death screen", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR)) {
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row_dynamic(ctx, height*0.4, 1);
		if (deathTextDisplay == 1)
			nk_label(ctx, "You have been smashed!!!", NK_TEXT_CENTERED | NK_TEXT_ALIGN_CENTERED);
		else if (deathTextDisplay == 2)
			nk_label(ctx, "You have been strangled!!!", NK_TEXT_CENTERED | NK_TEXT_ALIGN_CENTERED);
		else if (deathTextDisplay == 3)
			nk_label(ctx, "You have been murdered!!!", NK_TEXT_CENTERED | NK_TEXT_ALIGN_CENTERED);
		else
			nk_label(ctx, "You have been shot through the heart!", NK_TEXT_CENTERED | NK_TEXT_ALIGN_CENTERED);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
	} 
	nk_end(ctx);
	nk_style_pop_color(ctx);
	nk_style_pop_style_item(ctx);
}

static void ui_prepare_title(struct nk_context *ctx, struct media *media, int width, int height, char * title, ClientGame * game) {
		nk_layout_row_static(ctx, height*0.042, 10, 1);
		static const float ratio[] = { 0.5f, 0.5f };
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row(ctx, NK_DYNAMIC, 65, 2, ratio);
		nk_text(ctx, title, strlen(title), NK_TEXT_CENTERED);

		if (game->prepareTimer > std::chrono::seconds::zero()) {
			auto timeExpr = chrono::duration_cast<chrono::seconds>(game->prepareTimer);
			string result_string = to_string(timeExpr.count());
			char * result = new char[100];
			strcpy(result, ("Time Left:  " + result_string).c_str());
			//const char* result = ("Time Left:  " + result_string).c_str();
			nk_text(ctx, result, strlen(result), NK_TEXT_CENTERED);
		}
		else {
			game->switchPhase();
		}
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
}

static void
kill_layout(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player,
	vector<nanoseconds> skill_timers, LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes,
	int killTextDeterminant, ClientGame* game) {
	

	if (game->prepareTimer > std::chrono::seconds::zero()) {
		set_style(ctx, THEME_BLACK);
		if (!player->isAlive) {
			ui_deadscreen(ctx, media, width, height, killTextDeterminant);
		}
		ui_leaderboard(ctx, media, leaderBoard, usernames, archetypes);

		ui_skills(ctx, media, width, height, player, skill_timers);
		logger()->debug("PLAYER GOLD {}", player->gold);
		ui_killphase_header(ctx, media, width, height, 1, player->gold, 2);
	}
	else {
		game->switchPhase();
	}
}



static  void
lobby_layout(struct nk_context *ctx, struct media *media, int width, int height,  ClientGame * game) {
	static bool available = true;
	static bool selected = false;
	static char buf[256] = { 0 };
	set_style(ctx, THEME_BLACK);

	static ArcheType op = HUMAN;
	if (nk_begin(ctx, "Lobby", nk_rect(0, 0, width, height),
		NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR
	))
	{
		static const char * characterTypeStrings[] = { "HUMAN", "MAGE", "ASSASSIN","WARRIOR","KING" };
		static const float ratio[] = { 0.35f, 0.3f, 0.35f };  /* 0.3 + 0.4 + 0.3 = 1 */
		static const float text_input_ratio[] = { 0.15f, 0.85f };
		nk_layout_row(ctx, NK_DYNAMIC, 40, 2, ratio);
		nk_text(ctx, "Username: ", 10, NK_TEXT_ALIGN_RIGHT);
		// in window
		nk_edit_string_zero_terminated(ctx, NK_EDIT_FIELD, buf, sizeof(buf) - 1, nk_filter_default);

		static const float choice_ratio[] = { 0.18f, 0.16f, 0.16f, 0.16f, 0.16f,0.18f };
		nk_layout_row_static(ctx, 0.03*height, 15, 1);
		nk_layout_row(ctx, NK_DYNAMIC, height *0.35, 6, choice_ratio);
		// somewhere out of cycle
		nk_spacing(ctx, 1);
		for (int i = 1; i < 5; i++) {
			if (nk_group_begin(ctx, characterTypeStrings[i], NK_WINDOW_NO_SCROLLBAR)) { // column 1
				nk_layout_row_dynamic(ctx, width *0.15, 1); // nested row

				if (i == WARRIOR)
					nk_image(ctx, media->warrior);
				else if (i == MAGE)
					nk_image(ctx, media->mage);
				else if (i == ASSASSIN)
					nk_image(ctx, media->assasin);
				else
					nk_image(ctx, media->king);
				//nk_layout_row_static(ctx, 0.1*height, 15, 1);
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_option_label(ctx, characterTypeStrings[i], op == i)) op = static_cast<ArcheType>(i);

			}
			nk_group_end(ctx);
		}
		nk_spacing(ctx, 1);

		//horizontal centered
		nk_layout_row(ctx, NK_DYNAMIC, 60, 3, ratio);
		nk_spacing(ctx, 1);
		if (nk_button_label(ctx, "Confirm")) {
			fprintf(stdout, "button pressed, curr selection: %s, curr buf: %s\n", characterTypeStrings[op], buf);
			available = game->sendCharacterSelection(buf, op);
			selected = true;
		}

		nk_spacing(ctx, 1);
	}
	nk_end(ctx);

	if (!available) {
		if (nk_begin(ctx, "Alert", nk_rect(width*0.3, height*0.3, width*0.4, 200),
			NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_CLOSABLE
		)) {
			ui_widget_centered(ctx, media, 150);
			nk_text(ctx, "This Skeleton is no longer available!", 37, NK_TEXT_ALIGN_CENTERED);
			nk_spacing(ctx, 1);
		}
		else {
			available = true;
		}
		nk_end(ctx);
		selected = false;
	}
	else if (selected) {
		game->switchPhase();
	}
}

// order leaderboard by kills, most first
static void ui_round_results(struct nk_context *ctx, struct media *media,
	LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes, int width, int height, int & currPreparePage, ClientGame * game) {

	// order kills, usernames, and archetypes by client with most kills first...
	vector<int> kills;
	vector<string> ordered_usernames;
	vector<ArcheType> ordered_types;
	vector<int> curKills = leaderBoard->currentKills;
	static const float lbratio[] = { 0.20f, 0.08f, 0.02f, 0.30f,0.4f };  /* 0.3 + 0.4 + 0.3 = 1 */
	static const float globalLBratio[] = { 0.20f, 0.08f, 0.02f, 0.30f,0.20f,0.20f };  /* 0.3 + 0.4 + 0.3 = 1 */
	static const float btnRatio[] = { 0.97f, 0.03f };
	// make parallel arrays 'kills' & 'ordered_usernames' having same index for players based on number of kills
	for (int i = 0; i < GAME_SIZE; i++)
	{
		// find max element in list; get total kills for that player
		auto it = std::max_element(curKills.begin(), curKills.end());
		int index = it - curKills.begin();
		int numKills = *it;

		// add next client with most kills username, kills & archetype
		ordered_usernames.push_back(usernames[index]);
		ordered_types.push_back(archetypes[index]);
		kills.push_back(numKills);

		*it = -1;		// reset current max to -1
	}
	if (nk_begin(ctx, "round result", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_BACKGROUND))
	{
		ui_prepare_title(ctx, media, width, height, "Summary", game);
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row_dynamic(ctx, height*0.15, 1);
		nk_label(ctx, "Round Summary", NK_TEXT_LEFT | NK_TEXT_ALIGN_CENTERED);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
		nk_layout_row(ctx, NK_DYNAMIC, width*0.02, 5, lbratio);
		nk_spacing(ctx, 1);
		nk_label(ctx, "Rank", NK_TEXT_LEFT);
		nk_spacing(ctx, 1);
		// username & points
		nk_label(ctx, "Name", NK_TEXT_LEFT);
		nk_label(ctx, "Kills", NK_TEXT_LEFT);
		for (int i = 0; i < GAME_SIZE; i++) {

			const char * player_id;
			string s = std::to_string(i + 1);
			player_id = s.c_str();
			const char * player_point;

			// total kills for player
			string point_s = std::to_string(kills[i]);
			player_point = point_s.c_str();

			nk_layout_row(ctx, NK_DYNAMIC, width*0.02, 5, lbratio);
			nk_spacing(ctx, 1);
			nk_text(ctx, player_id, strlen(player_id), NK_TEXT_LEFT);
			switch (ordered_types[i])	// archetype icon on leaderboard
			{
			case MAGE: nk_image(ctx, media->mage);	  break;
			case ASSASSIN: nk_image(ctx, media->assasin); break;
			case WARRIOR: nk_image(ctx, media->warrior); break;
			case KING: nk_image(ctx, media->king);	  break;
			}

			// username & points
			nk_text(ctx, ordered_usernames[i].c_str(), strlen(ordered_usernames[i].c_str()), NK_TEXT_LEFT);
			nk_text(ctx, player_point, strlen(player_point), NK_TEXT_LEFT);
		}
		nk_style_set_font(ctx, &(media->font_64->handle));
		nk_layout_row_dynamic(ctx, height*0.2, 1);
		nk_label(ctx, "Overall Summary", NK_TEXT_LEFT | NK_TEXT_ALIGN_CENTERED);
		nk_style_set_font(ctx, &(glfw.atlas.default_font->handle));
		nk_layout_row(ctx, NK_DYNAMIC, width*0.02, 6, globalLBratio);
		nk_spacing(ctx, 1);
		nk_label(ctx, "Rank", NK_TEXT_LEFT);
		nk_spacing(ctx, 1);
		// username & points
		nk_label(ctx, "Name", NK_TEXT_LEFT);
		nk_label(ctx, "Gold", NK_TEXT_LEFT);
		nk_label(ctx, "Points", NK_TEXT_LEFT);
		for (int i = 0; i < GAME_SIZE; i++) {

			const char * player_id;
			string s = std::to_string(i + 1);
			player_id = s.c_str();
			const char * player_point;

			// total kills for player
			string point_s = std::to_string(kills[i]);
			player_point = point_s.c_str();

			nk_layout_row(ctx, NK_DYNAMIC, width*0.02, 6, globalLBratio);
			nk_spacing(ctx, 1);
			nk_text(ctx, player_id, strlen(player_id), NK_TEXT_LEFT);

			switch (ordered_types[i])	// archetype icon on leaderboard
			{
			case MAGE: nk_image(ctx, media->mage);	  break;
			case ASSASSIN: nk_image(ctx, media->assasin); break;
			case WARRIOR: nk_image(ctx, media->warrior); break;
			case KING: nk_image(ctx, media->king);	  break;
			}

			// TODO: SWITCH OUT WITH REAL SUMMARY PACKETS
			nk_text(ctx, ordered_usernames[i].c_str(), strlen(ordered_usernames[i].c_str()), NK_TEXT_LEFT);
			nk_text(ctx, player_point, strlen(player_point), NK_TEXT_LEFT);
			nk_text(ctx, player_point, strlen(player_point), NK_TEXT_LEFT);
		}

	}
	nk_end(ctx);
	if (nk_begin(ctx, "SwitchPage", nk_rect(0.03*width, 0.9*height, height*0.1, height*0.1),
		NK_WINDOW_NO_SCROLLBAR| NK_WINDOW_BORDER)) {
		nk_layout_row_static(ctx, height*0.1, height*0.1, 1);
		if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_RIGHT)) {
			currPreparePage = 1;
		}
	}
	nk_end(ctx);
}

static void ui_shop(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player, int & currPreparePage, ClientGame * game) {

	if (nk_begin(ctx, "Prepare", nk_rect(0, 0, width, height),
		NK_WINDOW_NO_SCROLLBAR|NK_WINDOW_BACKGROUND
	))
	{
		char* skill_string[4] = { "Evade", "Projectile", "AOE", "Cone AOE" };
		char* prices[4] = { "5", "10", "15", "20" };

		static int op = 0;
		static const float ratio[] = { 0.35f, 0.3f, 0.35f };  /* 0.3 + 0.4 + 0.3 = 1 */
		

		static const float choice_ratio[] = { 0.12f, 0.19f, 0.19f, 0.19f, 0.19f,0.12f };

		ui_prepare_title(ctx, media, width, height, "Shop", game);

		nk_layout_row(ctx, NK_DYNAMIC, height *0.35, 6, choice_ratio);
		nk_spacing(ctx, 1);
		for (int j = 0; j < 3; j++) {
			int i = j / 2;
			if (j % 2 == 0) {
				if (nk_group_begin(ctx, skill_string[i], NK_WINDOW_NO_SCROLLBAR)) { // column 1
					nk_layout_row_dynamic(ctx, width *0.15, 1); // nested row

					nk_image(ctx, media->mage_skills[i]);
					nk_layout_row_dynamic(ctx, 20, 1);
					nk_text(ctx, skill_string[i], strlen(skill_string[i]), NK_TEXT_LEFT);
					nk_text(ctx, prices[i], strlen(prices[i]), NK_TEXT_LEFT);
					nk_layout_row_dynamic(ctx, 20, 1);
					if (nk_button_label(ctx, "LEVEL UP")) {
						//TODO: level up
					}

					nk_group_end(ctx);
				}
			}
			else {
				nk_spacing(ctx, 1);
			}
		}
		nk_spacing(ctx, 0.3);

		nk_layout_row(ctx, NK_DYNAMIC, height *0.4, 5, choice_ratio);
		nk_spacing(ctx, 1);
		for (int j = 2; j < 5; j++) {
			int i = j / 2 + 1;
			if (j % 2 == 0) {
				if (nk_group_begin(ctx, skill_string[i], NK_WINDOW_NO_SCROLLBAR)) { // column 1
					nk_layout_row_dynamic(ctx, width *0.15, 1); // nested row

					nk_image(ctx, media->mage_skills[i]);
					nk_layout_row_dynamic(ctx, 20, 1);
					nk_text(ctx, skill_string[i], strlen(skill_string[i]), NK_TEXT_LEFT);
					nk_text(ctx, prices[i], strlen(prices[i]), NK_TEXT_LEFT);
					nk_layout_row_dynamic(ctx, 20, 1);
					if (nk_button_label(ctx, "LEVEL UP")) {
						//TODO: level up
					}

					nk_group_end(ctx);
				}
			}
			else {
				nk_spacing(ctx, 1);
			}
		}

	}
	nk_end(ctx);

	if (nk_begin(ctx, "SwitchPageBack", nk_rect(0.03*width, 0.9*height, height*0.1, height*0.1),
		NK_WINDOW_NO_SCROLLBAR| NK_WINDOW_BORDER)) {
		nk_layout_row_static(ctx, height*0.1, height*0.1, 1);
		if (nk_button_symbol(ctx, NK_SYMBOL_TRIANGLE_LEFT)) {
			currPreparePage = 0;
		}
	}
	nk_end(ctx);
}

static  void
prepare_layout(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player, LeaderBoard* leaderBoard, vector<string> usernames, vector<ArcheType> archetypes, ClientGame * game) {
	static int curr = 0;// 0 for summary, 1 for shop
	set_style(ctx, THEME_BLACK);
	if (curr == 0) {
		ui_round_results(ctx, media, leaderBoard, usernames, archetypes, width, height, curr, game);
	}
	else {
		ui_shop(ctx, media, width, height, player, curr, game);
	}
}