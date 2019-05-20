#pragma once
#include "../../NuklearInit.h"
//static void
//button_demo(struct nk_context *ctx, struct media *media)
//{
//	static int option = 1;
//	static int toggle0 = 1;
//	static int toggle1 = 0;
//	static int toggle2 = 1;
//
//	nk_style_set_font(ctx, &media->font_20->handle);
//	nk_begin(ctx, "Button Demo", nk_rect(50, 50, 255, 610),
//		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE);
//
//	/*------------------------------------------------
//	 *                  MENU
//	 *------------------------------------------------*/
//	nk_menubar_begin(ctx);
//	{
//		/* toolbar */
//		nk_layout_row_static(ctx, 40, 40, 4);
//		if (nk_menu_begin_image(ctx, "Music", media->play, nk_vec2(110, 120)))
//		{
//			/* settings */
//			nk_layout_row_dynamic(ctx, 25, 1);
//			nk_menu_item_image_label(ctx, media->play, "Play", NK_TEXT_RIGHT);
//			nk_menu_item_image_label(ctx, media->stop, "Stop", NK_TEXT_RIGHT);
//			nk_menu_item_image_label(ctx, media->pause, "Pause", NK_TEXT_RIGHT);
//			nk_menu_item_image_label(ctx, media->next, "Next", NK_TEXT_RIGHT);
//			nk_menu_item_image_label(ctx, media->prev, "Prev", NK_TEXT_RIGHT);
//			nk_menu_end(ctx);
//		}
//		nk_button_image(ctx, media->tools);
//		nk_button_image(ctx, media->cloud);
//		nk_button_image(ctx, media->pen);
//	}
//	nk_menubar_end(ctx);
//
//	/*------------------------------------------------
//	 *                  BUTTON
//	 *------------------------------------------------*/
//	ui_header(ctx, media, "Push buttons");
//	ui_widget(ctx, media, 35);
//	if (nk_button_label(ctx, "Push me"))
//		fprintf(stdout, "pushed!\n");
//	ui_widget(ctx, media, 35);
//	if (nk_button_image_label(ctx, media->rocket, "Styled", NK_TEXT_CENTERED))
//		fprintf(stdout, "rocket!\n");
//
//	/*------------------------------------------------
//	 *                  REPEATER
//	 *------------------------------------------------*/
//	ui_header(ctx, media, "Repeater");
//	ui_widget(ctx, media, 35);
//	if (nk_button_label(ctx, "Press me"))
//		fprintf(stdout, "pressed!\n");
//
//	/*------------------------------------------------
//	 *                  TOGGLE
//	 *------------------------------------------------*/
//	ui_header(ctx, media, "Toggle buttons");
//	ui_widget(ctx, media, 35);
//	if (nk_button_image_label(ctx, (toggle0) ? media->checked : media->unchecked, "Toggle", NK_TEXT_LEFT))
//		toggle0 = !toggle0;
//
//	ui_widget(ctx, media, 35);
//	if (nk_button_image_label(ctx, (toggle1) ? media->checked : media->unchecked, "Toggle", NK_TEXT_LEFT))
//		toggle1 = !toggle1;
//
//	ui_widget(ctx, media, 35);
//	if (nk_button_image_label(ctx, (toggle2) ? media->checked : media->unchecked, "Toggle", NK_TEXT_LEFT))
//		toggle2 = !toggle2;
//
//	/*------------------------------------------------
//	 *                  RADIO
//	 *------------------------------------------------*/
//	ui_header(ctx, media, "Radio buttons");
//	ui_widget(ctx, media, 35);
//	if (nk_button_symbol_label(ctx, (option == 0) ? NK_SYMBOL_CIRCLE_OUTLINE : NK_SYMBOL_CIRCLE_SOLID, "Select", NK_TEXT_LEFT))
//		option = 0;
//	ui_widget(ctx, media, 35);
//	if (nk_button_symbol_label(ctx, (option == 1) ? NK_SYMBOL_CIRCLE_OUTLINE : NK_SYMBOL_CIRCLE_SOLID, "Select", NK_TEXT_LEFT))
//		option = 1;
//	ui_widget(ctx, media, 35);
//	if (nk_button_symbol_label(ctx, (option == 2) ? NK_SYMBOL_CIRCLE_OUTLINE : NK_SYMBOL_CIRCLE_SOLID, "Select", NK_TEXT_LEFT))
//		option = 2;
//
//	/*------------------------------------------------
//	 *                  CONTEXTUAL
//	 *------------------------------------------------*/
//	nk_style_set_font(ctx, &media->font_18->handle);
//	if (nk_contextual_begin(ctx, NK_WINDOW_NO_SCROLLBAR, nk_vec2(150, 300), nk_window_get_bounds(ctx))) {
//		nk_layout_row_dynamic(ctx, 30, 1);
//		if (nk_contextual_item_image_label(ctx, media->copy, "Clone", NK_TEXT_RIGHT))
//			fprintf(stdout, "pressed clone!\n");
//		if (nk_contextual_item_image_label(ctx, media->del, "Delete", NK_TEXT_RIGHT))
//			fprintf(stdout, "pressed delete!\n");
//		if (nk_contextual_item_image_label(ctx, media->convert, "Convert", NK_TEXT_RIGHT))
//			fprintf(stdout, "pressed convert!\n");
//		if (nk_contextual_item_image_label(ctx, media->edit, "Edit", NK_TEXT_RIGHT))
//			fprintf(stdout, "pressed edit!\n");
//		nk_contextual_end(ctx);
//	}
//	nk_style_set_font(ctx, &media->font_14->handle);
//	nk_end(ctx);
//}

