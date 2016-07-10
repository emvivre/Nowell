/* This file is part of NOWELL.
 *
 * NOWELL is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * NOWELL is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with NOWELL.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "game.h"
#include "menu_title.h"

#define MENU_TITLE_MUSIC "menu.ogg"
#define MENU_TITLE_NEW_GAME_IMAGE "b_newgame.bmp"
#define MENU_TITLE_NEW_GAME_TOUCH_IMAGE "b_2newgame.bmp"
#define MENU_TITLE_NEW_GAME_X 210
#define MENU_TITLE_NEW_GAME_Y 90
#define MENU_TITLE_TRAINING_IMAGE "b_tutorial.bmp"
#define MENU_TITLE_TRAINING_TOUCH_IMAGE "b_2tutorial.bmp"
#define MENU_TITLE_TRAINING_X 210
#define MENU_TITLE_TRAINING_Y 300
#define CURSOR_INIT_X 100
#define CURSOR_INIT_Y 200
#define CURSOR_STRIDE_X 2
#define CURSOR_STRIDE_Y 2

void *font_menu_title;
static surface_t *msg_newgame; 
static surface_t *msg_training;
static surface_t *img_back;
static surface_t *cursor;
static int cursor_x, cursor_y;
static int newgame_x, newgame_y;
static surface_t *newgame_zoom;
static surface_t *newgame_no_zoom;
static int training_x, training_y;
static surface_t *training_zoom;
static surface_t *training_no_zoom;
static void *menu_title_music;

static void menu_title_music_hook(void)
{
	stuff_music_play(menu_title_music, 0, menu_title_music_hook);
}


void menu_title_init(void)
{
	char path[PATH_LENGTH_MAX];

	// initialize current univer to menu title
	univers_current.handle_player = menu_title_handle_player;
	univers_current.compute_physic = menu_title_compute_physic;
	univers_current.render_scene = menu_title_render_scene;
	univers_current.finalize = menu_title_finalize;
	
	// allocate stuff
	img_back = stuff_load_img(MENU_TITLE_IMAGE);	
	cursor = stuff_load_img(MENU_TITLE_CURSOR);
	cursor_x = CURSOR_INIT_X;
	cursor_y = CURSOR_INIT_Y;

	// msg newgame
	sprintf(path, "%s%s", IMAGE_PATH, MENU_TITLE_NEW_GAME_IMAGE);
	newgame_no_zoom = stuff_load_img(path);
	newgame_x = MENU_TITLE_NEW_GAME_X;
	newgame_y = MENU_TITLE_NEW_GAME_Y;
	msg_newgame = newgame_no_zoom; 
	sprintf(path, "%s%s", IMAGE_PATH, MENU_TITLE_NEW_GAME_TOUCH_IMAGE); 
	newgame_zoom = stuff_load_img(path);

	// msg training
	sprintf(path, "%s%s", IMAGE_PATH, MENU_TITLE_TRAINING_IMAGE); 
	training_no_zoom = stuff_load_img(path);
	training_x = MENU_TITLE_TRAINING_X;
	training_y = MENU_TITLE_TRAINING_Y;		
	msg_training = training_no_zoom;
	sprintf(path, "%s%s", IMAGE_PATH, MENU_TITLE_TRAINING_TOUCH_IMAGE); 
	training_zoom = stuff_load_img(path);
	
	// load music
	sprintf(path, "%s%s", MUSIC_PATH, MENU_TITLE_MUSIC);
	menu_title_music = stuff_music_load(path);
	stuff_music_play(menu_title_music, 0, menu_title_music_hook);			
}

void menu_title_finalize(void)
{
	// free stuff	
	stuff_free_surface(img_back);	
	stuff_free_surface(cursor);	
	stuff_free_surface(newgame_zoom);	
	stuff_free_surface(newgame_no_zoom);	
	stuff_free_surface(training_zoom);	
	stuff_free_surface(training_no_zoom);	
	stuff_font_unload(font_menu_title);       	
}

static void menu_title_stop_music(void)
{
	stuff_music_stop(menu_title_music);
}

void menu_title_handle_player(void)
{	
	// with keyboard
	if (buttons[BUTTON_ESCAPE] == BUTTON_PRESSED) {
		buttons[BUTTON_ESCAPE] = BUTTON_NO_PRESSED;
		stuff_quit(0);
	}

	if (buttons[BUTTON_MOUSE_RIGHT] == BUTTON_PRESSED) {
		buttons[BUTTON_MOUSE_RIGHT] = BUTTON_NO_PRESSED; 
	}	       

	if (buttons[BUTTON_UP] == BUTTON_PRESSED) {
		cursor_y -= CURSOR_STRIDE_Y;
		buttons[BUTTON_UP] = BUTTON_NO_PRESSED;
	}
	if (buttons[BUTTON_DOWN] == BUTTON_PRESSED) {
		cursor_y += CURSOR_STRIDE_Y;
		buttons[BUTTON_DOWN] = BUTTON_NO_PRESSED;
	}
	if (buttons[BUTTON_LEFT] == BUTTON_PRESSED) {
		cursor_x -= CURSOR_STRIDE_X;
		buttons[BUTTON_LEFT] = BUTTON_NO_PRESSED;
	}
	if (buttons[BUTTON_RIGHT] == BUTTON_PRESSED) {
		cursor_x += CURSOR_STRIDE_X;
		buttons[BUTTON_RIGHT] = BUTTON_NO_PRESSED;
	}
	
	// with mouse
	cursor_x = mouse_x;
	cursor_y = mouse_y;
	
	// offset the cursor
	cursor_x -= cursor->w/2;
	cursor_y -= cursor->h/2;       	

	if (buttons[BUTTON_MOUSE_LEFT] == BUTTON_PRESSED) {
		if (msg_newgame == newgame_zoom) {
			PRINTF("New game\n");
			menu_title_stop_music();
			g_init();
			menu_title_finalize();
		}
		else if (msg_training == training_zoom) {
			PRINTF("Training\n");
			menu_title_stop_music();
			tutorial_init();
			menu_title_finalize();
		}
		buttons[BUTTON_MOUSE_LEFT] = BUTTON_NO_PRESSED;
	}
}

void menu_title_compute_physic(void)
{
	int x1, y1;

	x1 = cursor_x;
	y1 = cursor_y;
	
	if (c_surface(newgame_no_zoom, newgame_x, newgame_y, cursor, x1, y1))
		msg_newgame = newgame_zoom;
	else
		msg_newgame = newgame_no_zoom;
	if (c_surface(training_no_zoom, training_x, training_y, cursor, x1, y1)
	    || c_surface(training_zoom, training_x, training_y, cursor, x1, y1))
		msg_training = training_zoom;
	else
		msg_training = training_no_zoom;
}

void menu_title_render_scene(void)
{
	r_draw_background(img_back, NULL);
	r_draw_surface(msg_newgame, newgame_x, newgame_y);
	r_draw_surface(msg_training, training_x, training_y);
	r_draw_surface(cursor, cursor_x, cursor_y);
}
