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

#define TUTORIAL_MUSIC "tuto.ogg"
#define TUTORIAL_IMAGE "tuto.bmp"

static void *tutorial_music;
static surface_t *img_back;

static void tutorial_render_scene(void);
static void tutorial_handle_player(void);
static void tutorial_compute_physic(void);
static void tutorial_finalize(void);

static void tutorial_music_hook(void)
{	
	stuff_music_play(tutorial_music, 0, tutorial_music_hook);
}

void tutorial_init(void)
{
	char path[PATH_LENGTH_MAX];
	
	// initialize current univer to menu title
	univers_current.handle_player = tutorial_handle_player;
	univers_current.compute_physic = tutorial_compute_physic;
	univers_current.render_scene = tutorial_render_scene;
	univers_current.finalize = tutorial_finalize;

	// load image 
	sprintf(path, "%s%s", IMAGE_PATH, TUTORIAL_IMAGE);
	img_back = stuff_load_img(path);	

	// load music
	sprintf(path, "%s%s", MUSIC_PATH, TUTORIAL_MUSIC);
	tutorial_music = stuff_music_load(path);
	stuff_music_play(tutorial_music, 0, tutorial_music_hook);			
}

static void tutorial_compute_physic(void)
{
	
}

static void tutorial_finalize(void)
{
	//free stuff
	stuff_free_surface(img_back);	
}

static void tutorial_handle_player(void)
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
	
	if (button_pressed) {
		stuff_music_stop(tutorial_music);	
		menu_title_init();
		tutorial_finalize();
	}
		
}

static void tutorial_render_scene(void)
{	
	 r_draw_background(img_back, NULL);
}
