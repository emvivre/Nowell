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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include "SDL.h"
#include "SDL_image.h"
#include "SDL_rotozoom.h"
#include "SDL_ttf.h"

#ifndef WII
#include "SDL_mixer.h"
#endif

#include "common.h"
#include "sdl_stuff.h"

#define QUIT_CODE_MSG -1

float motion_angle;
char buttons[MAX_BUTTONS];
int button_fire_last_frame; // last frame when the button fire was pressed
int button_fire_before_last_frame; // the before last frame when the button fire was pressed

int mouse_x, mouse_y; // coordinate absolute
int mouse_rel_x, mouse_rel_y; // coordinate relative
int quit_program;

surface_t screen_surface;
static SDL_Surface *sdl_screen_surface;
static int video_flags;

#define INIT_UINT_FROM_COLOR(C, I) {			\
	*(I) =((C)->r<<16) + ((C)->g<<8) + ((C)->b);	\
	*(I) |= 0xFF<<24;				\
} while(0)
	  
#define INIT_SURFACE_FROM_SDL(SDL, S) {	\
		(S)->w = (SDL)->w;	\
		(S)->h = (SDL)->h;	\
		(S)->data = (SDL);	\
	} while(0)

#define INIT_REC_FROM_SDL(SDL, R) { \
		(R)->pos.x = (Sint16) (SDL)->x;	\
		(R)->pos.y = (Sint16) (SDL)->y;	\
		(R)->w = (Sint16) (SDL)->x;	\
		(R)->h = (Sint16) (SDL)->x;	\
	} while(0);

#define INIT_SDL_FROM_REC(R, SDL) { \
		(SDL)->x = (R)->pos.x;		\
		(SDL)->y = (R)->pos.y;		\
		(SDL)->w = (R)->w;		\
		(SDL)->h = (R)->h;		\
	} while(0)

#define INIT_SDL_FROM_COLOR(C, SDL) { \
		(SDL)->r = (C)->r;    \
		(SDL)->g = (C)->g;    \
		(SDL)->b = (C)->b;    \
	}while(0)

void stuff_blit_surface(surface_t *s_src, rect_t *r_src, surface_t *s_dst, rect_t *r_dst)
{
	if (!r_src || !r_dst)
		SDL_BlitSurface(s_src->data, NULL, s_dst->data, NULL);
	else {
		SDL_Rect rec_src, rec_dst;
		INIT_SDL_FROM_REC(r_src, &rec_src);
		INIT_SDL_FROM_REC(r_dst, &rec_dst);
		SDL_BlitSurface(s_src->data, &rec_src, s_dst->data, &rec_dst);
	}
}

void stuff_refresh_screen(void)
{
	SDL_Flip(sdl_screen_surface);
}


void stuff_set_alpha(surface_t *s, char alpha)
{	
	SDL_Surface *surface = (SDL_Surface *)s->data;
	SDL_SetAlpha(surface, SDL_SRCALPHA | SDL_RLEACCEL, alpha);
}

surface_t *stuff_load_img(const char *img)
{
	SDL_Surface *surface;
	surface_t *s;
	
	surface = IMG_Load(img);
	if (!surface) 
		stuff_quit_msg("Unable to load image : %s\n", img);
	s = (surface_t *)mem_alloc(sizeof *s);
	INIT_SURFACE_FROM_SDL(surface, s);
	return s;
}

void stuff_free_surface(surface_t *s)
{
	SDL_FreeSurface(s->data);
	mem_free(s);
}

static void stuff_set_repeat(int delay, int interval)
{
	SDL_EnableKeyRepeat(delay, interval);
}

static void stuff_init_mouse()
{
	SDL_ShowCursor(FALSE);
	SDL_WM_GrabInput(SDL_GRAB_OFF);
}

/*********
 * TTF
 **************/
static void stuff_init_ttf()
{
	TTF_Init();
}

static void stuff_quit_ttf()
{
	TTF_Quit();
}

void *stuff_font_load(char *font, int pt_size)
{
	return TTF_OpenFont(font, pt_size);
}

void stuff_font_unload(void *font)
{
	TTF_CloseFont((TTF_Font *)font);
}

