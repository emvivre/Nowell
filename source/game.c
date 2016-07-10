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

#include <math.h>
#include <string.h>

#include "common.h"
#include "game.h"
#include "list.h"

/* player */
#define PLAYER_NB_LIVE_INIT 3
#define PLAYER_NB_LIVE_MAX 10
#define PLAYER_NB_BOMB_INIT 3
#define PLAYER_NB_BBOM_MAX 6
#define PLAYER_ENT_X_INIT (SCREEN_WIDTH/2)
#define PLAYER_ENT_Y_INIT (SCREEN_HEIGHT - SCREEN_HEIGHT/10)
#define PLAYER_ENT_X_STRIDE 10
#define PLAYER_ENT_Y_STRIDE 10
#define PLAYER_MASK_INIT_X 0
#define PLAYER_MASK_INIT_Y 0
#define PLAYER_MASK_WIDTH 5
#define PLAYER_MASK_HEIGHT 5
#define PLAYER_Y_MIN 32
#define PLAYER_Y_MAX (SCREEN_HEIGHT - 32)
#define PLAYER_POS_PADDING_COEF_X -10
#define PLAYER_POS_PADDING_COEF_Y -10

#define PLAYER_LIFE_POS_X 0
#define PLAYER_LIFE_POS_Y 30
#define PLAYER_LIFE_STRIDE 50

#define PLAYER_SCORE_COLOR colors[COLOR_BLACK]
#define PLAYER_SCORE_POS_X 0
#define PLAYER_SCORE_POS_Y 70

#define PLAYER_IMMORTAL_TIME_SEC (SEC2FRAME(2))
#define PLAYER_IMMORTAL_INVISIBLE 300
#define PLAYER_IMMORTAL_VISIBLE 1000

#define PLAYER_DELAY_NO_FIRE_WHEN_PREPARE (SEC2FRAME(0.6))

/* en dessou de ce temps la, c'est du lazy */
#define PLAYER_DELAY_FIRE_MODE_LAZY (1)
#define PLAYER_BULLET_CADENCE_LAZY (SEC2FRAME(0.05))
#define PLAYER_DELAY_SLOW_DOWN (SEC2FRAME(0.1))
#define PLAYER_COEF_SLOW_DOWN 0.5

#define BOSS_FONT "menu_title_5inq_Handserif.ttf"
#define BOSS_FONT_PT_SIZE 32
#define BOSS_FONT_COLOR colors[COLOR_RED]
#define BOSS_TEXT_POS_X 40
#define BOSS_TEXT_POS_Y 350

/* definition des temps max de transformation */
#define BOSS_DELAY_TRANS_MSG_START (SEC2FRAME(2))
#define BOSS_DELAY_TRANS_MSG_END (SEC2FRAME(2))
#define BOSS_DELAY_TRANS_DYING (SEC2FRAME(0.01))
#define BOSS_SCROLL_STRIDE SCROLLING_VERTICAL_SPEED

/* en dessous de se seuil la transfomation du boss n'existe plus */
#define BOSS_LIFE_MIN 10
#define BOSS_BARRE_LIFE_X 160
#define BOSS_BARRE_LIFE_Y 40
#define BOSS_BARRE_LIFE_WIDTH_MAX 300  
#define BOSS_BARRE_LIFE_HEIGHT_MAX 30
#define BOSS_BARRE_COLOR colors[COLOR_RED]

/* number of frame touching is visible */
#define ENEMY_DELAY_TOUCH (SEC2FRAME(0.05))

#define BULLET_MASK_WIDTH 32
#define BULLET_MASK_HEIGHT 32

#define CROSSHAIR_IMAGE "crosshair_32x32.bmp"
#define CANON_IMAGE "canon.bmp"
#define PLAYER_IMAGE "player.bmp"
#define LIFE_IMAGE "life_32x32.bmp"
#define SCORE_FONT "menu_title_5inq_Handserif.ttf"
#define SCORE_PT_SIZE 32

#define GAMEOVER_DELAY (SEC2FRAME(10))
#define GAMEOVER_FONT "menu_title_5inq_Handserif.ttf"
#define GAMEOVER_PT_SIZE 64
#define GAMEOVER_MSG "Game Over"
#define GAMEOVER_POS_X 100
#define GAMEOVER_POS_Y 150
#define GAMEOVER_ZOOM 2
#define GAMEOVER_COLOR colors[COLOR_RED]

#define PAUSE_FONT "menu_title_5inq_Handserif.ttf"
#define PAUSE_PT_SIZE 64

#define PAUSE_UPPER_MSG "Pause"
#define PAUSE_UPPER_POS_X 200
#define PAUSE_UPPER_POS_Y 150
#define PAUSE_UPPER_ZOOM 2
#define PAUSE_UPPER_COLOR colors[COLOR_GREEN]

#define PAUSE_DOWN_MSG "Press A to continue or B to quit"
#define PAUSE_DOWN_POS_X 115
#define PAUSE_DOWN_POS_Y 300
#define PAUSE_DOWN_ZOOM 0.6
#define PAUSE_DOWN_COLOR colors[COLOR_GREEN]


#define LEVEL_START 0
#define LEVEL_0_FILE "level_0.xml"

#define MUSIC "game.ogg"

/* number of y scroll per frame */
#define SCROLLING_VERTICAL_SPEED 1

vertex_t axis_X = {1, 0};
vertex_t axis_Y = {0, 1};

static void *music; // current music 
static void *music_level; // music general
static int screen_y_stride;
static int player_immortal_time[2] = {PLAYER_IMMORTAL_VISIBLE, PLAYER_IMMORTAL_INVISIBLE};
static bullet_desc_t bullet_desc_player_snowball;

bullet_desc_t *bullet_desc_snowball = &bullet_desc_player_snowball;

static unsigned int pause_frame_save; // numero de la frame lorsque la pause a ete declancher
static char pause;  // si a true alors pause
static void *pause_font;
static surface_t *pause_upper_surface;
static surface_t *pause_down_surface;

static unsigned int gameover_end; // end of gameover
static void *gameover_font;
static char gameover; // si a true alors gameover
static surface_t *gameover_surface; 
static void *score_font; 
static void *boss_font;
static rect_t boss_barre_life; 

mask_t bullet_mask; // default bullet's mask 
mask_t player_mask; // player's collision mask
player_t player_0; // player no 0
level_t *g_level; // le level courrant
list_t *g_traces; // list des traces
list_t *g_bullets; 
list_t *g_bullets_player;
list_t *g_bullets_desc; 
list_t *g_bullets_type; 
list_t *g_waves_desc; 
list_t *g_moves; // list des mouvements 
list_t *g_enemy_moves_desc; // list d'agrgat de mouvements 
list_t *g_masks;
list_t *g_enemies_desc;
list_t *g_enemies;
list_t *g_entities;
list_t *g_boss; // contient les boss
list_t *g_sprites; // contient les descripteur de sprites
list_t *g_musics; // list of musics
static list_t *g_sprites_context; // contient les contextes de sprites

