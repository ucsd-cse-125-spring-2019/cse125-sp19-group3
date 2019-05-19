/*
 * Nuklear - 1.32.0 - public domain
 * no warrenty implied; use at your own risk.
 * authored from 2015-2016 by Micha Mettke
 */
/*
 * ==============================================================
 *
 *                              API
 *
 * ===============================================================
 */
#ifndef NUKLEARINIT_H_
#define NUKLEARINIT_H_


#include <GLFW/glfw3.h>
#define NK_INCLUDE_FIXED_TYPES
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_STANDARD_VARARGS
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#define NK_GLFW_GL3_IMPLEMENTATION
#define NK_KEYSTATE_BASED_INPUT
#include "nuklear-master/nuklear.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#define UNUSED(a) (void)a
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) < (b) ? (b) : (a))
#define LEN(a) (sizeof(a)/sizeof(a)[0])
struct media {
	struct nk_font *font_14;
	struct nk_font *font_18;
	struct nk_font *font_20;
	struct nk_font *font_22;
	struct nk_font *font_32;
	struct nk_font *font_64;
	struct nk_font *font_128;

	struct nk_image unchecked;
	struct nk_image checked;
	struct nk_image rocket;
	struct nk_image cloud;
	struct nk_image pen;
	struct nk_image play;
	struct nk_image pause;
	struct nk_image stop;
	struct nk_image prev;
	struct nk_image next;
	struct nk_image tools;
	struct nk_image dir;
	struct nk_image copy;
	struct nk_image convert;
	struct nk_image del;
	struct nk_image edit;
	struct nk_image images[9];
	struct nk_image menu[6];
};


struct device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vert_shdr;
    GLuint frag_shdr;
    GLint attrib_pos;
    GLint attrib_uv;
    GLint attrib_col;
    GLint uniform_tex;
    GLint uniform_proj;
    GLuint font_tex;
};

enum nk_glfw_init_state{
    NK_GLFW3_DEFAULT=0,
    NK_GLFW3_INSTALL_CALLBACKS
};

NK_API struct nk_context*   nk_glfw3_init(GLFWwindow *win, enum nk_glfw_init_state);
NK_API void                 nk_glfw3_shutdown(void);
NK_API void                 nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas);
NK_API void                 nk_glfw3_font_stash_end(void);
NK_API void                 nk_glfw3_new_frame(void);
NK_API void                 nk_glfw3_render(enum nk_anti_aliasing, int max_vertex_buffer, int max_element_buffer);

NK_API void                 nk_glfw3_device_destroy(void);
NK_API void                 nk_glfw3_device_create(void);
NK_API void					text_input(GLFWwindow *win, unsigned int codepoint);
NK_API void                 nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint);
NK_API void                 nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff);
NK_API void                 nk_glfw3_mouse_button_callback(GLFWwindow *win, int button, int action, int mods);

#endif

static void
die(const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap);
	va_end(ap);
	fputs("\n", stderr);
	exit(EXIT_FAILURE);
}

static struct nk_image
icon_load(const char *filename)
{
	int x, y, n;
	GLuint tex;
	unsigned char *data = stbi_load(filename, &x, &y, &n, 0);
	if (!data) die("[SDL]: failed to load image: %s", filename);

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(data);
	return nk_image_id((int)tex);
}

static void
device_upload_atlas(struct device *dev, const void *image, int width, int height)
{
	glGenTextures(1, &dev->font_tex);
	glBindTexture(GL_TEXTURE_2D, dev->font_tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
		GL_RGBA, GL_UNSIGNED_BYTE, image);
}
/* ===============================================================
 *
 *                          CUSTOM WIDGET
 *
 * ===============================================================*/
static int
ui_piemenu(struct nk_context *ctx, struct nk_vec2 pos, float radius,
	struct nk_image *icons, int item_count)
{
	int ret = -1;
	struct nk_rect total_space;
	struct nk_rect bounds;
	int active_item = 0;

	/* pie menu popup */
	struct nk_color border = ctx->style.window.border_color;
	struct nk_style_item background = ctx->style.window.fixed_background;
	ctx->style.window.fixed_background = nk_style_item_hide();
	ctx->style.window.border_color = nk_rgba(0, 0, 0, 0);

	total_space = nk_window_get_content_region(ctx);
	ctx->style.window.spacing = nk_vec2(0, 0);
	ctx->style.window.padding = nk_vec2(0, 0);

