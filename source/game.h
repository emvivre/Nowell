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
#include "list.h"

/*************
 * ennemy, entity, bullet
 ****************************/

#define DEFAULT_LENGTH 32
#define TRACE_NAME_MAX DEFAULT_LENGTH
#define MOVE_NAME_MAX DEFAULT_LENGTH
#define MOVE_DESC_NAME_MAX DEFAULT_LENGTH
#define BULLET_TYPE_NAME_MAX DEFAULT_LENGTH
#define BULLET_NAME_MAX DEFAULT_LENGTH
#define WAVE_NAME_MAX DEFAULT_LENGTH
#define ENEMY_NAME_MAX DEFAULT_LENGTH
#define LEVEL_NAME_MAX DEFAULT_LENGTH
#define MASK_NAME_MAX DEFAULT_LENGTH
#define BOSS_NAME_MAX DEFAULT_LENGTH
#define SPRITE_NAME_MAX DEFAULT_LENGTH
#define MUSIC_DESC_NAME_MAX DEFAULT_LENGTH
#define MUSIC_FILE_NAME_MAX DEFAULT_LENGTH
#define MESSAGE_LENGTH_MAX 1024


#define MASK_WIDTH_MAX 128
#define MASK_HEIGHT_MAX 128
#define MASK_BUFFER_MAX (MASK_WIDTH_MAX * MASK_HEIGHT_MAX + MASK_HEIGHT_MAX)

typedef struct mask_s
{
	char name[MASK_NAME_MAX];
	surface_t surface;
} mask_t;

enum {ENTITY_PLAYER,
      ENTITY_COMPUTER,
      MAX_ENTITY_TYPE
};

typedef struct entity_s 
{
	int id; // identifiant de l'entite
	int type; // ENTITY_PLAYER ou ENTITY_COMPUTER
	pos_t pos; // position de l'entite dans le monde 2D
	surface_t *surface; // la surface pour le dessiner, width, height
	pos_t heart_pos; // position de son coeur (point faible) le centre
	mask_t *mask_collision; // masque de collision
} entity_t;


enum {
	FIRE_MODE_NORMAL,
	FIRE_MODE_PREPARE_TO_LAZY_MODE,
	FIRE_MODE_LAZY
};

typedef struct player_s
{	
	pos_t pos_padding; // padding pour la position
	entity_t ent; // position, width, height
        int live; // nombre de vie
	unsigned int n_bomb; // number of bomb (i.e. gift)
	unsigned int last_fire; // last frame where player begin to fire
	int immortal; /* if the player is immortal, e.g. when he born */
	unsigned int immortal_begin; // number of the successive frame that player is immortal
	int visible; // si a TRUE alors player est visible
	unsigned int immortal_frame_visible; // prochain changement d'etat entre visible ou pas pour l'immortal
	int fire_mode; // si il tire normal ou mode arrosage
	unsigned int lazy_mode_begin; // futur frame when player will be in lazy mode if he continue to hold down
	unsigned int score; // score du joueur
	surface_t *crosshair;  // surface of the crosshair
	surface_t *surface_rot; // surface with rotation
	surface_t *canon; // canon du player
	surface_t *canon_rot;
	double canon_angle; // angle que fait le canon par rapport a l'axe Y
	int angle_surface_rot; // current angle of rotation 
	pos_t crosshair_pos; // position of the crosshair
	vector_t crosshair_vector; // normalized vector btw player and crosshair
} player_t;

// trace fire
enum { 	
	TRACE_DONT_CARE, // random
	TRACE_AUTOAIM, // trace auto_aim (rectiline) to the player
	TRACE_RECTILINE, // trace make a line 
	TRACE_CIRCULAR, // trace make a circle (do a rotation)
	MAX_TRACE
};

typedef struct tr_autoaim_s {	
	double dx; // calculer a partir de la position du player en faisant un dotProduct
	double dy; // correspond a une velocity de 1
	double speed; // velocity
} tr_autoaim_t;

typedef struct tr_rectiline_s {
	double dx; // vecteur normal
	double dy; 
	double speed;
} tr_rectiline_t;

// la list des different type possible de centre de rotation
enum {
	CENTER_FIX,
	CENTER_ENEMY,
	NB_PT_ROTATION
};

typedef struct tr_circular_s {
        double rot_x[2], rot_y[2]; // coef pour la rotation
	double speed;
	int rot_center; // info sur le centre de rotation : FIX ou specifie a la position de l'ennemie
} tr_circular_t;