/* y of the screen, it's the counter use to spawn enemies */
static int screen_y;
static surface_t *background;
static surface_t *player_life;
static rect_t background_clip;
static enemy_t *enemy_next; /* prochain enemy a spawn */
static boss_t *boss_current; /* boss actuel : utiliser pour manager un boss */
static boss_t *boss_next; /* prochain boss a spawn */
static int enemy_id; /* id of enemy, up at each spawn of an enemy */
static int entity_id; /* id d'une entite */
static int bullet_id; /* id d'une bullet */
static int sprite_id;

static void g_music_finish_hook(void)
{
	stuff_music_play(music, 1, g_music_finish_hook);
}

static int g_check_collision_enemy_bullet(bullet_t *b)
{
	int s_x, s_y;
	int d_x, d_y;
	
	s_x = player_0.ent.pos.x - player_0.ent.mask_collision->surface.w/2;
	s_y = player_0.ent.pos.y - player_0.ent.mask_collision->surface.h/2;
	d_x = b->ent.pos.x - b->bullet->type->surface->w/2;
	d_y = b->ent.pos.y - b->bullet->type->surface->h/2;
	if (c_surface(&player_0.ent.mask_collision->surface, s_x, s_y, b->bullet->type->surface, d_x, d_y))
		return TRUE;
	return FALSE;
}

static enemy_t *g_check_collision_player_bullet(bullet_t *b)
{
	enemy_t *e;
	int d_x, d_y;
	
	d_x = b->ent.pos.x - b->bullet->type->surface->w/2;
	d_y = b->ent.pos.y - b->bullet->type->surface->h/2;

	/* check position of all enemy */
	list_parcour_init(g_level->enemy);
	while ((e = list_parcour_next(g_level->enemy))) {
		int s_x, s_y;
		
		if (!e->popup )
			break;
		s_x = e->ent.pos.x - e->ent.mask_collision->surface.w/2 + e->desc->mask_position.x;
		s_y = e->ent.pos.y - e->ent.mask_collision->surface.h/2 + e->desc->mask_position.y;
		if (c_surface(&e->ent.mask_collision->surface, s_x, s_y, b->bullet->type->surface, d_x, d_y))
			return e;
	}
	return NULL;
}

static int g_check_collision_enemy_with_player(enemy_t *e)
{
	int s_x, s_y;
	int d_x, d_y;
	s_x = e->ent.pos.x - e->ent.mask_collision->surface.w/2 + e->desc->mask_position.x;
	s_y = e->ent.pos.y - e->ent.mask_collision->surface.h/2 + e->desc->mask_position.y;
	
	d_x = player_0.ent.pos.x - player_0.ent.mask_collision->surface.w/2;
	d_y = player_0.ent.pos.y - player_0.ent.mask_collision->surface.h/2;
	if (c_surface(&e->ent.mask_collision->surface, s_x, s_y,
		      &player_0.ent.mask_collision->surface, d_x, d_y))
		return TRUE;
	return FALSE;
}

static void g_credit(void)
{
	stuff_music_stop();
	credit_init();
	g_finalize();
}

static void g_gameover_finish(void)
{
	gameover = FALSE;
	stuff_music_stop();
	menu_title_init();		
	g_finalize();
}

static void g_compute_physic_pause(void)
{

}

static void g_compute_physic_gameover(void)
{
	if (gameover_end < frames) {
		g_gameover_finish();
	}	
}

static void g_player_die(void)
{
	if (!player_0.immortal) {		
		player_0.live--;
		if (player_0.live < 0) { 
			/* gameover */
			gameover = TRUE;
			gameover_end = frames + GAMEOVER_DELAY;
			univers_current.compute_physic = g_compute_physic_gameover;			   
		}
		else {
			player_0.immortal = TRUE;
			player_0.visible = TRUE;
			player_0.immortal_begin = 0;
			player_0.immortal_frame_visible = frames + player_immortal_time[player_0.immortal];
			PRINTF("---------========== PLAYER LIFE : %d ==========---------\n", player_0.live);
		}	
	}
}

static void g_enemy_remove(enemy_t *e)
{
	list_rm(g_level->enemy, &e->id);
}

static sprite_t *g_sprite_enemy_dead_create(enemy_t *e)
{
	sprite_t *s;

	s = (sprite_t *)mem_alloc(sizeof *s);
	s->id = sprite_id++;
	s->pos.x = e->ent.pos.x;
	s->pos.y = e->ent.pos.y;
	s->frame_current = 0;
	s->desc = e->desc->sprite_dead;
	s->next_frame = frames + s->desc->delay;
	s->frame = s->desc->frames;	
	return s;
}

static void g_enemy_die(enemy_t *e)
{
	e->live--;
	e->touch = TRUE;
	e->touch_begin = frames;
	if (e->live < 0 ) {
		sprite_t *s;

		player_0.score += e->desc->score;

		/* add sprite of the dead */
		s = g_sprite_enemy_dead_create(e);
		list_add(g_sprites_context, &s->id, s);
		e->ent.pos.x = -100;
		e->ent.pos.y = -100;		
		g_enemy_remove(e);		
	}
}

// function of compare 
int g_strcmp(void *k1, void *k2)
{
	return strcmp((char *)k1, (char *)k2);
}

int g_intcmp(void *k1, void *k2)
{
	int i1, i2;

	i1 = *(int *)k1;
	i2 = *(int *)k2;
	if (i1 < i2)
		return -1;
	if (i1 > i2)
		return 1;
	return 0;
}

static void g_allocation_player(void)
{
	char path[PATH_LENGTH_MAX];

	// load player image
	sprintf(path, "%s%s", IMAGE_PATH, PLAYER_IMAGE);
	player_0.ent.surface = stuff_load_img(path);

	// load crosshair image
	sprintf(path, "%s%s", IMAGE_PATH, CROSSHAIR_IMAGE);
	player_0.crosshair = stuff_load_img(path);

	// load life image
	sprintf(path, "%s%s", IMAGE_PATH, LIFE_IMAGE);
	player_life = stuff_load_img(path);	

	// load canon image
	sprintf(path, "%s%s", IMAGE_PATH, CANON_IMAGE);
	player_0.canon = stuff_load_img(path);
	player_0.canon_rot = stuff_rotate(player_0.canon, 0);
}