surface_t *stuff_font_render(void *font, const char *text, color_t c)
{
	surface_t *s;
	SDL_Surface *surface;
	SDL_Color sdl_c;
	
	INIT_SDL_FROM_COLOR(&c, &sdl_c);
	surface = TTF_RenderText_Blended((TTF_Font*) font, text, sdl_c);
	s = (surface_t *)mem_alloc(sizeof *s);	
	INIT_SURFACE_FROM_SDL(surface, s);
	return s;
}

void stuff_init()
{
    const SDL_VideoInfo *video_info;
        
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
	    fprintf(stderr, "Video initialization failed: %s\n",
		    SDL_GetError());
	    stuff_quit(1);
    }
    video_info = SDL_GetVideoInfo();
    if (!video_info) {
	    fprintf(stderr, "Video query failed: %s\n",
		    SDL_GetError());
	    stuff_quit(1);
    }
    video_flags = SDL_DOUBLEBUF; 
    video_flags |= SDL_HWPALETTE;       
    video_flags |= SDL_FULLSCREEN;
    if (video_info->hw_available)
	    video_flags |= SDL_HWSURFACE;
    else
	    video_flags |= SDL_SWSURFACE;
    if (video_info->blit_hw)
	    video_flags |= SDL_HWACCEL;
    //SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    sdl_screen_surface = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP,
			       video_flags);
    if (!sdl_screen_surface) {
	    fprintf( stderr,  "Video mode set failed: %s\n", SDL_GetError( ) );
	    stuff_quit( 1 );
    }
    
    INIT_SURFACE_FROM_SDL(sdl_screen_surface, &screen_surface);
    stuff_set_repeat(KEY_REPEAT_DELAY_US, KEY_REPEAT_INTERVAL_US);    
    stuff_init_mouse();
    stuff_init_ttf();
    stuff_music_init();
    #ifdef WII
    wii_initialize();
    #endif
}

void stuff_quit(int code)
{
	char *sdl_error;

	if (TTF_WasInit())
		stuff_quit_ttf();
	sdl_error = SDL_GetError();
	if (sdl_error) 
		PRINTF("SDL_Error : %s\n", sdl_error);
        SDL_Quit();
        exit(code);
}

void stuff_quit_msg(char *fmt, ...)
{
	va_list va;

	va_start(va, fmt);
	vfprintf(stderr, fmt, va);
	va_end(va);
	stuff_quit(QUIT_CODE_MSG);
}

void stuff_display_fps()
{
        static int frames = 0;
        static int time_last = 0;

        frames++;
        if (time_now - time_last >= REFRESH_DISPLAY_FPS) {
                float seconds = (time_now - time_last) / 1000.0;
                float fps = frames / seconds;
                printf("%d frames in %g : %g FPS\n", frames, seconds, fps);
                time_last = time_now;
                frames = 0;
        }       
}

static void handle_key_press()
{
	unsigned char *keystate;
	
	keystate = SDL_GetKeyState(NULL);
	if (keystate[SDLK_ESCAPE]) 
		buttons[BUTTON_ESCAPE] = BUTTON_PRESSED;
#ifndef WII
	if (keystate[SDLK_UP]) {
		buttons[BUTTON_UP] = BUTTON_PRESSED;
		motion_angle = 0;
	}
	if (keystate[SDLK_DOWN]) {
		buttons[BUTTON_DOWN] = BUTTON_PRESSED;
		motion_angle = 180;
	}
	if (keystate[SDLK_LEFT]) { 
		buttons[BUTTON_LEFT] = BUTTON_PRESSED;
		motion_angle = 90;
	}
	if (keystate[SDLK_RIGHT]) {
		buttons[BUTTON_RIGHT] = BUTTON_PRESSED;
		motion_angle = -90;
	}
	if (keystate[SDLK_F1])
                SDL_WM_ToggleFullScreen(sdl_screen_surface);
#endif
}

static void handle_mouse(SDL_MouseMotionEvent *event)
{
	mouse_x = event->x;
	mouse_y = event->y;
	mouse_rel_x = event->xrel;
	mouse_rel_y = event->yrel;
}