	if (nk_popup_begin(ctx, NK_POPUP_STATIC, "piemenu", NK_WINDOW_NO_SCROLLBAR,
		nk_rect(pos.x - total_space.x - radius, pos.y - radius - total_space.y,
			2 * radius, 2 * radius)))
	{
		int i = 0;
		struct nk_command_buffer* out = nk_window_get_canvas(ctx);
		const struct nk_input *in = &ctx->input;

		total_space = nk_window_get_content_region(ctx);
		ctx->style.window.spacing = nk_vec2(4, 4);
		ctx->style.window.padding = nk_vec2(8, 8);
		nk_layout_row_dynamic(ctx, total_space.h, 1);
		nk_widget(&bounds, ctx);

		/* outer circle */
		nk_fill_circle(out, bounds, nk_rgb(50, 50, 50));
		{
			/* circle buttons */
			float step = (2 * 3.141592654f) / (float)(MAX(1, item_count));
			float a_min = 0; float a_max = step;

			struct nk_vec2 center = nk_vec2(bounds.x + bounds.w / 2.0f, bounds.y + bounds.h / 2.0f);
			struct nk_vec2 drag = nk_vec2(in->mouse.pos.x - center.x, in->mouse.pos.y - center.y);
			float angle = (float)atan2(drag.y, drag.x);
			if (angle < -0.0f) angle += 2.0f * 3.141592654f;
			active_item = (int)(angle / step);

			for (i = 0; i < item_count; ++i) {
				struct nk_rect content;
				float rx, ry, dx, dy, a;
				nk_fill_arc(out, center.x, center.y, (bounds.w / 2.0f),
					a_min, a_max, (active_item == i) ? nk_rgb(45, 100, 255) : nk_rgb(60, 60, 60));

				/* separator line */
				rx = bounds.w / 2.0f; ry = 0;
				dx = rx * (float)cos(a_min) - ry * (float)sin(a_min);
				dy = rx * (float)sin(a_min) + ry * (float)cos(a_min);
				nk_stroke_line(out, center.x, center.y,
					center.x + dx, center.y + dy, 1.0f, nk_rgb(50, 50, 50));

				/* button content */
				a = a_min + (a_max - a_min) / 2.0f;
				rx = bounds.w / 2.5f; ry = 0;
				content.w = 30; content.h = 30;
				content.x = center.x + ((rx * (float)cos(a) - ry * (float)sin(a)) - content.w / 2.0f);
				content.y = center.y + (rx * (float)sin(a) + ry * (float)cos(a) - content.h / 2.0f);
				nk_draw_image(out, content, &icons[i], nk_rgb(255, 255, 255));
				a_min = a_max; a_max += step;
			}
		}
		{
			/* inner circle */
			struct nk_rect inner;
			inner.x = bounds.x + bounds.w / 2 - bounds.w / 4;
			inner.y = bounds.y + bounds.h / 2 - bounds.h / 4;
			inner.w = bounds.w / 2; inner.h = bounds.h / 2;
			nk_fill_circle(out, inner, nk_rgb(45, 45, 45));

			/* active icon content */
			bounds.w = inner.w / 2.0f;
			bounds.h = inner.h / 2.0f;
			bounds.x = inner.x + inner.w / 2 - bounds.w / 2;
			bounds.y = inner.y + inner.h / 2 - bounds.h / 2;
			nk_draw_image(out, bounds, &icons[active_item], nk_rgb(255, 255, 255));
		}
		nk_layout_space_end(ctx);
		if (!nk_input_is_mouse_down(&ctx->input, NK_BUTTON_RIGHT)) {
			nk_popup_close(ctx);
			ret = active_item;
		}
	}
	else ret = -2;
	ctx->style.window.spacing = nk_vec2(4, 4);
	ctx->style.window.padding = nk_vec2(8, 8);
	nk_popup_end(ctx);

	ctx->style.window.fixed_background = background;
	ctx->style.window.border_color = border;
	return ret;
}

/* ===============================================================
 *
 *                          GRID
 *
 * ===============================================================*/
static void
grid_demo(struct nk_context *ctx, struct media *media)
{
	static char text[3][64];
	static int text_len[3];
	static const char *items[] = { "Item 0","item 1","item 2" };
	static int selected_item = 0;
	static int check = 1;

	int i;
	nk_style_set_font(ctx, &media->font_20->handle);
	if (nk_begin(ctx, "Grid Demo", nk_rect(600, 350, 275, 250),
		NK_WINDOW_TITLE | NK_WINDOW_BORDER | NK_WINDOW_MOVABLE |
		NK_WINDOW_NO_SCROLLBAR))
	{
		nk_style_set_font(ctx, &media->font_18->handle);
		nk_layout_row_dynamic(ctx, 30, 2);
		nk_label(ctx, "Floating point:", NK_TEXT_RIGHT);
		nk_edit_string(ctx, NK_EDIT_FIELD, text[0], &text_len[0], 64, nk_filter_float);
		nk_label(ctx, "Hexadecimal:", NK_TEXT_RIGHT);
		nk_edit_string(ctx, NK_EDIT_FIELD, text[1], &text_len[1], 64, nk_filter_hex);
		nk_label(ctx, "Binary:", NK_TEXT_RIGHT);
		nk_edit_string(ctx, NK_EDIT_FIELD, text[2], &text_len[2], 64, nk_filter_binary);
		nk_label(ctx, "Checkbox:", NK_TEXT_RIGHT);
		nk_checkbox_label(ctx, "Check me", &check);
		nk_label(ctx, "Combobox:", NK_TEXT_RIGHT);
		if (nk_combo_begin_label(ctx, items[selected_item], nk_vec2(nk_widget_width(ctx), 200))) {
			nk_layout_row_dynamic(ctx, 25, 1);
			for (i = 0; i < 3; ++i)
				if (nk_combo_item_label(ctx, items[i], NK_TEXT_LEFT))
					selected_item = i;
			nk_combo_end(ctx);
		}
	}
	nk_end(ctx);
	nk_style_set_font(ctx, &media->font_14->handle);
}

/* ===============================================================
 *
 *                          BUTTON DEMO
 *
 * ===============================================================*/