static void g_initialize_player(void)
{
	// initialise mask
	player_mask.surface.w = PLAYER_MASK_WIDTH;
	player_mask.surface.h = PLAYER_MASK_HEIGHT;	
	
	player_0.live = PLAYER_NB_LIVE_INIT;
	player_0.n_bomb = PLAYER_NB_BOMB_INIT;
	player_0.last_fire = 0;
	player_0.ent.pos.x = PLAYER_ENT_X_INIT;
	player_0.ent.pos.y = PLAYER_ENT_Y_INIT;
	player_0.ent.mask_collision = &player_mask;
	player_0.immortal = TRUE;
	player_0.immortal_begin = frames;
	player_0.visible = TRUE;
	player_0.immortal_frame_visible = frames + player_immortal_time[player_0.visible];
	player_0.fire_mode = FIRE_MODE_NORMAL;
	player_0.score = 0;
	player_0.angle_surface_rot = 0;
	player_0.surface_rot = stuff_rotate(player_0.ent.surface, (double) player_0.angle_surface_rot);
	player_0.pos_padding.x = 0;
	player_0.pos_padding.y = 0;

	// load crosshair	
	player_0.crosshair_pos.x = player_0.ent.pos.x;
	player_0.crosshair_pos.y = player_0.ent.pos.y - 40;
	player_0.crosshair_vector.x = 0;
	player_0.crosshair_vector.y = 1;	
	
}

static void g_allocation_level(void)
{
	g_level->enemy = list_create(g_intcmp);
	g_level->enemy_sort = g_enemies;
	g_level->enemy_total = list_count(g_enemies);	
}

static void g_initialize_level(void)
{       
	screen_y = SCREEN_HEIGHT;
	screen_y_stride = SCROLLING_VERTICAL_SPEED;
	gameover = FALSE;
	pause = FALSE;
	g_level->enemy_current = 0;
	PRINTF("total enemies : %d\n", g_level->enemy_total);	       
	list_parcour_init(g_level->enemy_sort);	
	enemy_next = (enemy_t *)list_parcour_next(g_level->enemy_sort);
	list_parcour_init(g_boss);	
	boss_next = (boss_t *)list_parcour_next(g_boss);
	boss_current = NULL;
}

void game_allocation(void)
{
	char path[PATH_LENGTH_MAX];
	surface_t *gameover_normal;
	surface_t *pause_normal;

	// load background image
	background = stuff_load_img(GAME_BACKGROUND);	

	// initialize some structures 
	g_level = (level_t *)mem_alloc(sizeof *g_level);
	g_traces = list_create(g_strcmp);
	g_bullets = list_create(g_intcmp);
	g_bullets_player = list_create(g_intcmp);
	g_bullets_desc = list_create(g_strcmp);
	g_bullets_type = list_create(g_strcmp);
	g_waves_desc = list_create(g_strcmp);
	g_moves = list_create(g_strcmp);
	g_enemy_moves_desc = list_create(g_strcmp);
	g_masks = list_create(g_strcmp);
	g_enemies_desc = list_create(g_strcmp);
	g_enemies = list_create(g_intcmp);
	g_boss = list_create(g_intcmp);
	g_entities = list_create(g_strcmp);
	g_sprites = list_create(g_strcmp);
	g_sprites_context = list_create(g_intcmp);
	g_musics = list_create(g_strcmp);

	// read xml file
	sprintf(path, "%s%s", LEVEL_PATH, LEVEL_0_FILE);
	g_level_xml(path);

	// initialize score font
	sprintf(path, "%s%s", FONT_PATH, SCORE_FONT);	
	score_font = stuff_font_load(path, SCORE_PT_SIZE);

	// initialize gameover font 
	sprintf(path, "%s%s", FONT_PATH, GAMEOVER_FONT);	
	gameover_font = stuff_font_load(path, GAMEOVER_PT_SIZE);

	// initialize gameover surface
	gameover_normal = stuff_font_render(gameover_font, GAMEOVER_MSG, GAMEOVER_COLOR);
	gameover_surface = stuff_zoom(gameover_normal, GAMEOVER_ZOOM);
	stuff_free_surface(gameover_normal);

	// initialize boss font
	sprintf(path, "%s%s", FONT_PATH, BOSS_FONT);	
	boss_font = stuff_font_load(path, BOSS_FONT_PT_SIZE);

	// initialize pause font
	sprintf(path, "%s%s", FONT_PATH, PAUSE_FONT);	
	pause_font = stuff_font_load(path, PAUSE_PT_SIZE);
	
	// initialize pause surface
	pause_normal = stuff_font_render(pause_font, PAUSE_UPPER_MSG, PAUSE_UPPER_COLOR);
	pause_upper_surface = stuff_zoom(pause_normal, PAUSE_UPPER_ZOOM);
	stuff_free_surface(pause_normal);
	pause_normal = stuff_font_render(pause_font, PAUSE_DOWN_MSG, PAUSE_DOWN_COLOR);
	pause_down_surface = stuff_zoom(pause_normal, PAUSE_DOWN_ZOOM);
	stuff_free_surface(pause_normal);

	// allocation player
	g_allocation_player();

	// allocation level 
	g_allocation_level();
}

void g_init(void)
{		
	char path[PATH_LENGTH_MAX];

	// initialize current univer to menu title
	univers_current.handle_player = g_handle_player;
	univers_current.compute_physic = g_compute_physic;
	univers_current.render_scene = g_render_scene;
	univers_current.finalize = g_finalize;

	// initialize background
	background_clip.pos.x = 0;
	background_clip.pos.y = background->h - SCREEN_HEIGHT;
	background_clip.w = SCREEN_WIDTH;
	background_clip.h = SCREEN_HEIGHT;
	
	// initialize player
	g_initialize_player();
	
	// initialise default bullet mask
	bullet_mask.surface.w = BULLET_MASK_WIDTH;
	bullet_mask.surface.h = BULLET_MASK_HEIGHT;

	// chargement de la musique generale
	sprintf(path, "%s%s", MUSIC_PATH, MUSIC);
	music_level = stuff_music_load(path);
	music = music_level;
	stuff_music_play(music, 1, g_music_finish_hook);

	g_initialize_level();       

	PRINTF("-= Initialization level finish =-\n");
}

static void g_bullet_init_trace(bullet_t *b, entity_t *e)
{
	trace_t *t;
	vector_t v;

	t = b->bullet->trace;
	switch(t->type) {
	case TRACE_AUTOAIM:
		m_vector_normalize_btw_2_points(e->pos.x + b->bullet->pos.x, e->pos.y + b->bullet->pos.y, player_0.ent.pos.x, player_0.ent.pos.y, &v);
		b->dx = t->tr.rectiline.speed * v.x;
		b->dy = t->tr.rectiline.speed * v.y;
		break;
	case TRACE_RECTILINE:
		b->dx = t->tr.rectiline.speed * t->tr.rectiline.dx;
		b->dy = t->tr.rectiline.speed * t->tr.rectiline.dy;
		break;
	case TRACE_CIRCULAR:
		switch (t->tr.circular.rot_center) {
		case CENTER_FIX:
			b->rot_center.pos_fix.x = b->ent.pos.x + 1;
			b->rot_center.pos_fix.y = b->ent.pos.y + 1;
			break;
		case CENTER_ENEMY:
			b->rot_center.pos_enemy = &e->pos;
			b->rot_center.relative_enemy.x = b->bullet->pos.x;
			b->rot_center.relative_enemy.y = b->bullet->pos.y;
			break;
		}
		break;
	}
       
}