static void handle_mouse_button()
{
	unsigned char mouse_key;
	
	mouse_key = SDL_GetMouseState(NULL, NULL);
	if (mouse_key & SDL_BUTTON(1)) {
		buttons[BUTTON_MOUSE_LEFT] = BUTTON_PRESSED;
		button_fire_before_last_frame = button_fire_last_frame;
		button_fire_last_frame = frames;
	}
	if (mouse_key & SDL_BUTTON(3))
		buttons[BUTTON_MOUSE_RIGHT] = BUTTON_PRESSED;
}

void stuff_handle_event()
{
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch(event.type) {
		case SDL_MOUSEMOTION:
			handle_mouse(&event.motion);
			break;
		case SDL_QUIT:
			quit_program = TRUE;
			break;
		}  
	}
	handle_mouse_button();
	handle_key_press();
#ifdef WII
	wii_handle_key();
#endif
}

void *stuff_music_load(const char *file)
{
#ifdef WII
	return wii_music_load(file);
#else
	Mix_Music *m;
	
	m = Mix_LoadMUS(file);
	if (!m) {
		stuff_quit_msg("Unable to open music : %s\n", file);
	}
	return m;	       	      
#endif
}

void stuff_music_init(void)
{
#ifdef WII
	wii_music_init();
#else
	if (Mix_OpenAudio(MUSIC_FREQUENCY, AUDIO_S16SYS, MUSIC_CHANNEL, MUSIC_CHUNK_SIZE) < 0 ) {
		stuff_quit_msg("Unable to open audio!\n");
	}
#endif
}

void stuff_music_destroy(void)
{
#ifdef WII	
#else
	Mix_CloseAudio();
#endif
}

void stuff_music_play(void *music, int loop, void (*finish)(void))
{
#ifdef WII
	wii_music_play(music);
#else	
	Mix_HookMusicFinished(finish);
	if ( Mix_PlayMusic((Mix_Music *)music, loop) < 0 ) {
		stuff_quit_msg("Unable to play music\n");
	}	
#endif
}

void stuff_music_unload(void *music)
{
#ifdef WII       
	wii_music_unload(music);
#else
	Mix_FreeMusic(music);	
#endif
}

void stuff_music_stop()
{
#ifdef WII
	wii_music_stop();
#else
	Mix_HaltMusic();
#endif
}

void stuff_music_pause()
{
#ifdef WII
#else
	Mix_PauseMusic();
#endif
}

unsigned int stuff_get_time(void)
{
	return SDL_GetTicks();
}

void stuff_wait_event_or_sleep(void)
{
        unsigned int delay;		
	
	delay = stuff_get_time() - time_now;
	while (delay < TIME_PER_FRAME ) {
		SDL_Delay(1);
		delay++;
	}
}

surface_t *stuff_rotate(surface_t *s, double angle)
{
	surface_t *surface;
	SDL_Surface *sdl_surface;
	
	sdl_surface = rotozoomSurface(s->data, angle, 1, ROTATION_ANTI_ALIASTING);
	surface = (surface_t *)mem_alloc(sizeof *surface);	
	INIT_SURFACE_FROM_SDL(sdl_surface, surface);
	return surface;
}

surface_t *stuff_zoom(surface_t *s, double zoom)
{
	surface_t *surface;
	SDL_Surface *sdl_surface;
	
	sdl_surface = zoomSurface(s->data, zoom, zoom, ZOOM_ANTI_ALIASTING);
	surface = (surface_t *)mem_alloc(sizeof *surface);	
	INIT_SURFACE_FROM_SDL(sdl_surface, surface);
	return surface;
}

void stuff_fill_rect(rect_t *rect, color_t c)
{
	SDL_Rect r;
	unsigned int color;

	INIT_SDL_FROM_REC(rect, &r);
	color = SDL_MapRGB(sdl_screen_surface->format, c.r, c.g, c.b);
	SDL_FillRect(sdl_screen_surface, &r, color );
}

void stuff_set_color_transparence(surface_t *surface, color_t c)
{
	unsigned int color;
	SDL_Surface *sdl_surface;
	
	sdl_surface = (SDL_Surface *)surface->data;
	color = SDL_MapRGB(sdl_surface->format, c.r, c.g, c.b);
	SDL_SetColorKey(sdl_surface, SDL_SRCCOLORKEY, color);
}
