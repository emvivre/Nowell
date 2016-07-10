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

#define CREDIT_TIME (SEC2FRAME(145))
#define CREDIT_MUSIC "cred.ogg"
#define CREDIT_IMAGE "cred.jpg"

static void *credit_music;
static surface_t *img_back;

static void credit_render_scene(void);
static void credit_handle_player(void);
static void credit_compute_physic(void);
static void credit_finalize(void);
static int credit_end; // frame of the end

static void credit_music_hook(void)
{	
	stuff_music_play(credit_music, 0, credit_music_hook);
}

void credit_init(void)
{
	char path[PATH_LENGTH_MAX];
	
	// initialize current univer to menu title
	univers_current.handle_player = credit_handle_player;
	univers_current.compute_physic = credit_compute_physic;
	univers_current.render_scene = credit_render_scene;
	univers_current.finalize = credit_finalize;

	// load image 
	sprintf(path, "%s%s", IMAGE_PATH, CREDIT_IMAGE);
	img_back = stuff_load_img(path);	

	// load music
	sprintf(path, "%s%s", MUSIC_PATH, CREDIT_MUSIC);
	credit_music = stuff_music_load(path);
	stuff_music_play(credit_music, 0, credit_music_hook);			
	
	credit_end = frames + CREDIT_TIME;
}

static void credit_compute_physic(void)
{
	if (credit_end <= frames) { 
		stuff_music_stop(credit_music);	
		menu_title_init();
		credit_finalize();		
	}
}

static void credit_finalize(void)
{
	//free stuff
	stuff_free_surface(img_back);	
}

static void credit_handle_player(void)
{	
	int button_pressed = FALSE;
	
	if (buttons[BUTTON_ESCAPE] == BUTTON_PRESSED) {
		buttons[BUTTON_ESCAPE] = BUTTON_NO_PRESSED;
		button_pressed = TRUE;
	}
	if (buttons[BUTTON_MOUSE_RIGHT] == BUTTON_PRESSED) {
		buttons[BUTTON_MOUSE_RIGHT] = BUTTON_NO_PRESSED; 
		button_pressed = TRUE;
	}
	if (buttons[BUTTON_MOUSE_LEFT] == BUTTON_PRESSED) {
		buttons[BUTTON_MOUSE_LEFT] = BUTTON_NO_PRESSED;
		button_pressed = TRUE;
	}

	/*	
	if (button_pressed) {
		stuff_music_stop(credit_music);	
		menu_title_init();
		credit_finalize();
	}
	*/		

}

static void credit_render_scene(void)
{	
	 r_draw_background(img_back, NULL);
}