void g_player_fire(void)
{
	bullet_t *b;

	player_0.last_fire = frames;
	b = (bullet_t *)mem_alloc(sizeof *b);
	b->id = bullet_id++;
	b->ent.pos.x = player_0.ent.pos.x + bullet_desc_snowball->pos.x;
	b->ent.pos.y = player_0.ent.pos.y + bullet_desc_snowball->pos.y;
	b->ent.mask_collision = &bullet_mask;
	b->bullet = bullet_desc_snowball;
	b->length = m_length_btw_two_points(b->ent.pos.x, b->ent.pos.y,
					    player_0.ent.pos.x, player_0.ent.pos.y);
	b->dx = b->bullet->trace->tr.rectiline.speed * player_0.crosshair_vector.x;
	b->dy = b->bullet->trace->tr.rectiline.speed * player_0.crosshair_vector.y;
	list_add(g_bullets_player, &b->id, b);
}

static void g_compute_canon_angle()
{
	double angle;
	
	angle = m_angle_rot_Y(player_0.crosshair_vector.x, player_0.crosshair_vector.y);
	angle -= 180;
	player_0.canon_angle = angle;
}

static void g_compute_pos_padding(void)
{
	int angle;
	double ratio;

	angle = (int) motion_angle;
	if (angle < 0)
		angle = -angle;
	angle = angle % 90;
	if (angle > 45 ) {
		angle -= 45;
		angle = 45 - angle;
	}	
	ratio = (double) angle/45;
	player_0.pos_padding.x = ratio * PLAYER_POS_PADDING_COEF_X;
	player_0.pos_padding.y = ratio * PLAYER_POS_PADDING_COEF_Y;
	
}

void g_handle_player(void)
{
	float slow_down = 1.0f;		


	/* compute the position of the crosshair */
	player_0.crosshair_pos.x = mouse_x;
	player_0.crosshair_pos.y = mouse_y;
	m_vector_normalize_btw_2_points(player_0.ent.pos.x, player_0.ent.pos.y, player_0.crosshair_pos.x,
					player_0.crosshair_pos.y, &player_0.crosshair_vector);

	g_compute_canon_angle();
	stuff_free_surface(player_0.canon_rot);
	player_0.canon_rot = stuff_rotate(player_0.canon, player_0.canon_angle);

	g_compute_pos_padding();
	if (player_0.angle_surface_rot != (int) motion_angle) {
		player_0.angle_surface_rot = (int) motion_angle;
		stuff_free_surface(player_0.surface_rot);
		player_0.surface_rot = stuff_rotate(player_0.ent.surface, (double) player_0.angle_surface_rot);
	}
	if (buttons[BUTTON_ESCAPE] == BUTTON_PRESSED) {
		buttons[BUTTON_ESCAPE] = BUTTON_NO_PRESSED;
		if (gameover) {
			g_gameover_finish();
		}
		else if (!pause) {
			pause = TRUE;
			pause_frame_save = frames;
			univers_current.compute_physic = g_compute_physic_pause;   
		}
	}
	if (gameover) {
		if (buttons[BUTTON_MOUSE_LEFT] == BUTTON_PRESSED) {
			buttons[BUTTON_MOUSE_LEFT] = BUTTON_NO_PRESSED;		
			g_gameover_finish();
		}
		else if (buttons[BUTTON_MOUSE_RIGHT] == BUTTON_PRESSED) {
			buttons[BUTTON_MOUSE_RIGHT] = BUTTON_NO_PRESSED;
			g_gameover_finish();
		}
		return ;
	}

	if (pause) {
		if (buttons[BUTTON_MOUSE_LEFT] == BUTTON_PRESSED) {
			buttons[BUTTON_MOUSE_LEFT] = BUTTON_NO_PRESSED;		
			pause = FALSE;
			frames = pause_frame_save;
			univers_current.compute_physic = g_compute_physic;
		}
		else if (buttons[BUTTON_MOUSE_RIGHT] == BUTTON_PRESSED) {
			buttons[BUTTON_MOUSE_RIGHT] = BUTTON_NO_PRESSED;
			pause = FALSE;
			frames = pause_frame_save;			
			g_gameover_finish();
		}
		return ;
	}

	if (buttons[BUTTON_MOUSE_LEFT] == BUTTON_PRESSED) {
		/* fire */
		buttons[BUTTON_MOUSE_LEFT] = BUTTON_NO_PRESSED;		
		if (player_0.fire_mode == FIRE_MODE_LAZY) {
			/* check if player continue to hold down fire button */		
			if (frames - button_fire_before_last_frame > PLAYER_DELAY_FIRE_MODE_LAZY)
				player_0.fire_mode = FIRE_MODE_NORMAL;
			else {
				/* player continue to hold down, so slow down motion */
				slow_down = PLAYER_COEF_SLOW_DOWN;
				
				/* check if bullet can be fire */
				if (frames - player_0.last_fire > PLAYER_BULLET_CADENCE_LAZY) 
					g_player_fire();				
			}
		}
		else if (player_0.fire_mode == FIRE_MODE_PREPARE_TO_LAZY_MODE) {
			/* check if player hold down */
			if (frames - button_fire_before_last_frame > PLAYER_DELAY_FIRE_MODE_LAZY)
				player_0.fire_mode = FIRE_MODE_NORMAL;
			else if (player_0.lazy_mode_begin < frames) 
				player_0.fire_mode = FIRE_MODE_LAZY;
		}
		
		if (player_0.fire_mode == FIRE_MODE_NORMAL) {
			g_player_fire();				

			/* check if player hold down */			
			if (frames - button_fire_before_last_frame <= PLAYER_DELAY_FIRE_MODE_LAZY) {
				player_0.fire_mode = FIRE_MODE_PREPARE_TO_LAZY_MODE;
				player_0.lazy_mode_begin = frames + PLAYER_DELAY_NO_FIRE_WHEN_PREPARE;
			}			
		}
	}
	if (buttons[BUTTON_RIGHT] == BUTTON_PRESSED) { 
		buttons[BUTTON_RIGHT] = BUTTON_NO_PRESSED;
		player_0.ent.pos.x += PLAYER_ENT_X_STRIDE * slow_down ;
	}
	
	if (buttons[BUTTON_LEFT] == BUTTON_PRESSED) { 
		buttons[BUTTON_LEFT] = BUTTON_NO_PRESSED;
		player_0.ent.pos.x -= PLAYER_ENT_X_STRIDE * slow_down ;
	}

	if (buttons[BUTTON_UP] == BUTTON_PRESSED) { 
		buttons[BUTTON_UP] = BUTTON_NO_PRESSED;
		player_0.ent.pos.y -= PLAYER_ENT_Y_STRIDE * slow_down ;
	}
	
	if (buttons[BUTTON_DOWN] == BUTTON_PRESSED) { 
		buttons[BUTTON_DOWN] = BUTTON_NO_PRESSED;
		player_0.ent.pos.y += PLAYER_ENT_Y_STRIDE * slow_down ;
	}	
#ifdef WII
	{
		player_0.ent.pos.x += PLAYER_ENT_X_STRIDE * nunchuk_x * slow_down;
		player_0.ent.pos.y += PLAYER_ENT_Y_STRIDE * nunchuk_y * slow_down;
	}
#endif			
	
}