typedef struct {
	char name[TRACE_NAME_MAX];
	int type; // type of trace
	union {
		tr_rectiline_t rectiline;
		tr_circular_t circular;
	} tr;
} trace_t;

// description du type de bullet
typedef struct bullet_type_s {
	char name[BULLET_TYPE_NAME_MAX];
	surface_t *surface;	
} bullet_type_t;

// description d'un bullette
typedef struct bullet_desc_s {
	char name[BULLET_NAME_MAX];
	pos_t pos; // position de depart relative par rapport au centre de l'entity
	trace_t *trace; // descripteur du tracage
	bullet_type_t *type;
} bullet_desc_t;

typedef struct rot_source_s {
	pos_t *pos_enemy; // emplacement de l'ennemie courant
	pos_t relative_enemy; // position relative par rapport a l'enemy avant son deplacement
	pos_t pos_fix; // centre de rotation fixe
} rot_source_t;

// context d'un bullet 
typedef struct {
	int id;
	double dx; // delta x (context du mouvement du bullet)
	double dy; // delta y (context du mouvement du bullet)
	double length; // distance entre le bullet et son createur
	entity_t ent; // entite de bullet 	
	bullet_desc_t *bullet; // description du bullet
	rot_source_t rot_center;
} bullet_t;

// move ennemy
enum {
	MOVE_RECTILINE,
	MOVE_BEZIER,
	MAX_MOVE
};

typedef struct {
	pos_t pos; // position suivante
} move_rectiline_t;

typedef struct {	
	int x1, y1;
	int x2, y2;
} move_bezier_t;

typedef struct {
	char name[MOVE_NAME_MAX];
	int type;
	int move_relative; // si TRUE, alors la position des x indiquer dans les moves sont relatives a
	                   // la position de l'ennemie au checkpoint
	double speed; // indique la vitesse du mouvement
	union {
		move_rectiline_t rectiline;
		move_bezier_t bezier;
	} m;
} move_t;

// descripion d'une vague
typedef struct {
	char name[WAVE_NAME_MAX];
	int delay; // nombre de frame entre chaque vague
	int bullet_count; // nombre de bullet a chaque vague
	bullet_desc_t **bullets; // la description de chaque boullette devant etre creer
} wave_desc_t;

// context (l'instance) d'une vague
typedef struct {	
	int last_fire; // derniere fois que la vague a ete enclancher
	list_t *bullets; // liste chaine de context de bullet
} wave_t;

typedef struct sprite_desc_s {
	char name[SPRITE_NAME_MAX];
	int frame_count; // nombre de frames total
	int delay; // delay entre chaque frame en tick
	surface_t *frames; // tableau de frame
} sprite_desc_t;

/* definition d'un context d'un sprite */
typedef struct sprite_s {
	int id;
	pos_t pos; // position of the sprite
	int frame_current; // frame courrante du sprite
	int next_frame; // tick vers la prochaine frame
	surface_t *frame; // surface de la frame current
	sprite_desc_t *desc; // descripteur de sprite	
} sprite_t;

typedef struct enemy_desc_s {
	char name[ENEMY_NAME_MAX];
	surface_t *surface_init; // surface initial
	surface_t *surface_when_touch; // surface when enemy is touch by a player's bullet 
	mask_t *mask_collision; // masque de collision
	pos_t mask_position; // position du masque de collision
	int wave_count; // nombre de vague
	int initial_life; // point de vie au debut
	wave_desc_t **wave_desc; // la description de chaque vague	
	int score; // score add if player kill this type of enemy
	sprite_desc_t *sprite_dead; // sprite of the dead
} enemy_desc_t;

// un move_desc est un agregat de mouvement
typedef struct enemy_move_desc_s {
	char name[MOVE_DESC_NAME_MAX];
	int move_count; // nombre de movement
	move_t **moves; // description des mouvements : les differents points de positions
} enemy_move_desc_t;

