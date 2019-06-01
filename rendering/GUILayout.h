#pragma once
#include "../../NuklearInit.h"
#define EVADE_INDEX 0
#define PROJ_INDEX 1
#define OMNI_SKILL_INDEX 2
#define DIR_SKILL_INDEX 3
const char * intToCharArray(int i) {
	string s = std::to_string(i);
	const int n = s.length();
	return s.c_str();
}


// order leaderboard by kills, most first
static void ui_leaderboard(struct nk_context *ctx, struct media *media,
	LeaderBoard* leaderBoard, vector<string> usernames) {

	vector<int> curKills = leaderBoard->currentKills;
	vector<string> ordered_usernames;
	vector<int> kills;
	for ( int i = 0; i < GAME_SIZE; i++)
	{
		// find max element in list; get total kills for that player
		auto it = std::max_element(curKills.begin(), curKills.end());
		int index = it - curKills.begin();
		int numKills = *it;

		// add next highest score & username
		ordered_usernames.push_back(usernames[index]);
		kills.push_back(numKills);

		*it = -1;		// reset current max to -1
	}

	if (nk_begin(ctx, "Leaderboard", nk_rect(10, 10, 300, 300),
		NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{
		static const float ratio[] = { 0.25f, 0.50f,0.25f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row(ctx, NK_DYNAMIC, 45, 3, ratio);
		nk_label(ctx, "Rank", NK_TEXT_LEFT);

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

			nk_layout_row(ctx, NK_DYNAMIC, 45, 3, ratio);
			nk_text(ctx, player_id, strlen(player_id), NK_TEXT_LEFT);
			//nk_image(ctx, media->king);

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
		static const float ratio[] = { 0.3f,0.3f, 0.4f };  /* 0.3 + 0.4 + 0.3 = 1 */
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

		nk_layout_row(ctx, NK_DYNAMIC, width * 0.07,3, ratio);

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
					if (type == WARRIOR)
						nk_image(ctx, media->warrior_skills[i]);
					else if (type == MAGE)
						nk_image(ctx, media->mage_skills[i]);
					else if (type == ASSASSIN)
						nk_image(ctx, media->assassin_skills[i]);
					else
						nk_image(ctx, media->king_skills[i]);
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
static void
kill_layout(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player,
	vector<nanoseconds> skill_timers, LeaderBoard* leaderBoard, vector<string> usernames) {
	
	set_style(ctx, THEME_BLACK);

	ui_leaderboard(ctx, media, leaderBoard, usernames);

	ui_skills(ctx, media,  width,  height, player, skill_timers);
	ui_killphase_header(ctx, media, width, height, 1, 10, 2);
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
		static const char * characterTypeStrings[] = { "HUMAN", "MAGE", "ASSASIN","WARRIOR","KING" };
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
		game->waitingInitScene();
	}
}

static  void
prepare_layout(struct nk_context *ctx, struct media *media, int width, int height, ScenePlayer * player) {
	
	set_style(ctx, THEME_BLACK);
	if (nk_begin(ctx, "Prepare", nk_rect(0, 0, width, height),
		NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR
	))
	{
		char* skill_string [4] = { "Evade", "Projectile", "AOE", "Cone AOE" };
		char* prices[4] = { "5", "10", "15", "20" };

		static int op = 0;
		static const float ratio[] = { 0.35f, 0.3f, 0.35f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row_static(ctx, 0.15*height, 15, 1);

		static const float choice_ratio[] = { 0.12f, 0.19f, 0.19f, 0.19f, 0.19f,0.12f };

		nk_layout_row(ctx, NK_DYNAMIC, height *0.35, 6, choice_ratio);
		nk_spacing(ctx, 1);
		for (int i = 0; i < 4; i++) {
			if (nk_group_begin(ctx, skill_string[i], NK_WINDOW_NO_SCROLLBAR)) { // column 1
				nk_layout_row_dynamic(ctx, width *0.18, 1); // nested row

				nk_image(ctx, media->mage_skills[i]);
				//nk_layout_row_static(ctx, 0.1*height, 15, 1);
				nk_layout_row_dynamic(ctx, 20, 1);
				if (nk_option_label(ctx, skill_string[i], op == i)) op = i;
				nk_text(ctx, prices[i], strlen(prices[i]), NK_TEXT_RIGHT);

				nk_group_end(ctx);
			}
		}
		nk_spacing(ctx, 1);

		//horizontal centered
		nk_layout_row(ctx, NK_DYNAMIC, 50, 3, ratio);
		nk_spacing(ctx, 1);
		if (nk_button_label(ctx, "Confirm"))
			fprintf(stdout, "button pressed\n" );
		nk_spacing(ctx, 1);

	}
	nk_end(ctx);
}

//static void
//basic_demo(struct nk_context *ctx, struct media *media)
//{
//	static int image_active;
//	static int check0 = 1;
//	static int check1 = 0;
//	static size_t prog = 80;
//	static int selected_item = 0;
//	static int selected_image = 3;
//	static int selected_icon = 0;
//	static const char *items[] = { "Item 0","item 1","item 2" };
//	static int piemenu_active = 1;
//	static struct nk_vec2 piemenu_pos;
//
//	int i = 0;
//	nk_style_set_font(ctx, &media->font_20->handle);
//	nk_begin(ctx, "Basic Demo", nk_rect(320, 50, 275, 610),
//		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE);
//
//	/*------------------------------------------------
//	 *                  POPUP BUTTON
//	 *------------------------------------------------*/
//	ui_header(ctx, media, "Popup & Scrollbar & Images");
//	ui_widget(ctx, media, 35);
//	if (nk_button_image_label(ctx, media->dir, "Images", NK_TEXT_CENTERED))
//		image_active = !image_active;
//
//	/*------------------------------------------------
//	 *                  SELECTED IMAGE
//	 *------------------------------------------------*/
//	ui_header(ctx, media, "Selected Image");
//	ui_widget_centered(ctx, media, 100);
//	nk_image(ctx, media->images[selected_image]);
//
//	/*------------------------------------------------
//	 *                  IMAGE POPUP
//	 *------------------------------------------------*/
//	if (image_active) {
//		struct nk_panel popup;
//		if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Image Popup", 0, nk_rect(265, 0, 320, 220))) {
//			nk_layout_row_static(ctx, 82, 82, 3);
//			for (i = 0; i < 9; ++i) {
//				if (nk_button_image(ctx, media->images[i])) {
//					selected_image = i;
//					image_active = 0;
//					nk_popup_close(ctx);
//				}
//			}
//			nk_popup_end(ctx);
//		}
//	}
//	/*------------------------------------------------
//	 *                  COMBOBOX
//	 *------------------------------------------------*/
//	ui_header(ctx, media, "Combo box");
//	ui_widget(ctx, media, 80);
//	if (nk_combo_begin_label(ctx, items[selected_item], nk_vec2(nk_widget_width(ctx), 200))) {
//		nk_layout_row_dynamic(ctx, 35, 1);
//		for (i = 0; i < 3; ++i)
//			if (nk_combo_item_label(ctx, items[i], NK_TEXT_LEFT))
//				selected_item = i;
//		nk_combo_end(ctx);
//	}
//
//	ui_widget(ctx, media, 40);
//	if (nk_combo_begin_image_label(ctx, items[selected_icon], media->images[selected_icon], nk_vec2(nk_widget_width(ctx), 200))) {
//		nk_layout_row_dynamic(ctx, 35, 1);
//		for (i = 0; i < 3; ++i)
//			if (nk_combo_item_image_label(ctx, media->images[i], items[i], NK_TEXT_RIGHT))
//				selected_icon = i;
//		nk_combo_end(ctx);
//	}
//
//	/*------------------------------------------------
//	 *                  CHECKBOX
//	 *------------------------------------------------*/
//	ui_header(ctx, media, "Checkbox");
//	ui_widget(ctx, media, 60);
//	nk_checkbox_label(ctx, "Flag 1", &check0);
//	ui_widget(ctx, media, 30);
//	nk_checkbox_label(ctx, "Flag 2", &check1);
//
//	/*------------------------------------------------
//	 *                  PROGRESSBAR
//	 *------------------------------------------------*/
//	ui_header(ctx, media, "Progressbar");
//	ui_widget(ctx, media, 35);
//	nk_progress(ctx, &prog, 100, nk_true);
//
//	/*------------------------------------------------
//	 *                  PIEMENU
//	 *------------------------------------------------*/
//	if (nk_input_is_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_RIGHT,
//		nk_window_get_bounds(ctx), nk_true)) {
//		piemenu_pos = ctx->input.mouse.pos;
//		piemenu_active = 1;
//	}
//
//	if (piemenu_active) {
//		int ret = ui_piemenu(ctx, piemenu_pos, 140, &media->menu[0], 6);
//		if (ret == -2) piemenu_active = 0;
//		if (ret != -1) {
//			fprintf(stdout, "piemenu selected: %d\n", ret);
//			piemenu_active = 0;
//		}
//	}
//	nk_style_set_font(ctx, &media->font_14->handle);
//	nk_end(ctx);
//}