static void g_enemy_init_move(enemy_t *enemy, int move_index)
{
	move_t *move;

	enemy->move_current = move_index;	
	move = enemy->move_desc->moves[move_index];
	switch(move->type) {	
		vector_t target;
		vector_t source;		
		double length;
		double angle;

	case MOVE_RECTILINE:
		target.x = move->m.rectiline.pos.x;
		target.y = move->m.rectiline.pos.y;
		// PRINTF("target.y:%f\n", target.y);
		source.x = enemy->ent.pos.x;
		source.y = enemy->ent.pos.y;
		m_vector_substract(&target, &source, &target);
		length = m_vector_length(&target);
		m_vector_normalize(&target, &target);
		enemy->dx = move->speed * target.x;
		enemy->dy = move->speed * target.y;			
		enemy->move_end = (int) frames + length / move->speed + 1;
		
		/* compute angle of rotation */
		angle = m_angle_rot_Y(target.x, target.y);
		if (enemy->surface_rot)
			stuff_free_surface(enemy->surface_rot);
		if (enemy->surface_touch_rot)
			stuff_free_surface(enemy->surface_touch_rot);
		enemy->surface_rot = stuff_rotate(enemy->desc->surface_init, angle);
		enemy->surface_touch_rot = stuff_rotate(enemy->desc->surface_when_touch, angle);
	}
}

void g_bullet_update_trace(bullet_t *b)
{
	trace_t *t;
	
	t = b->bullet->trace;
	if (t->type == TRACE_CIRCULAR) {
		tr_circular_t *c;
		vector_t v;
		vector_t v_source;
		if (t->tr.circular.rot_center == CENTER_FIX) {	       
			v_source.x = b->rot_center.pos_fix.x;
			v_source.y = b->rot_center.pos_fix.y;
			v.x = b->ent.pos.x;
			v.y = b->ent.pos.y;
			m_vector_substract(&v, &v_source, &v);			
		}
		else {
			v_source.x = b->rot_center.pos_enemy->x;
			v_source.y = b->rot_center.pos_enemy->y;
			v.x = b->rot_center.relative_enemy.x;
			v.y = b->rot_center.relative_enemy.y;
		}
		m_vector_normalize(&v, &v);
		c = &t->tr.circular;
		b->dx = b->length * (c->rot_x[0] * v.x + c->rot_x[1] * v.y);
		b->dy = b->length * (c->rot_y[0] * v.x + c->rot_y[1] * v.y);
		b->length += c->speed;
		b->ent.pos.x = v_source.x + b->dx;
		b->ent.pos.y = v_source.y + b->dy;
		
		// save relative position of the bullet
		b->rot_center.relative_enemy.x = b->dx;
		b->rot_center.relative_enemy.y = b->dy;
	}
	else if (t->type == TRACE_RECTILINE || t->type == TRACE_AUTOAIM) {
		b->ent.pos.x += b->dx;
		b->ent.pos.y += b->dy;
	}
}

/* return TRUE si l'enemie est en dehors de l'ecran */
static int g_enemy_out_screen(enemy_t *e)
{
	return c_out_screen(&e->ent.pos, e->desc->surface_init);
}

/* return TRUE si la bullet est en dehors de l'ecran */
static int g_bullet_out_screen(bullet_t *b)
{
	return c_out_screen(&b->ent.pos, b->bullet->type->surface);
}

static void g_change_music(void *m)
{
	stuff_music_stop();	
	music = m;
	stuff_music_play(music, 1, g_music_finish_hook);
}

/* clean all structure with current boss */
static void g_boss_destroy(void)
{
	boss_current->show_life = FALSE;
	screen_y_stride = SCROLLING_VERTICAL_SPEED;	
        boss_current->enemy.live = -1;
	g_enemy_die(&boss_current->enemy);
	boss_current = NULL;
}


static void g_handle_fire_enemy(enemy_t *e)
{
	int i;
	/* check if enemy is touch */
	if (e->touch) {
		if (e->touch_begin + ENEMY_DELAY_TOUCH < frames) 
			e->touch = FALSE;
	}
			
	/* check if enemy can fire */
	for(i=0; i < e->desc->wave_count; i++) {
		wave_t *wave;
		wave_desc_t *wave_desc;
				
		wave = &e->waves[i];
		wave_desc = e->desc->wave_desc[i];
		if (frames - wave->last_fire > wave_desc->delay) {
			int j;
			bullet_desc_t *b_desc;
			bullet_t *b;

			// PRINTF("wave fire !\n");
			wave->last_fire = frames;					
			for (j=0; j<wave_desc->bullet_count; j++) {
				b = (bullet_t *)mem_alloc(sizeof *b);
				b_desc = wave_desc->bullets[j];
				b->id = bullet_id++;
				b->ent.pos.x = e->ent.pos.x + b_desc->pos.x;
				b->ent.pos.y = e->ent.pos.y + b_desc->pos.y;
				b->ent.mask_collision = &bullet_mask;
				b->bullet = b_desc;
				b->length = m_length_btw_two_points(b->ent.pos.x, b->ent.pos.y,
								    e->ent.pos.x, e->ent.pos.y);
				g_bullet_init_trace(b, &e->ent);
				list_add(g_bullets, &b->id, b);
			} // end of each bullet				
		} // end of fire of wave			

	} // end of each wave
}

static void g_initialize_wave(enemy_t *e)
{
	int nb_waves;
	int i;

	/* creation d'un context pour chaque wave */
	nb_waves = e->desc->wave_count;
	if (nb_waves)
		e->waves = (wave_t *)mem_alloc(nb_waves * sizeof(wave_t));	
	for(i=0; i<nb_waves; i++) {
		wave_t *w;
		
		w = &e->waves[i];
		w->last_fire = 0;
	}
}