typedef struct enemy_s {
	int id; // identifiant
	double dx; // delta x (context du mouvement)
	double dy; // delta y (context du mouvement)	
	int out_screen; // nombre de frame consecutive que l'enemie est en dehors de l'ecran
	entity_t ent; // entity de l'ennemie
	pos_t pos_init; // position initial de l'ennemie, le y est relative au trigger_y*10 de l'ennemie
	int trigger_y; // a partir de quel y/10 du level le context de l'ennemie est creer
	int live; // indique le nombre de points de vie
	enemy_desc_t *desc; // description de l'ennemie pour connaitre mouvement et wave
	int move_end; // contient le no de frame de fin de mouvement 
	int move_current; // movement courrant
	enemy_move_desc_t *move_desc; // point vers le descripteur des mouvements
	surface_t *surface_rot; // surface avec une rotation de tel sorte que l'enemie se trouve dans le sens
	                       // du mouvement
	surface_t *surface_touch_rot; // surface avec rotation lorque l'enemie est touche
	wave_t *waves; // le context de chaque vague, le nombre de vague est dans la structure enemy_desc_t
	unsigned int touch_begin; // frame when bullet touch this enemy
	char touch; // TRUE if enemy is touch
	char popup; // TRUE if enemy has been pop-up       
} enemy_t;


typedef struct enemy_part_s {
	unsigned int delay; // nombre de frames avant que la partie ne se suicide
	char message_start[MESSAGE_LENGTH_MAX]; // message de bienvenue
	char message_end[MESSAGE_LENGTH_MAX]; // message de fin
	enemy_desc_t *desc;	
} enemy_part_t ;


/* les etats possibles d'une transformation */
enum {
	TRANSFORMATION_WAIT_TRIGGER_END,
	TRANSFORMATION_MSG_START,
	TRANSFORMATION_FIGHT,
	TRANSFORMATION_MSG_END,
	TRANSFORMATION_DYING,
	NB_TRANSFORMATION_MAX
};

typedef struct music_desc_s{
	char name[MUSIC_DESC_NAME_MAX];
	char file[MUSIC_FILE_NAME_MAX];	
} music_desc_t;

typedef struct music {
	void *music;
	music_desc_t *desc;
} music_t;

typedef struct boss_s {
	char name[BOSS_NAME_MAX];
	pos_t pos;
	pos_t pos_initial;
	unsigned int trigger_y;
	unsigned int trigger_y_end; // end of scrolling
	unsigned int enemy_part_count; // nombre de transformation de l'ennemie
	unsigned int enemy_part_current; // numero de la transfomartion actuel
	int transformation_stat; // etat de la transformation 
	int transformation_trigger; // changement d'etat de la transformation, le futur etat
	unsigned int transformation_trigger_frame; // no de la frame futur, pour changer d'etat
	surface_t *msg_current; // message courrant a afficher;
	enemy_part_t *enemy_part;
	enemy_t enemy; // context du boss
	int live; // vie courrant du boss
	int live_part_last_frame; // energie de la partie dans la derniere frame
	int live_max;  // vie max du boss
	char show_life; // montre bar de vie
	music_t music_current; // music in boss
	music_t music_after; // music after boss
} boss_t;

typedef struct level_s {
	char name[LEVEL_NAME_MAX]; // nom du level
	surface_t *background; // le background 

	int enemy_total; // nombre d'enemy total dans le level
	int enemy_current; // nombre d'enemy creer pour le moment
	list_t *enemy_sort; // prochain enemie trier par les y       
	list_t *enemy; // list d'instance d'enemy 	
} level_t;

#define PLAYER_BULLET_DESC_SNOWBALL "player_snowball"

extern bullet_desc_t *bullet_desc_snowball;
extern level_t *g_level; // le level courrant
extern list_t *g_traces; // list des traces
extern list_t *g_bullets; 
extern list_t *g_bullets_desc; 
extern list_t *g_bullets_type; 
extern list_t *g_waves_desc; 
extern list_t *g_moves; // list des mouvements 
extern list_t *g_enemy_moves_desc; // list d'agrgat de mouvements 
extern list_t *g_masks;
extern list_t *g_enemies_desc;
extern list_t *g_enemies;
extern list_t *g_entities;
extern list_t *g_boss;
extern list_t *g_sprites;
extern list_t *g_musics;

// return TRUE si le player peut faire feu
// int g_player_can_fire(player_t *p);

// return TRUE si l'ennemy peut faire feu
// int g_enemy_can_fire(enemy_t *e);

// update les coordonnes de l'entite en fonction du mouvement
// void g_update_ent(entity_t *ent, move_t *move);

// game.c 
void game_allocation(void);
void g_init(void);
void g_handle_player(void);
void g_compute_physic(void);
void g_render_scene(void);
void g_finalize(void);

// initialize level from xml file
void g_level_xml(char *xml_file);



/* g_xml.c */
