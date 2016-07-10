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

#ifndef __COMMON_H__
#define __COMMON_H__

#include <stdio.h>
#ifdef WII
// wii stuff
#define main SDL_main
#define FONT_PATH "/apps/santa/font/"
#define IMAGE_PATH "/apps/santa/image/"
#define LEVEL_PATH "/apps/santa/level/"
#define MASK_PATH "/apps/santa/masks/"
#define MUSIC_PATH "/apps/santa/music/"
#define MENU_TITLE_IMAGE "/apps/santa/image/menu_title.jpeg"
#define MENU_TITLE_CURSOR "/apps/santa/image/menu_cursor_title.bmp"
#define MENU_TITLE_FONT "/apps/santa/font/menu_title_5inq_Handserif.ttf"
#define GAME_BACKGROUND "/apps/santa/image/game_background.jpeg"
#define LEVEL_ZERO_BACKGROUND "/apps/santa/image/game_background.jpeg"

#define PRINTF(fmt, arg...)
#else
// computer stuff
#define FONT_PATH "font/"
#define IMAGE_PATH "image/"
#define LEVEL_PATH "level/"
#define MASK_PATH "masks/"
#define MUSIC_PATH "music/"
#define MENU_TITLE_IMAGE "image/menu_title.jpeg"
#define MENU_TITLE_CURSOR "image/menu_cursor_title.bmp"
#define MENU_TITLE_FONT "font/menu_title_5inq_Handserif.ttf"
#define GAME_BACKGROUND "image/game_background.jpeg"
#define LEVEL_ZERO_BACKGROUND "image/game_background.jpeg"

#define PRINTF(fmt, arg...) printf(fmt, ## arg)
#endif

#define PATH_LENGTH_MAX 128

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define SCREEN_RESOLUTION (SCREEN_WIDTH*SCREEN_HEIGHT)
#define SCREEN_BPP 16

#define MAX_NUMBER 0x0FFFFFFF

#define TIME_PER_FRAME 40 // in millisecond
#define TIME_OFFSET 1
#define REFRESH_DISPLAY_FPS 5000 // in milisecond

#define ENEMY_TIMEOUT_OUT_SCREEN_SEC 5
#define MILLISECOND_PER_FRAME 40
#define SECOND_PER_FRAME (MILLISECOND_PER_FRAME/1000)
#define FRAME_PER_SECOND (1000 * 1/MILLISECOND_PER_FRAME)
#define FRAME2SEC(F) (F*SECOND_PER_FRAME)
#define SEC2FRAME(S) (S*FRAME_PER_SECOND)

#ifndef NULL
#define NULL ((void *)0)
#endif

enum {FALSE, TRUE};

/* music */
#define MUSIC_FREQUENCY 44100
#define MUSIC_CHANNEL 2
#define MUSIC_CHUNK_SIZE 2048

#define ROTATION_ANTI_ALIASTING TRUE
#define ZOOM_ANTI_ALIASTING TRUE


#define KEY_REPEAT_DELAY_US 4000
#define KEY_REPEAT_INTERVAL_US 4000
enum {BUTTON_UP,
      BUTTON_DOWN,
      BUTTON_LEFT,
      BUTTON_RIGHT,
      BUTTON_A,
      BUTTON_B,
      BUTTON_ESCAPE,
      BUTTON_MOUSE_LEFT,
      BUTTON_MOUSE_RIGHT,
      MAX_BUTTONS
};
enum {BUTTON_NO_PRESSED,
      BUTTON_PRESSED, 
      BUTTON_RELEASED, 
      BUTTON_HELDED, 
      MAX_BUTTON_STATED
};
extern char buttons[MAX_BUTTONS];
extern int button_fire_last_frame; // last frame when the button fire was pressed
extern int button_fire_before_last_frame; // the before last frame when the button fire was pressed

extern float motion_angle; // angle que fait le nunchuk ou le clavier avec les touches fleches
extern int mouse_x, mouse_y; // coordinate absolute
extern int mouse_rel_x, mouse_rel_y; // coordinate relative

typedef struct univers_s
{
	void (*handle_player)(void);
	void (*compute_physic)(void);
	void (*render_scene)(void);
	void (*finalize)(void); // unload univers
} univers_t;

#define PI 3.14159265358979323846
#define DEG2RAD(X) (X * PI/180)
#define RAD2DEG(X) (X * 180/PI)
#define DOT_PRODUCT(X1, Y1, X2, Y2) (X1*X2 + Y1*Y2)