static void
ui_header(struct nk_context *ctx, struct media *media, const char *title)
{
	nk_style_set_font(ctx, &media->font_18->handle);
	nk_layout_row_dynamic(ctx, 20, 1);
	nk_label(ctx, title, NK_TEXT_LEFT);
}

static void
ui_widget(struct nk_context *ctx, struct media *media, float height)
{
	static const float ratio[] = { 0.15f, 0.85f };
	nk_style_set_font(ctx, &media->font_22->handle);
	nk_layout_row(ctx, NK_DYNAMIC, height, 2, ratio);
	nk_spacing(ctx, 1);
}

static void
ui_widget_centered(struct nk_context *ctx, struct media *media, float height)
{
	static const float ratio[] = { 0.15f, 0.50f, 0.35f };
	nk_style_set_font(ctx, &media->font_22->handle);
	nk_layout_row(ctx, NK_DYNAMIC, height, 3, ratio);
	nk_spacing(ctx, 1);
}

static void
button_demo(struct nk_context *ctx, struct media *media)
{
	static int option = 1;
	static int toggle0 = 1;
	static int toggle1 = 0;
	static int toggle2 = 1;

	nk_style_set_font(ctx, &media->font_20->handle);
	nk_begin(ctx, "Button Demo", nk_rect(50, 50, 255, 610),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE);

	/*------------------------------------------------
	 *                  MENU
	 *------------------------------------------------*/
	nk_menubar_begin(ctx);
	{
		/* toolbar */
		nk_layout_row_static(ctx, 40, 40, 4);
		if (nk_menu_begin_image(ctx, "Music", media->play, nk_vec2(110, 120)))
		{
			/* settings */
			nk_layout_row_dynamic(ctx, 25, 1);
			nk_menu_item_image_label(ctx, media->play, "Play", NK_TEXT_RIGHT);
			nk_menu_item_image_label(ctx, media->stop, "Stop", NK_TEXT_RIGHT);
			nk_menu_item_image_label(ctx, media->pause, "Pause", NK_TEXT_RIGHT);
			nk_menu_item_image_label(ctx, media->next, "Next", NK_TEXT_RIGHT);
			nk_menu_item_image_label(ctx, media->prev, "Prev", NK_TEXT_RIGHT);
			nk_menu_end(ctx);
		}
		nk_button_image(ctx, media->tools);
		nk_button_image(ctx, media->cloud);
		nk_button_image(ctx, media->pen);
	}
	nk_menubar_end(ctx);

	/*------------------------------------------------
	 *                  BUTTON
	 *------------------------------------------------*/
	ui_header(ctx, media, "Push buttons");
	ui_widget(ctx, media, 35);
	if (nk_button_label(ctx, "Push me"))
		fprintf(stdout, "pushed!\n");
	ui_widget(ctx, media, 35);
	if (nk_button_image_label(ctx, media->rocket, "Styled", NK_TEXT_CENTERED))
		fprintf(stdout, "rocket!\n");

	/*------------------------------------------------
	 *                  REPEATER
	 *------------------------------------------------*/
	ui_header(ctx, media, "Repeater");
	ui_widget(ctx, media, 35);
	if (nk_button_label(ctx, "Press me"))
		fprintf(stdout, "pressed!\n");

	/*------------------------------------------------
	 *                  TOGGLE
	 *------------------------------------------------*/
	ui_header(ctx, media, "Toggle buttons");
	ui_widget(ctx, media, 35);
	if (nk_button_image_label(ctx, (toggle0) ? media->checked : media->unchecked, "Toggle", NK_TEXT_LEFT))
		toggle0 = !toggle0;

	ui_widget(ctx, media, 35);
	if (nk_button_image_label(ctx, (toggle1) ? media->checked : media->unchecked, "Toggle", NK_TEXT_LEFT))
		toggle1 = !toggle1;

	ui_widget(ctx, media, 35);
	if (nk_button_image_label(ctx, (toggle2) ? media->checked : media->unchecked, "Toggle", NK_TEXT_LEFT))
		toggle2 = !toggle2;

	/*------------------------------------------------
	 *                  RADIO
	 *------------------------------------------------*/
	ui_header(ctx, media, "Radio buttons");
	ui_widget(ctx, media, 35);
	if (nk_button_symbol_label(ctx, (option == 0) ? NK_SYMBOL_CIRCLE_OUTLINE : NK_SYMBOL_CIRCLE_SOLID, "Select", NK_TEXT_LEFT))
		option = 0;
	ui_widget(ctx, media, 35);
	if (nk_button_symbol_label(ctx, (option == 1) ? NK_SYMBOL_CIRCLE_OUTLINE : NK_SYMBOL_CIRCLE_SOLID, "Select", NK_TEXT_LEFT))
		option = 1;
	ui_widget(ctx, media, 35);
	if (nk_button_symbol_label(ctx, (option == 2) ? NK_SYMBOL_CIRCLE_OUTLINE : NK_SYMBOL_CIRCLE_SOLID, "Select", NK_TEXT_LEFT))
		option = 2;

	/*------------------------------------------------
	 *                  CONTEXTUAL
	 *------------------------------------------------*/
	nk_style_set_font(ctx, &media->font_18->handle);
	if (nk_contextual_begin(ctx, NK_WINDOW_NO_SCROLLBAR, nk_vec2(150, 300), nk_window_get_bounds(ctx))) {
		nk_layout_row_dynamic(ctx, 30, 1);
		if (nk_contextual_item_image_label(ctx, media->copy, "Clone", NK_TEXT_RIGHT))
			fprintf(stdout, "pressed clone!\n");
		if (nk_contextual_item_image_label(ctx, media->del, "Delete", NK_TEXT_RIGHT))
			fprintf(stdout, "pressed delete!\n");
		if (nk_contextual_item_image_label(ctx, media->convert, "Convert", NK_TEXT_RIGHT))
			fprintf(stdout, "pressed convert!\n");
		if (nk_contextual_item_image_label(ctx, media->edit, "Edit", NK_TEXT_RIGHT))
			fprintf(stdout, "pressed edit!\n");
		nk_contextual_end(ctx);
	}
	nk_style_set_font(ctx, &media->font_14->handle);
	nk_end(ctx);
}