/* manage boss_current */
static void g_manage_boss(void)
{
	enemy_part_t *p;
	p = &boss_current->enemy_part[boss_current->enemy_part_current];

	/* etat WAITING_TRIGGER_END */
	if (boss_current->transformation_stat == TRANSFORMATION_WAIT_TRIGGER_END) {
		if (screen_y >= boss_current->trigger_y_end) {
			/* block scrolling */
			PRINTF("BLOCK SCROLLING !\n");
			screen_y_stride = 0;
			boss_current->enemy.live = boss_current->enemy.desc->initial_life;
			boss_current->transformation_stat = TRANSFORMATION_MSG_START;
			boss_current->transformation_trigger = TRANSFORMATION_FIGHT;
			boss_current->transformation_trigger_frame = frames + BOSS_DELAY_TRANS_MSG_START;
			boss_current->show_life = TRUE;
		}
		else {
			/* scroll enemy to bottom */
			boss_current->pos.y += BOSS_SCROLL_STRIDE;
		}
	}
	
	/* etat MSG_START */
	if (!boss_current->msg_current && boss_current->transformation_stat == TRANSFORMATION_MSG_START) {
		/* check if there is a message start to show */     		
		if (*p->message_start) {
			/* there is a message */
			PRINTF("MESSAGE START\n");
			boss_current->msg_current = stuff_font_render(boss_font, p->message_start,
								      BOSS_FONT_COLOR);
		}
		else {
			/* force le passage a l'etat suivant : FIGHT */			
			boss_current->transformation_stat = TRANSFORMATION_FIGHT;
			boss_current->transformation_trigger = TRANSFORMATION_MSG_END;
			boss_current->transformation_trigger_frame = frames + p->delay;
			
		}
	}

	/* etat MSG_END */
	if (!boss_current->msg_current && boss_current->transformation_stat == TRANSFORMATION_MSG_END) {
		/* check if there is a message start to show */     
		if (*p->message_end) {
			PRINTF("MESSAGE END\n");
			/* there is a message */
			boss_current->msg_current = stuff_font_render(boss_font, p->message_end,
								      BOSS_FONT_COLOR);
		}
		else {
			/* force le passage a l'etat suivant : DYING */			
			boss_current->transformation_stat = TRANSFORMATION_DYING;
			boss_current->transformation_trigger = TRANSFORMATION_MSG_START;
			boss_current->transformation_trigger_frame = frames + BOSS_DELAY_TRANS_DYING;
		}
	}

	/* etat DYING */
	if (boss_current->transformation_stat == TRANSFORMATION_DYING) {
		/* make explosion, play son dying */
		PRINTF("DYING\n");
	}
	
	/* etat FIGHT */
	if (boss_current->transformation_stat == TRANSFORMATION_FIGHT) {
		/* do special stuff */
		/* si il y a plus d'energie alors passage en mode MSG_END */
		boss_current->live -=  boss_current->live_part_last_frame - boss_current->enemy.live;
		boss_current->live_part_last_frame = boss_current->enemy.live;			
		boss_barre_life.w = BOSS_BARRE_LIFE_WIDTH_MAX * boss_current->live / boss_current->live_max;
		PRINTF("PART LIFE %d TOTAL LIFE: %d\n", boss_current->enemy.live, boss_current->live);
		if (boss_current->enemy.live < 10 || boss_current->live < 1 ) {
			boss_current->transformation_stat = TRANSFORMATION_MSG_END;
			boss_current->transformation_trigger = TRANSFORMATION_DYING;
			boss_current->transformation_trigger_frame = frames + BOSS_DELAY_TRANS_MSG_END;
		}		
	}

	/* check if it's time to change stat */
	if (boss_current->transformation_stat != TRANSFORMATION_WAIT_TRIGGER_END &&
	    boss_current->transformation_trigger_frame <= frames ) {
		/* if there is a text load, free it */
		if (boss_current->msg_current) {
			stuff_free_surface(boss_current->msg_current);
			boss_current->msg_current = NULL;
		}
		/* check if we must change transformation */
		if (boss_current->transformation_trigger < boss_current->transformation_stat) {			
			/* check if there is a next transformation */
			boss_current->enemy_part_current++;		       
			
			if (boss_current->enemy_part_current >= boss_current->enemy_part_count) {
				/* boss is full destroy */
				char path[PATH_LENGTH_MAX];				

				sprintf(path, "%s%s", MUSIC_PATH, boss_current->music_after.desc->file);
				boss_current->music_after.music = stuff_music_load(path);
				g_change_music(boss_current->music_after.music);
				g_boss_destroy();				
				/* check if last boss */
				if (!boss_next) {
					/* end of the game */
					g_credit();
				}
				return ;			
			}						
			/* clear memory allocated for waves, the last will be desallocated automaticaly */
			if (boss_current->enemy.desc->wave_count) {
				mem_free(boss_current->enemy.waves);
			}
			/* boss loss energy */
			boss_current->live -= boss_current->enemy.live;
			/* change transformation */
			p = &boss_current->enemy_part[boss_current->enemy_part_current];
			boss_current->enemy.desc = p->desc;
			boss_current->live_part_last_frame = p->desc->initial_life;
			
			/* initial energy of part*/
			boss_current->enemy.live = p->desc->initial_life;
			g_initialize_wave(&boss_current->enemy);
		}
		boss_current->transformation_stat = boss_current->transformation_trigger;		

		/* compute the next trigger and when */
		boss_current->transformation_trigger++;
		if (boss_current->transformation_trigger >= NB_TRANSFORMATION_MAX) 
			boss_current->transformation_trigger = TRANSFORMATION_MSG_START;
		boss_current->transformation_trigger_frame = frames;
		switch(boss_current->transformation_trigger) {
		case TRANSFORMATION_MSG_START:
			boss_current->transformation_trigger_frame += BOSS_DELAY_TRANS_DYING;
			break;
		case TRANSFORMATION_FIGHT:
			boss_current->transformation_trigger_frame += BOSS_DELAY_TRANS_MSG_START;
			break;
		case TRANSFORMATION_MSG_END:
			boss_current->enemy.ent.pos.x = boss_current->pos.x;
			boss_current->enemy.ent.pos.y = -boss_current->pos.y + screen_y - boss_current->trigger_y;		
			boss_current->transformation_trigger_frame += p->delay;
			break;
		case TRANSFORMATION_DYING:
			boss_current->transformation_trigger_frame += BOSS_DELAY_TRANS_MSG_END;
			break;
		}
	}
	boss_current->enemy.ent.pos.x = boss_current->pos.x;
	boss_current->enemy.ent.pos.y = boss_current->pos.y;
}