/* ===============================================================
 *
 *                          BASIC DEMO
 *
 * ===============================================================*/
static  void
kill_layout(struct nk_context *ctx, struct media *media, int width, int height, struct nk_color background_color) {
	//ctx->style.window.fixed_background = nk_style_item_color({);
	ctx->style.window.fixed_background = nk_style_item_color(background_color);

	if (nk_begin(ctx, "scoreboard", nk_rect(10, 10, width*0.4, height*0.4),
		NK_WINDOW_BORDER | NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{
		enum { EASY, HARD };
		static int op = EASY;
		nk_layout_row_static(ctx, height*0.1, width*0.1, 1);
		if (nk_button_label(ctx, "button"))
			fprintf(stdout, "button pressed\n");

		nk_layout_row_dynamic(ctx, height*0.2, 2);
		if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
		if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

	}
	nk_end(ctx);

}

static  void
lobby_layout(struct nk_context *ctx, struct media *media, int width, int height, struct nk_color background_color) {
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
		NK_WINDOW_BORDER | NK_WINDOW_NO_SCROLLBAR //| NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
		//NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE
	))
	{
		enum { HUMAN, MAGE, ASSASIN, WARRIOR, KING };
		static int op = HUMAN;
		static const float ratio[] = { 0.3f, 0.4f, 0.3f };  /* 0.3 + 0.4 + 0.3 = 1 */
		nk_layout_row_static(ctx, 0.2*height, 15, 1);

		static const float choice_ratio[] = { 0.15f, 0.20f, 0.20f, 0.20f, 0.25f };
		/*nk_layout_row(ctx , NK_DYNAMIC, height *0.2, 5, choice_ratio);
		nk_spacing(ctx, 1);
		nk_image(ctx, media->king);
		nk_image(ctx, media->mage);
		nk_image(ctx, media->assasin);
		nk_image(ctx, media->warrior);*/
		if (!available) {
			nk_layout_row_static(ctx, 0.05*height, 15, 1);
			ui_widget_centered(ctx, media, 0.1*height);
			nk_text(ctx, "This Skeleton is no longer available", 36, NK_TEXT_ALIGN_CENTERED);
			nk_spacing(ctx, 1);
		}
		else {
			nk_layout_row_static(ctx, 0.15*height, 15, 1);
		}
		nk_layout_row(ctx, NK_DYNAMIC, height *0.2, 5, choice_ratio);
		nk_spacing(ctx, 1);
		if (nk_option_label(ctx, "HUMAN", op == HUMAN)) op = HUMAN;
		if (nk_option_label(ctx, "MAGE", op == MAGE)) op = MAGE;
		if (nk_option_label(ctx, "ASSASIN", op == ASSASIN)) op = ASSASIN;
		if (nk_option_label(ctx, "WARRIOR", op == WARRIOR)) op = WARRIOR;
		
		//horizontal centered
		nk_layout_row_static(ctx, 0.05*height, 15, 1);
		nk_layout_row(ctx, NK_DYNAMIC, height *0.1, 3, ratio);
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