/* ===============================================================
 *
 *                          BASIC DEMO
 *
 * ===============================================================*/
static  void
kill_layout(struct nk_context *ctx, struct media *media) {
	if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
		NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
	{
		enum { EASY, HARD };
		static int op = EASY;
		static int property = 20;
		nk_layout_row_static(ctx, 30, 80, 1);
		if (nk_button_label(ctx, "button"))
			fprintf(stdout, "button pressed\n");

		nk_layout_row_dynamic(ctx, 30, 2);
		if (nk_option_label(ctx, "easy", op == EASY)) op = EASY;
		if (nk_option_label(ctx, "hard", op == HARD)) op = HARD;

		nk_layout_row_dynamic(ctx, 25, 1);
		nk_property_int(ctx, "Compression:", 0, &property, 100, 10, 1);

		nk_layout_row_dynamic(ctx, 20, 1);
		nk_label(ctx, "background:", NK_TEXT_LEFT);
		nk_layout_row_dynamic(ctx, 25, 1);
	}
	nk_end(ctx);
}

static  void
lobby_layout(struct nk_context *ctx, struct media *media, int width, int height ) {
	if (nk_begin(ctx, "Lobby", nk_rect(0, 0, width, height),
		NK_WINDOW_BORDER //| NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
		//NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE
	))
	{
		enum { HUMAN, MAGE, ASSASIN, WARRIOR,KING };
		static int op = HUMAN;
		static const float ratio[] = { 0.2f, 0.6f, 0.2f };  /* 0.2 + 0.6 + 0.2 = 1 */
		nk_layout_row_static(ctx, (1 - 0.15*4)*height / 2, 15, 1);
		nk_layout_row_dynamic(ctx, height *0.1, 5);
		nk_spacing(ctx, 1);
		if (nk_option_label(ctx, "HUMAN", op == HUMAN)) op = HUMAN;
		if (nk_option_label(ctx, "MAGE", op == MAGE)) op = MAGE;
		if (nk_option_label(ctx, "ASSASIN", op == ASSASIN)) op = ASSASIN;
		if (nk_option_label(ctx, "WARRIOR", op == WARRIOR)) op = WARRIOR;

		//horizontal centered
		nk_layout_row(ctx, NK_DYNAMIC,height *0.1, 3,ratio);
		nk_spacing(ctx, 1);
		if (nk_button_label(ctx, "Confirm"))
			fprintf(stdout, "button pressed\n");
		nk_spacing(ctx, 1);

	}
	nk_end(ctx);
}

