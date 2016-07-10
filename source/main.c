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
#include "physic.h"

#define LOAD_IMAGE "load.jpg"

univers_t univers_current;

static void 
init_all()
{
	mem_init();
	game_allocation();
	menu_title_init();
	// credit_init();
	// g_init();
}

static void
main_loop()
{
	while (1) {
		frames++;
		time_now = stuff_get_time();
		stuff_handle_event();	       	
#ifndef WII		
		stuff_display_fps();				
#endif
		univers_current.handle_player();
		univers_current.compute_physic();	
		univers_current.render_scene();	
		stuff_refresh_screen();
		stuff_wait_event_or_sleep();		
	}
}

static void
print_load(void)
{
	surface_t *screen;
	char path[PATH_LENGTH_MAX];
	
	sprintf(path, "%s%s", IMAGE_PATH, LOAD_IMAGE);
	screen = stuff_load_img(path);
	r_draw_background(screen, NULL);
	stuff_refresh_screen();
	stuff_free_surface(screen);
}

int 
main(int argc, char **argv)
{
	stuff_init();
	print_load();
	init_all();
	main_loop();
	return 0;
}