static void g_initialize_enemy(enemy_t *e, int init_move)
{
	enemy_desc_t *desc;

	desc  = e->desc;
	e->id = enemy_id++;	
	e->ent.id = entity_id++;
	e->ent.type = ENTITY_COMPUTER;

	e->ent.mask_collision = desc->mask_collision;
	e->live = desc->initial_life;
	e->popup = TRUE;
	e->touch = FALSE;

	g_initialize_wave(e);
	if (init_move)
		g_enemy_init_move(e, 0);
}

void g_compute_physic(void)
{
	enemy_t *e;
	bullet_t *b;
	sprite_t *s;

	screen_y += screen_y_stride;
	// printf("screen_y : %d %d\n", screen_y, enemy_next->trigger_y);

	/* check si un boss doit pop-up */
	if (!boss_current && boss_next && screen_y >= boss_next->trigger_y) { 
		char path[PATH_LENGTH_MAX];
		enemy_t *e;
		int i;

		PRINTF("POP Boss! \n");

		boss_current = boss_next; 
		boss_current->enemy_part_current = 0;
		boss_current->transformation_stat = TRANSFORMATION_WAIT_TRIGGER_END;
		boss_current->msg_current = NULL;
		boss_current->pos.x = boss_current->pos_initial.x;
		boss_current->pos.y = boss_current->pos_initial.y;
	
		e = &boss_current->enemy;
		e->dx = e->dy = 0;
		e->move_end = MAX_NUMBER;
		e->desc = boss_current->enemy_part->desc;
		e->ent.pos.x = boss_current->pos.x;
		e->ent.pos.y = -boss_current->pos.y + screen_y - boss_current->trigger_y;
		e->surface_rot = NULL;
		e->surface_touch_rot = NULL;
		g_initialize_enemy(e, FALSE);		
		list_add(g_level->enemy, &boss_current->enemy.id, &boss_current->enemy);

		boss_current->live_max = 0;
		for(i=0; i < boss_current->enemy_part_count; i++) {
			enemy_part_t *p;
			
			p = &boss_current->enemy_part[i];
			boss_current->live_max += p->desc->initial_life - BOSS_LIFE_MIN;
		}
		boss_current->live = boss_current->live_max;
		boss_current->live_part_last_frame = e->desc->initial_life;
		boss_current->show_life = FALSE;
		boss_current->enemy.live = MAX_NUMBER;

		boss_barre_life.pos.x = BOSS_BARRE_LIFE_X;
		boss_barre_life.pos.y = BOSS_BARRE_LIFE_Y;
		boss_barre_life.w = BOSS_BARRE_LIFE_WIDTH_MAX;
		boss_barre_life.h = BOSS_BARRE_LIFE_HEIGHT_MAX;

		/* load new music */
		sprintf(path, "%s%s", MUSIC_PATH, boss_current->music_current.desc->file);
		boss_current->music_current.music = stuff_music_load(path);				
		g_change_music(boss_current->music_current.music);

		/* change boss_next */
		boss_next = (boss_t *)list_parcour_next(g_boss);		
	}

	/* evoluer le boss */
	if (boss_current) {
		g_manage_boss();
	}

	/* check si un enemy doit pop-up */
	if (enemy_next && screen_y >= enemy_next->trigger_y) {		
		enemy_next->ent.pos.x = enemy_next->pos_init.x;
		enemy_next->ent.pos.y = -enemy_next->pos_init.y + screen_y - enemy_next->trigger_y;
		g_initialize_enemy(enemy_next, TRUE);

		list_add(g_level->enemy, &enemy_next->id, enemy_next);
		enemy_next = (enemy_t *)list_parcour_next(g_level->enemy_sort);
	}

	/* evolue l'enemie */
	list_parcour_init(g_level->enemy);
	while ((e = list_parcour_next(g_level->enemy))) {
		if (e->move_end > frames) {
			e->ent.pos.x += e->dx;
			e->ent.pos.y += e->dy;
		}
		else {
			/* change de mouvement */
			e->move_current++;
			if (e->move_desc->move_count > e->move_current) {
				/* we can change the movement */
				g_enemy_init_move(e, e->move_current);
			}
			else {
				/* final mouvement */
				e->dx = 0;
				e->dy = 0;
				e->move_end = MAX_NUMBER;
			}				
		}
		
		/* check whether enemy is out-bound of the screen */
		if (g_enemy_out_screen(e)) {
			e->out_screen++;	
			if (FRAME2SEC(e->out_screen) >= ENEMY_TIMEOUT_OUT_SCREEN_SEC) {
				e->live = -1;
				g_enemy_remove(e);
			}
		}
		else {
			e->out_screen = 0;
			g_handle_fire_enemy(e);
			if (g_check_collision_enemy_with_player(e))
				g_player_die();
			
			    
		}
	} // end of each enemy	
	
	/* update enemies's bullets */
	list_parcour_init(g_bullets);	
	while((b = list_parcour_next(g_bullets))) {
		g_bullet_update_trace(b);
		
		if (g_bullet_out_screen(b)) { 
			//PRINTF("Remove bullet no %d\n", b->id);
			list_parcour_rm_current(g_bullets);
			mem_free(b);
			continue;
		}
		
		/* check collision whith player */
		if (g_check_collision_enemy_bullet(b)) {
			g_player_die();
			list_parcour_rm_current(g_bullets);
			mem_free(b);
		}
	}		

	/* update player's bullets */
	list_parcour_init(g_bullets_player);	
	while((b = list_parcour_next(g_bullets_player))) {
		enemy_t *e;

		g_bullet_update_trace(b);
		
		if (g_bullet_out_screen(b)) { 
			PRINTF("Remove bullet no %d\n", b->id);
			list_parcour_rm_current(g_bullets_player);
			mem_free(b);
			continue;
		}
		
		/* check collision with enemy */
		if ((e = g_check_collision_player_bullet(b))) {
			g_enemy_die(e);
			PRINTF("TOUCH ENEMY %d\n", e->id);
			PRINTF("Remove bullet no %d\n", b->id);
			list_parcour_rm_current(g_bullets_player);
			mem_free(b);
		}
	}		

	/* check position of player */
	if (player_0.ent.pos.x - player_0.ent.surface->w/2 < 0 ) 
		player_0.ent.pos.x = player_0.ent.surface->w/2;
	if (player_0.ent.pos.x + player_0.ent.surface->w/2 > SCREEN_WIDTH) 
		player_0.ent.pos.x = SCREEN_WIDTH - player_0.ent.surface->w/2;
	if (player_0.ent.pos.y - player_0.ent.surface->h/2 < PLAYER_Y_MIN ) 
		player_0.ent.pos.y = PLAYER_Y_MIN + player_0.ent.surface->h/2;
	if (player_0.ent.pos.y + player_0.ent.surface->h/2 > PLAYER_Y_MAX) 
		player_0.ent.pos.y = PLAYER_Y_MAX - player_0.ent.surface->h/2;
	
	/* check if player is immortal */	
	if (player_0.immortal) {
		player_0.immortal_begin++;
		if (player_0.immortal_begin >= PLAYER_IMMORTAL_TIME_SEC) {
			player_0.immortal = FALSE;
			player_0.visible = TRUE;
		}
		else {
			if (player_0.immortal_frame_visible >= frames) {
				player_0.visible ^= TRUE;
				player_0.immortal_frame_visible = frames +
					player_immortal_time[player_0.visible];
			}
		}
	}
	
	/* evolue les sprites */
	list_parcour_init(g_sprites_context);
	while ((s = list_parcour_next(g_sprites_context))) {
		if (s->next_frame <= frames) {
			s->frame_current++;
			if (s->frame_current >= s->desc->frame_count) {
				/* remove sprite */
				list_parcour_rm_current(g_sprites_context);
			}
			else {
				/* change frame */
				s->next_frame = frames + s->desc->delay;
				s->frame++;
			}
		       
		}
	}
}