static void
basic_demo(struct nk_context *ctx, struct media *media)
{
	static int image_active;
	static int check0 = 1;
	static int check1 = 0;
	static size_t prog = 80;
	static int selected_item = 0;
	static int selected_image = 3;
	static int selected_icon = 0;
	static const char *items[] = { "Item 0","item 1","item 2" };
	static int piemenu_active = 1;
	static struct nk_vec2 piemenu_pos;

	int i = 0;
	nk_style_set_font(ctx, &media->font_20->handle);
	nk_begin(ctx, "Basic Demo", nk_rect(320, 50, 275, 610),
		NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_TITLE);

	/*------------------------------------------------
	 *                  POPUP BUTTON
	 *------------------------------------------------*/
	ui_header(ctx, media, "Popup & Scrollbar & Images");
	ui_widget(ctx, media, 35);
	if (nk_button_image_label(ctx, media->dir, "Images", NK_TEXT_CENTERED))
		image_active = !image_active;

	/*------------------------------------------------
	 *                  SELECTED IMAGE
	 *------------------------------------------------*/
	ui_header(ctx, media, "Selected Image");
	ui_widget_centered(ctx, media, 100);
	nk_image(ctx, media->images[selected_image]);

	/*------------------------------------------------
	 *                  IMAGE POPUP
	 *------------------------------------------------*/
	if (image_active) {
		struct nk_panel popup;
		if (nk_popup_begin(ctx, NK_POPUP_STATIC, "Image Popup", 0, nk_rect(265, 0, 320, 220))) {
			nk_layout_row_static(ctx, 82, 82, 3);
			for (i = 0; i < 9; ++i) {
				if (nk_button_image(ctx, media->images[i])) {
					selected_image = i;
					image_active = 0;
					nk_popup_close(ctx);
				}
			}
			nk_popup_end(ctx);
		}
	}
	/*------------------------------------------------
	 *                  COMBOBOX
	 *------------------------------------------------*/
	ui_header(ctx, media, "Combo box");
	ui_widget(ctx, media, 80);
	if (nk_combo_begin_label(ctx, items[selected_item], nk_vec2(nk_widget_width(ctx), 200))) {
		nk_layout_row_dynamic(ctx, 35, 1);
		for (i = 0; i < 3; ++i)
			if (nk_combo_item_label(ctx, items[i], NK_TEXT_LEFT))
				selected_item = i;
		nk_combo_end(ctx);
	}

	ui_widget(ctx, media, 40);
	if (nk_combo_begin_image_label(ctx, items[selected_icon], media->images[selected_icon], nk_vec2(nk_widget_width(ctx), 200))) {
		nk_layout_row_dynamic(ctx, 35, 1);
		for (i = 0; i < 3; ++i)
			if (nk_combo_item_image_label(ctx, media->images[i], items[i], NK_TEXT_RIGHT))
				selected_icon = i;
		nk_combo_end(ctx);
	}

	/*------------------------------------------------
	 *                  CHECKBOX
	 *------------------------------------------------*/
	ui_header(ctx, media, "Checkbox");
	ui_widget(ctx, media, 60);
	nk_checkbox_label(ctx, "Flag 1", &check0);
	ui_widget(ctx, media, 30);
	nk_checkbox_label(ctx, "Flag 2", &check1);

	/*------------------------------------------------
	 *                  PROGRESSBAR
	 *------------------------------------------------*/
	ui_header(ctx, media, "Progressbar");
	ui_widget(ctx, media, 35);
	nk_progress(ctx, &prog, 100, nk_true);

	/*------------------------------------------------
	 *                  PIEMENU
	 *------------------------------------------------*/
	if (nk_input_is_mouse_click_down_in_rect(&ctx->input, NK_BUTTON_RIGHT,
		nk_window_get_bounds(ctx), nk_true)) {
		piemenu_pos = ctx->input.mouse.pos;
		piemenu_active = 1;
	}

	if (piemenu_active) {
		int ret = ui_piemenu(ctx, piemenu_pos, 140, &media->menu[0], 6);
		if (ret == -2) piemenu_active = 0;
		if (ret != -1) {
			fprintf(stdout, "piemenu selected: %d\n", ret);
			piemenu_active = 0;
		}
	}
	nk_style_set_font(ctx, &media->font_14->handle);
	nk_end(ctx);
}

/*
 * ==============================================================
 *
 *                          IMPLEMENTATION
 *
 * ===============================================================
 */
#ifdef NK_GLFW_GL3_IMPLEMENTATION

#ifndef NK_GLFW_TEXT_MAX
#define NK_GLFW_TEXT_MAX 256
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_LO
#define NK_GLFW_DOUBLE_CLICK_LO 0.02
#endif
#ifndef NK_GLFW_DOUBLE_CLICK_HI
#define NK_GLFW_DOUBLE_CLICK_HI 0.2
#endif

struct nk_glfw_device {
    struct nk_buffer cmds;
    struct nk_draw_null_texture null;
    GLuint vbo, vao, ebo;
    GLuint prog;
    GLuint vert_shdr;
    GLuint frag_shdr;
    GLint attrib_pos;
    GLint attrib_uv;
    GLint attrib_col;
    GLint uniform_tex;
    GLint uniform_proj;
    GLuint font_tex;
};

struct nk_glfw_vertex {
    float position[2];
    float uv[2];
    nk_byte col[4];
};

static struct nk_glfw {
    GLFWwindow *win;
    int width, height;
    int display_width, display_height;
    struct nk_glfw_device ogl;
    struct nk_context ctx;
    struct nk_font_atlas atlas;
    struct nk_vec2 fb_scale;
    unsigned int text[NK_GLFW_TEXT_MAX];
    int text_len;
    struct nk_vec2 scroll;
    double last_button_click;
    int is_double_click_down;
    struct nk_vec2 double_click_pos;
} glfw;

#ifdef __APPLE__
  #define NK_SHADER_VERSION "#version 150\n"
#else
  #define NK_SHADER_VERSION "#version 300 es\n"
#endif

