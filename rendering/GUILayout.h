#pragma once
#include "../../NuklearInit.h"

const char * intToCharArray(int i) {
	string s = std::to_string(i);
	const int n = s.length();
	return s.c_str();
}

static void ui_leaderboard(struct nk_context *ctx, struct media *media) {
	static const char *items[] = { "Player 0","Player 1","Player 2","Player 3" };
	static const int points[] = { 15,40,30,10 };

	if (nk_begin(ctx, "Leaderboard", nk_rect(10, 10, 300, 300),
		NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{
		static const float ratio[] = { 0.2f, 0.15f, 0.40f,0.25f };  /* 0.3 + 0.4 + 0.3 = 1 */
		for (int i = 0; i < 4; i++) {
			const char * player_id;
			string s = std::to_string(i);
			player_id = s.c_str();
			const char * player_point;
			string point_s = std::to_string(points[i]);
			player_point = point_s.c_str();
			nk_layout_row(ctx, NK_DYNAMIC, 45, 4, ratio);
			nk_text(ctx, player_id, strlen(player_id), NK_TEXT_LEFT);
			nk_image(ctx, media->king);
			nk_text(ctx, items[i], strlen(items[i]), NK_TEXT_LEFT);
			nk_text(ctx, player_point, strlen(player_point), NK_TEXT_LEFT);
		}

	}
	nk_end(ctx);
}
static void ui_skills(struct nk_context *ctx, struct media *media, int width, int height) {
	static const char *key_bindings[] = { "Q","W","E","R" };
	static const float cds[] = { 15,40,30,10 };
	static int op = HUMAN;
	if (nk_begin(ctx, "skills", nk_rect(width*0.3,  height*0.85, width*0.4, height*0.18),
		NK_WINDOW_BORDER| NK_WINDOW_NO_SCROLLBAR))
	{
		static const float ratio[] = { 0.125f,0.125f, 0.125f,0.125f, 0.125f,0.125f, 0.125f,0.125f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row(ctx, NK_DYNAMIC, height *0.18, 8, ratio);
		for (int i = 0; i < 4; i++) {
			if (nk_group_begin(ctx, key_bindings[i], NK_WINDOW_NO_SCROLLBAR)) { // column 1
				nk_layout_row_dynamic(ctx, width *0.05, 1); // nested row

				if (i == KING)
					nk_image(ctx, media->king);
				else if (i == MAGE)
					nk_image(ctx, media->mage);
				else if (i == ASSASSIN)
					nk_image(ctx, media->assasin);
				else
					nk_image(ctx, media->warrior);
				nk_layout_row_dynamic(ctx, 24, 1);
				nk_text(ctx, key_bindings[i], strlen(key_bindings[i]), NK_TEXT_ALIGN_CENTERED);
				/*const char * skill_cd;
				string s = std::to_string((int)cds[i]);
				skill_cd = s.c_str();
				nk_layout_row_dynamic(ctx, 24, 1);
				nk_text(ctx, skill_cd, strlen(skill_cd), NK_TEXT_ALIGN_CENTERED);*/
				nk_group_end(ctx);
			}
			nk_spacing(ctx, 1);
		}

	}
	nk_end(ctx);
}
static void
kill_layout(struct nk_context *ctx, struct media *media, int width, int height, struct nk_color background_color,ClientScene * scene) {
	ctx->style.window.fixed_background = nk_style_item_color(background_color);

	ui_leaderboard(ctx, media);

	ui_skills(ctx, media,  width,  height);
}

static  void
lobby_layout(struct nk_context *ctx, struct media *media, int width, int height, struct nk_color background_color, ClientScene * scene) {
	static bool available = false;

	ctx->style.window.fixed_background = nk_style_item_color(background_color);
	ctx->style.button.normal = nk_style_item_color(nk_rgb(200, 140, 200));
	ctx->style.button.hover = nk_style_item_color(nk_rgb(140, 80, 140));
	ctx->style.button.active = nk_style_item_color(nk_rgb(120, 40, 120));
	ctx->style.button.text_background = nk_rgb(140, 80, 140);
	ctx->style.button.text_normal = nk_rgb(140, 80, 140);
	ctx->style.button.text_hover = nk_rgb(240, 180, 240);
	ctx->style.button.text_active = nk_rgb(240, 180, 240);

	ctx->style.option.normal = nk_style_item_color(nk_rgb(200, 140, 200));
	ctx->style.option.hover = nk_style_item_color(nk_rgb(140, 80, 140));
	ctx->style.option.active = nk_style_item_color(nk_rgb(160, 120, 160));
	ctx->style.option.text_normal = nk_rgb(200, 140, 200);
	ctx->style.option.text_hover = nk_rgb(140, 80, 140);
	ctx->style.option.text_active = nk_rgb(160, 120, 160);
	if (nk_begin(ctx, "Lobby", nk_rect(0, 0, width, height),
		NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR
	))
	{
		static const char * characterTypeStrings[] = { "KING", "MAGE", "ASSASIN", "WARRIOR" };
		static int op = HUMAN;
		static const float ratio[] = { 0.35f, 0.3f, 0.35f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row_static(ctx, 0.15*height, 15, 1);

		static const float choice_ratio[] = { 0.12f, 0.19f, 0.19f, 0.19f, 0.19f,0.12f };

		nk_layout_row(ctx , NK_DYNAMIC, height *0.35, 6, choice_ratio);
		nk_spacing(ctx, 1);
		for (int i = 0; i < 4; i++) {
			if (nk_group_begin(ctx, characterTypeStrings[i], NK_WINDOW_NO_SCROLLBAR)) { // column 1
				nk_layout_row_dynamic(ctx, width *0.18, 1); // nested row

				if(i==KING)
					nk_image(ctx, media->king);
				else if (i == MAGE)
					nk_image(ctx, media->mage);
				else if (i == ASSASSIN)
					nk_image(ctx, media->assasin);
				else
					nk_image(ctx, media->warrior);
				//nk_layout_row_static(ctx, 0.1*height, 15, 1);
				nk_layout_row_dynamic(ctx, 30, 1);
				if (nk_option_label(ctx, characterTypeStrings[i], op == i)) op = i;

				nk_group_end(ctx);
			}
		}
		nk_spacing(ctx, 1);

		//horizontal centered
		nk_layout_row(ctx, NK_DYNAMIC, 50, 3, ratio);
		nk_spacing(ctx, 1);
		if (nk_button_label(ctx, "Confirm"))
			fprintf(stdout, "button pressed\n");
		nk_spacing(ctx, 1);

	}
	nk_end(ctx);


	ctx->style.window.fixed_background = nk_style_item_color(nk_white);
	if (!available) {
		if (nk_begin(ctx, "Alert", nk_rect(width*0.3, height*0.3, width*0.4, 200),
			NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_CLOSABLE
		)) {
			//nk_layout_row_static(ctx, 0.05*height, 15, 1);
			ui_widget_centered(ctx, media, 150);
			nk_text(ctx, "This Skeleton is no longer available!", 37, NK_TEXT_ALIGN_CENTERED);
			nk_spacing(ctx, 1);
		}
		nk_end(ctx);
	}
}

static  void
prepare_layout(struct nk_context *ctx, struct media *media, int width, int height, struct nk_color background_color, ClientScene * scene) {
	
	ctx->style.window.fixed_background = nk_style_item_color(background_color);
	ctx->style.button.normal = nk_style_item_color(nk_rgb(200, 140, 200));
	ctx->style.button.hover = nk_style_item_color(nk_rgb(140, 80, 140));
	ctx->style.button.active = nk_style_item_color(nk_rgb(120, 40, 120));
	ctx->style.button.text_background = nk_rgb(140, 80, 140);
	ctx->style.button.text_normal = nk_rgb(140, 80, 140);
	ctx->style.button.text_hover = nk_rgb(240, 180, 240);
	ctx->style.button.text_active = nk_rgb(240, 180, 240);

	ctx->style.option.normal = nk_style_item_color(nk_rgb(200, 140, 200));
	ctx->style.option.hover = nk_style_item_color(nk_rgb(140, 80, 140));
	ctx->style.option.active = nk_style_item_color(nk_rgb(160, 120, 160));
	ctx->style.option.text_normal = nk_rgb(200, 140, 200);
	ctx->style.option.text_hover = nk_rgb(140, 80, 140);
	ctx->style.option.text_active = nk_rgb(160, 120, 160);
	if (nk_begin(ctx, "Prepare", nk_rect(0, 0, width, height),
		NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR
	))
	{
		//typedef enum { KING, MAGE, ASSASIN, WARRIOR } characterType;
		//static const char * characterTypeStrings[] = { "KING", "MAGE", "ASSASIN", "WARRIOR" };
		// TODO: define skill string
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
			fprintf(stdout, "button pressed\n");
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