typedef struct {
	int x;
	int y;
} vertex_t;

typedef struct {
	double x;
	double y;
} vector_t;

extern vertex_t axis_X;
extern vertex_t axis_Y;

typedef struct {
	unsigned char r;
	unsigned char g;
	unsigned char b;
} color_t;

// couleure pre-defini
enum {
#define C(INDICE, COLOR, C, D, E) COLOR = INDICE,
#include "color.h"
#undef C
	NB_COLOR
};

// color.c
extern color_t colors[NB_COLOR+1];


typedef struct {
	double x;
	double y;
} pos_t;

// une surface permet juste de referencier des data
// donc ne contient pas de position dans le monde 2D
typedef struct {
	int w;
	int h;
	void *data;
} surface_t;
	
typedef struct {
	pos_t pos;
	int w;
	int h;
} rect_t;

// main.c
extern univers_t univers_current;

// *_stuff.h
#define FRAME_INIT 1
extern surface_t screen_surface;
extern int quit_program; 
extern int frames; // current frame
extern unsigned int time_delta; // if late with time
extern unsigned int time_now; // current time 

#ifdef WII
// wii.c
extern float nunchuk_x;
extern float nunchuk_y;

void wii_initialize(void);
void wii_handle_key(void);
void wii_music_init(void);
void wii_music_play(void *music);
int wii_music_is_playing(void);
void wii_music_stop();
void *wii_music_load(const char *file);
void wii_music_unload(void *music);
#endif

// *_stuff.h
void stuff_fill_rect(rect_t *rect, color_t c);
void stuff_init(void);
void stuff_quit_msg(char *fmt, ...);
void stuff_quit(int code);
void stuff_display_fps(void);
void stuff_handle_event(void);
void stuff_wait_event_or_sleep(void);
unsigned int stuff_get_time(void);
surface_t *stuff_load_img(const char *img);
void stuff_free_surface(surface_t *s);
void stuff_refresh_screen(void);
void stuff_blit_surface(surface_t *s_src, rect_t *r_src, surface_t *s_dst, rect_t *r_dst);
void stuff_set_alpha(surface_t *s, char alpha);
void *stuff_font_load(char *font, int pt_size);
void stuff_font_unload(void *font);
surface_t *stuff_font_render(void *font, const char *text, color_t c);
surface_t *stuff_rotate(surface_t *s, double angle);
surface_t *stuff_zoom(surface_t *s, double zoom);
void *stuff_music_load(const char *file);
void stuff_music_play(void *music, int loop, void (*finish)(void));
void stuff_music_stop();
void stuff_music_unload(void *music);
void stuff_music_pause(void);
void stuff_music_init(void);
void stuff_music_destroy(void);
void stuff_set_color_transparence(surface_t *surface, color_t c);

// credit.c
void credit_init(void);

// menu_title.c
extern void *font_menu_title;
void menu_title_init(void);
void menu_title_handle_player(void);
void menu_title_compute_physic(void);
void menu_title_render_scene(void);
void menu_title_finalize(void);

// tutorial.c 
void tutorial_init(void);

// dummy.c 
void dummy_handle_player(void);

// mem.c 
void mem_init(void);
void *mem_alloc(size_t size);
void mem_free(void *ptr);

// render.c
void r_blit_surface(surface_t *s, surface_t *d, int x, int y);
void r_draw_background(surface_t *s, rect_t *r);
void r_draw_surface(surface_t *s, int x, int y);

// collision.c 
int c_surface(surface_t *s, int s_x, int s_y, surface_t *d, int d_x, int d_y);
int c_out_screen(pos_t *p, surface_t *s);

// math.c 
double m_vector_length(vector_t *v);
void m_vector_normalize(vector_t *v, vector_t *result);
void m_vector_substract(vector_t *a, vector_t *b, vector_t *result);
double m_length_btw_two_points(int x1, int y1, int x2, int y2);
void m_vector_normalize_btw_2_points(int src_x, int src_y, int dst_x, int dst_y, vector_t *result);
double m_angle_rot_Y(double x, double y);

// return TRUE si collision entre une entite et le masque de collision de l'autre entite
// int c_entity_mask(entity_t *s, entity_t *d);

// physic
#endif /* not __COMMON_H__ */