NK_API void
nk_glfw3_device_create(void)
{
    GLint status;
    static const GLchar *vertex_shader =
        NK_SHADER_VERSION
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 TexCoord;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main() {\n"
        "   Frag_UV = TexCoord;\n"
        "   Frag_Color = Color;\n"
        "   gl_Position = ProjMtx * vec4(Position.xy, 0, 1);\n"
        "}\n";
    static const GLchar *fragment_shader =
        NK_SHADER_VERSION
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main(){\n"
        "   Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    struct nk_glfw_device *dev = &glfw.ogl;
    nk_buffer_init_default(&dev->cmds);
    dev->prog = glCreateProgram();
    dev->vert_shdr = glCreateShader(GL_VERTEX_SHADER);
    dev->frag_shdr = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(dev->vert_shdr, 1, &vertex_shader, 0);
    glShaderSource(dev->frag_shdr, 1, &fragment_shader, 0);
    glCompileShader(dev->vert_shdr);
    glCompileShader(dev->frag_shdr);
    glGetShaderiv(dev->vert_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glGetShaderiv(dev->frag_shdr, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);
    glAttachShader(dev->prog, dev->vert_shdr);
    glAttachShader(dev->prog, dev->frag_shdr);
    glLinkProgram(dev->prog);
    glGetProgramiv(dev->prog, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);

    dev->uniform_tex = glGetUniformLocation(dev->prog, "Texture");
    dev->uniform_proj = glGetUniformLocation(dev->prog, "ProjMtx");
    dev->attrib_pos = glGetAttribLocation(dev->prog, "Position");
    dev->attrib_uv = glGetAttribLocation(dev->prog, "TexCoord");
    dev->attrib_col = glGetAttribLocation(dev->prog, "Color");

    {
        /* buffer setup */
        GLsizei vs = sizeof(struct nk_glfw_vertex);
        size_t vp = offsetof(struct nk_glfw_vertex, position);
        size_t vt = offsetof(struct nk_glfw_vertex, uv);
        size_t vc = offsetof(struct nk_glfw_vertex, col);

        glGenBuffers(1, &dev->vbo);
        glGenBuffers(1, &dev->ebo);
        glGenVertexArrays(1, &dev->vao);

        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glEnableVertexAttribArray((GLuint)dev->attrib_pos);
        glEnableVertexAttribArray((GLuint)dev->attrib_uv);
        glEnableVertexAttribArray((GLuint)dev->attrib_col);

        glVertexAttribPointer((GLuint)dev->attrib_pos, 2, GL_FLOAT, GL_FALSE, vs, (void*)vp);
        glVertexAttribPointer((GLuint)dev->attrib_uv, 2, GL_FLOAT, GL_FALSE, vs, (void*)vt);
        glVertexAttribPointer((GLuint)dev->attrib_col, 4, GL_UNSIGNED_BYTE, GL_TRUE, vs, (void*)vc);
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

NK_INTERN void
nk_glfw3_device_upload_atlas(const void *image, int width, int height)
{
    struct nk_glfw_device *dev = &glfw.ogl;
    glGenTextures(1, &dev->font_tex);
    glBindTexture(GL_TEXTURE_2D, dev->font_tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (GLsizei)width, (GLsizei)height, 0,
                GL_RGBA, GL_UNSIGNED_BYTE, image);
}

NK_API void
nk_glfw3_device_destroy(void)
{
    struct nk_glfw_device *dev = &glfw.ogl;
    glDetachShader(dev->prog, dev->vert_shdr);
    glDetachShader(dev->prog, dev->frag_shdr);
    glDeleteShader(dev->vert_shdr);
    glDeleteShader(dev->frag_shdr);
    glDeleteProgram(dev->prog);
    glDeleteTextures(1, &dev->font_tex);
    glDeleteBuffers(1, &dev->vbo);
    glDeleteBuffers(1, &dev->ebo);
    nk_buffer_free(&dev->cmds);
}

NK_API void
nk_glfw3_render(enum nk_anti_aliasing AA, int max_vertex_buffer, int max_element_buffer)
{
    struct nk_glfw_device *dev = &glfw.ogl;
    struct nk_buffer vbuf, ebuf;
    GLfloat ortho[4][4] = {
        {2.0f, 0.0f, 0.0f, 0.0f},
        {0.0f,-2.0f, 0.0f, 0.0f},
        {0.0f, 0.0f,-1.0f, 0.0f},
        {-1.0f,1.0f, 0.0f, 1.0f},
    };
    ortho[0][0] /= (GLfloat)glfw.width;
    ortho[1][1] /= (GLfloat)glfw.height;

    /* setup global state */
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
    glActiveTexture(GL_TEXTURE0);

    /* setup program */
    glUseProgram(dev->prog);
    glUniform1i(dev->uniform_tex, 0);
    glUniformMatrix4fv(dev->uniform_proj, 1, GL_FALSE, &ortho[0][0]);
    glViewport(0,0,(GLsizei)glfw.display_width,(GLsizei)glfw.display_height);
    {
        /* convert from command queue into draw list and draw to screen */
        const struct nk_draw_command *cmd;
        void *vertices, *elements;
        const nk_draw_index *offset = NULL;

        /* allocate vertex and element buffer */
        glBindVertexArray(dev->vao);
        glBindBuffer(GL_ARRAY_BUFFER, dev->vbo);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dev->ebo);

        glBufferData(GL_ARRAY_BUFFER, max_vertex_buffer, NULL, GL_STREAM_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, max_element_buffer, NULL, GL_STREAM_DRAW);

        /* load draw vertices & elements directly into vertex + element buffer */
        vertices = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
        elements = glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY);
        {
            /* fill convert configuration */
            struct nk_convert_config config;
            static const struct nk_draw_vertex_layout_element vertex_layout[] = {
                {NK_VERTEX_POSITION, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, position)},
                {NK_VERTEX_TEXCOORD, NK_FORMAT_FLOAT, NK_OFFSETOF(struct nk_glfw_vertex, uv)},
                {NK_VERTEX_COLOR, NK_FORMAT_R8G8B8A8, NK_OFFSETOF(struct nk_glfw_vertex, col)},
                {NK_VERTEX_LAYOUT_END}
            };
            NK_MEMSET(&config, 0, sizeof(config));
            config.vertex_layout = vertex_layout;
            config.vertex_size = sizeof(struct nk_glfw_vertex);
            config.vertex_alignment = NK_ALIGNOF(struct nk_glfw_vertex);
            config.null = dev->null;
            config.circle_segment_count = 22;
            config.curve_segment_count = 22;
            config.arc_segment_count = 22;
            config.global_alpha = 1.0f;
            config.shape_AA = AA;
            config.line_AA = AA;

            /* setup buffers to load vertices and elements */
            nk_buffer_init_fixed(&vbuf, vertices, (size_t)max_vertex_buffer);
            nk_buffer_init_fixed(&ebuf, elements, (size_t)max_element_buffer);
            nk_convert(&glfw.ctx, &dev->cmds, &vbuf, &ebuf, &config);
        }
        glUnmapBuffer(GL_ARRAY_BUFFER);
        glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);

        /* iterate over and execute each draw command */
        nk_draw_foreach(cmd, &glfw.ctx, &dev->cmds)
        {
            if (!cmd->elem_count) continue;
            glBindTexture(GL_TEXTURE_2D, (GLuint)cmd->texture.id);
            glScissor(
                (GLint)(cmd->clip_rect.x * glfw.fb_scale.x),
                (GLint)((glfw.height - (GLint)(cmd->clip_rect.y + cmd->clip_rect.h)) * glfw.fb_scale.y),
                (GLint)(cmd->clip_rect.w * glfw.fb_scale.x),
                (GLint)(cmd->clip_rect.h * glfw.fb_scale.y));
            glDrawElements(GL_TRIANGLES, (GLsizei)cmd->elem_count, GL_UNSIGNED_SHORT, offset);
            offset += cmd->elem_count;
        }
        nk_clear(&glfw.ctx);
    }

    /* default OpenGL state */
    glUseProgram(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
	glEnable(GL_DEPTH_TEST);
	// Related to shaders and z value comparisons for the depth buffer
	glDepthFunc(GL_LEQUAL);
	// Set polygon drawing mode to fill front and back of each polygon
	// You can also use the paramter of GL_LINE instead of GL_FILL to see wireframes
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
}

NK_API void
nk_glfw3_char_callback(GLFWwindow *win, unsigned int codepoint)
{
    (void)win;
    if (glfw.text_len < NK_GLFW_TEXT_MAX)
        glfw.text[glfw.text_len++] = codepoint;
}

NK_API void text_input(GLFWwindow *win, unsigned int codepoint)
{
	nk_input_unicode((struct nk_context*)glfwGetWindowUserPointer(win), codepoint);
}

NK_API void
nk_gflw3_scroll_callback(GLFWwindow *win, double xoff, double yoff)
{
    (void)win; (void)xoff;
    glfw.scroll.x += (float)xoff;
    glfw.scroll.y += (float)yoff;
}

NK_API void
nk_glfw3_mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    double x, y;
    if (button != GLFW_MOUSE_BUTTON_LEFT) return;
    glfwGetCursorPos(window, &x, &y);
    if (action == GLFW_PRESS)  {
        double dt = glfwGetTime() - glfw.last_button_click;
        if (dt > NK_GLFW_DOUBLE_CLICK_LO && dt < NK_GLFW_DOUBLE_CLICK_HI) {
            glfw.is_double_click_down = nk_true;
            glfw.double_click_pos = nk_vec2((float)x, (float)y);
        }
        glfw.last_button_click = glfwGetTime();
    } else glfw.is_double_click_down = nk_false;
}

NK_INTERN void
nk_glfw3_clipboard_paste(nk_handle usr, struct nk_text_edit *edit)
{
    const char *text = glfwGetClipboardString(glfw.win);
    if (text) nk_textedit_paste(edit, text, nk_strlen(text));
    (void)usr;
}

NK_INTERN void
nk_glfw3_clipboard_copy(nk_handle usr, const char *text, int len)
{
    char *str = 0;
    (void)usr;
    if (!len) return;
    str = (char*)malloc((size_t)len+1);
    if (!str) return;
    memcpy(str, text, (size_t)len);
    str[len] = '\0';
    glfwSetClipboardString(glfw.win, str);
    free(str);
}

NK_API struct nk_context*
nk_glfw3_init(GLFWwindow *win, enum nk_glfw_init_state init_state)
{
    glfw.win = win;
    if (init_state == NK_GLFW3_INSTALL_CALLBACKS) {
		//glfwSetCharCallback(win, text_input);
        glfwSetScrollCallback(win, nk_gflw3_scroll_callback);
        glfwSetCharCallback(win, nk_glfw3_char_callback);
        glfwSetMouseButtonCallback(win, nk_glfw3_mouse_button_callback);
    }
    nk_init_default(&glfw.ctx, 0);
    glfw.ctx.clip.copy = nk_glfw3_clipboard_copy;
    glfw.ctx.clip.paste = nk_glfw3_clipboard_paste;
    glfw.ctx.clip.userdata = nk_handle_ptr(0);
    glfw.last_button_click = 0;
    nk_glfw3_device_create();

    glfw.is_double_click_down = nk_false;
    glfw.double_click_pos = nk_vec2(0, 0);

    return &glfw.ctx;
}

NK_API void
nk_glfw3_font_stash_begin(struct nk_font_atlas **atlas)
{
    nk_font_atlas_init_default(&glfw.atlas);
    nk_font_atlas_begin(&glfw.atlas);
    *atlas = &glfw.atlas;
}

NK_API void
nk_glfw3_font_stash_end(void)
{
    const void *image; int w, h;
    image = nk_font_atlas_bake(&glfw.atlas, &w, &h, NK_FONT_ATLAS_RGBA32);
    nk_glfw3_device_upload_atlas(image, w, h);
    nk_font_atlas_end(&glfw.atlas, nk_handle_id((int)glfw.ogl.font_tex), &glfw.ogl.null);
    if (glfw.atlas.default_font)
        nk_style_set_font(&glfw.ctx, &glfw.atlas.default_font->handle);
}

NK_API void
nk_glfw3_new_frame(void)
{
    int i;
    double x, y;
    struct nk_context *ctx = &glfw.ctx;
    struct GLFWwindow *win = glfw.win;

    glfwGetWindowSize(win, &glfw.width, &glfw.height);
    glfwGetFramebufferSize(win, &glfw.display_width, &glfw.display_height);
    glfw.fb_scale.x = (float)glfw.display_width/(float)glfw.width;
    glfw.fb_scale.y = (float)glfw.display_height/(float)glfw.height;

    nk_input_begin(ctx);
    for (i = 0; i < glfw.text_len; ++i)
        nk_input_unicode(ctx, glfw.text[i]);

#ifdef NK_GLFW_GL3_MOUSE_GRABBING
    /* optional grabbing behavior */
    if (ctx->input.mouse.grab)
        glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
    else if (ctx->input.mouse.ungrab)
        glfwSetInputMode(glfw.win, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
#endif

    nk_input_key(ctx, NK_KEY_DEL, glfwGetKey(win, GLFW_KEY_DELETE) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_ENTER, glfwGetKey(win, GLFW_KEY_ENTER) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TAB, glfwGetKey(win, GLFW_KEY_TAB) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_BACKSPACE, glfwGetKey(win, GLFW_KEY_BACKSPACE) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_UP, glfwGetKey(win, GLFW_KEY_UP) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_DOWN, glfwGetKey(win, GLFW_KEY_DOWN) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_START, glfwGetKey(win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_TEXT_END, glfwGetKey(win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_START, glfwGetKey(win, GLFW_KEY_HOME) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_END, glfwGetKey(win, GLFW_KEY_END) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_DOWN, glfwGetKey(win, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SCROLL_UP, glfwGetKey(win, GLFW_KEY_PAGE_UP) == GLFW_PRESS);
    nk_input_key(ctx, NK_KEY_SHIFT, glfwGetKey(win, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS||
                                    glfwGetKey(win, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS);

    if (glfwGetKey(win, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS ||
        glfwGetKey(win, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
        nk_input_key(ctx, NK_KEY_COPY, glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_PASTE, glfwGetKey(win, GLFW_KEY_V) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_CUT, glfwGetKey(win, GLFW_KEY_X) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_UNDO, glfwGetKey(win, GLFW_KEY_Z) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_REDO, glfwGetKey(win, GLFW_KEY_R) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_LEFT, glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_WORD_RIGHT, glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_START, glfwGetKey(win, GLFW_KEY_B) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_TEXT_LINE_END, glfwGetKey(win, GLFW_KEY_E) == GLFW_PRESS);
    } else {
        nk_input_key(ctx, NK_KEY_LEFT, glfwGetKey(win, GLFW_KEY_LEFT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_RIGHT, glfwGetKey(win, GLFW_KEY_RIGHT) == GLFW_PRESS);
        nk_input_key(ctx, NK_KEY_COPY, 0);
        nk_input_key(ctx, NK_KEY_PASTE, 0);
        nk_input_key(ctx, NK_KEY_CUT, 0);
        nk_input_key(ctx, NK_KEY_SHIFT, 0);
    }

    glfwGetCursorPos(win, &x, &y);
    nk_input_motion(ctx, (int)x, (int)y);
#ifdef NK_GLFW_GL3_MOUSE_GRABBING
    if (ctx->input.mouse.grabbed) {
        glfwSetCursorPos(glfw.win, ctx->input.mouse.prev.x, ctx->input.mouse.prev.y);
        ctx->input.mouse.pos.x = ctx->input.mouse.prev.x;
        ctx->input.mouse.pos.y = ctx->input.mouse.prev.y;
    }
#endif
    nk_input_button(ctx, NK_BUTTON_LEFT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_MIDDLE, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_RIGHT, (int)x, (int)y, glfwGetMouseButton(win, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS);
    nk_input_button(ctx, NK_BUTTON_DOUBLE, (int)glfw.double_click_pos.x, (int)glfw.double_click_pos.y, glfw.is_double_click_down);
    nk_input_scroll(ctx, glfw.scroll);
    nk_input_end(&glfw.ctx);
    glfw.text_len = 0;
    glfw.scroll = nk_vec2(0,0);
}

NK_API
void nk_glfw3_shutdown(void)
{
    nk_font_atlas_clear(&glfw.atlas);
    nk_free(&glfw.ctx);
    nk_glfw3_device_destroy();
    memset(&glfw, 0, sizeof(glfw));
}

#endif