static void g_draw_score(void)
{
	surface_t *score;
	char score_str[32];

	snprintf(score_str, 32, "%d", player_0.score);	
	score = stuff_font_render(score_font, score_str, PLAYER_SCORE_COLOR);
	r_draw_surface(score, PLAYER_SCORE_POS_X, PLAYER_SCORE_POS_Y);
	stuff_free_surface(score);
}

static void g_render_enemy(enemy_t *e)
{	
	surface_t *surface;
	if (e->touch) {
		surface = e->surface_touch_rot ? e->surface_touch_rot : e->desc->surface_when_touch;
		r_draw_surface(surface,
			       e->ent.pos.x - surface->w/2,
			       e->ent.pos.y - surface->h/2);
	}
	else {
		surface = e->surface_rot ? e->surface_rot : e->desc->surface_init;
		r_draw_surface(surface, 
			       e->ent.pos.x - surface->w/2,
			       e->ent.pos.y - surface->h/2);
	}
}

void g_render_scene(void)
{
	enemy_t *e;
	bullet_t *b;
	sprite_t *s;
	int life;
	int i;

	background_clip.pos.y = background->h - screen_y;
	r_draw_background(background, &background_clip);
		
	/* draw enemies */
	list_parcour_init(g_level->enemy);
	while ((e = list_parcour_next(g_level->enemy))) {
		g_render_enemy(e);
	}

	/* draw sprites */
	list_parcour_init(g_sprites_context);
	while ((s = list_parcour_next(g_sprites_context))) {	
		PRINTF("draw sprite %d %d (%d; %d)\n", s->id, s->frame_current, (int)s->pos.x, (int)s->pos.y);
		r_draw_surface(s->frame,
			       s->pos.x - s->frame->w/2,
			       s->pos.y - s->frame->h/2);
	}

	/* draw player */
	if (player_0.visible) {
		/* draw canon */
		r_draw_surface(player_0.canon_rot,
			       player_0.ent.pos.x - player_0.canon_rot->w/2,
			       player_0.ent.pos.y - player_0.canon_rot->h/2);
		   
		r_draw_surface(player_0.surface_rot,
			       player_0.ent.pos.x - player_0.ent.surface->w/2 + player_0.pos_padding.x,
			       player_0.ent.pos.y - player_0.ent.surface->h/2 + player_0.pos_padding.y);
		
	}
	
	/* draw enemies's bullets */
	list_parcour_init(g_bullets);
	while((b = list_parcour_next(g_bullets))) {
		r_draw_surface(b->bullet->type->surface, 
			       b->ent.pos.x - b->bullet->type->surface->w/2,
			       b->ent.pos.y - b->bullet->type->surface->h/2);
	}       
	
	/* draw player's bullets */
	list_parcour_init(g_bullets_player);
	while((b = list_parcour_next(g_bullets_player))) {
		r_draw_surface(b->bullet->type->surface, 
			       b->ent.pos.x - b->bullet->type->surface->w/2,
			       b->ent.pos.y - b->bullet->type->surface->h/2);
	}       

	/* draw player's crosshair */
	r_draw_surface(player_0.crosshair, 
		       player_0.crosshair_pos.x -player_0.crosshair->w/2,
		       player_0.crosshair_pos.y - player_0.crosshair->h/2);

	/* draw live */
	life = player_0.live;
	for (i=0; i<life; i++) 
		r_draw_surface(player_life, PLAYER_LIFE_POS_X + i * PLAYER_LIFE_STRIDE, PLAYER_LIFE_POS_Y);
	
	/* draw score */
	g_draw_score();

	if (boss_current) {		
		/* draw barre de vie du boss */
		if (boss_current->show_life)
			stuff_fill_rect(&boss_barre_life, BOSS_BARRE_COLOR);

		/* draw boss's message */
		if (boss_current->msg_current) {
			r_draw_surface(boss_current->msg_current, BOSS_TEXT_POS_X, BOSS_TEXT_POS_Y);
		}
	}

	/* draw gameover */
	if (gameover) {
		r_draw_surface(gameover_surface, GAMEOVER_POS_X, GAMEOVER_POS_Y);
	}
	
	/* draw pause */
	if (pause) {
		r_draw_surface(pause_upper_surface, PAUSE_UPPER_POS_X, PAUSE_UPPER_POS_Y);
		r_draw_surface(pause_down_surface, PAUSE_DOWN_POS_X, PAUSE_DOWN_POS_Y);
	}	
}

void g_finalize(void)
{
	void *p;
	sprite_t *s;

	if (boss_current) {
		g_boss_destroy();
	}
		
	/* kill all bullets */
	
	list_parcour_init(g_bullets);
	while ((p = list_parcour_next(g_bullets))) {
		list_parcour_rm_current(g_bullets);
		mem_free(p);
	}	
	
	list_parcour_init(g_bullets_player);
	while ((p = list_parcour_next(g_bullets_player))) {
		list_parcour_rm_current(g_bullets_player);
		mem_free(p);
	}

	/* kill all enemies */	
	list_parcour_init(g_level->enemy);
	while ((p = list_parcour_next(g_level->enemy))) {
		enemy_t *e;

		list_parcour_rm_current(g_level->enemy);
		e = (enemy_t *)p;
		mem_free(e->waves);
	}
	
	/* kill all sprites */
	list_parcour_init(g_sprites_context);
	while ((s = list_parcour_next(g_sprites_context))) {
		list_parcour_rm_current(g_sprites_context);
	}	
}
