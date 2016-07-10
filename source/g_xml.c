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

#include <fcntl.h>
#include <math.h>
#include <unistd.h>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include "common.h"
#include "game.h"
#include "list.h"
#include "string.h"

/* lecture du fichier xml, l'utilisation de la libxml a ete suposer suffisament portable pour ne pas avoir
 * besoin de faire un fichier source dedier au different portage possible 
 */

#define COMPARE(S, STR) !strcmp(S, STR)

static trace_t *g_search_trace(char *trace)
{
	trace_t *t;

	t = (trace_t *)list_search(g_traces, trace);
	if (!t) 
		stuff_quit_msg("Unable to find trace : %s\n", trace);
	return t;
}

static bullet_desc_t *g_search_bullet(char *bullet)
{
	bullet_desc_t *b;
	
	b = list_search(g_bullets_desc, bullet);
	if (!b) 
		stuff_quit_msg("Unable to find bullet descriptor : %s\n", bullet);
	return b;
}

static bullet_type_t *g_search_bullet_type(char *bullet)
{
	bullet_type_t *b;
	
	b= list_search(g_bullets_type, bullet);
	if (!b)
		stuff_quit_msg("Unable to find bullet type descriptor : %s\n", bullet);
	return b;
}

static move_t *g_search_move(char *move)
{
	move_t *m;
	
	m = (move_t *)list_search(g_moves, move);
	if (!m) 
		stuff_quit_msg("Unable to find move : %s\n", move);
	return m;
}

static mask_t *g_search_mask(char *mask)
{
	mask_t *m;
	
	m = (mask_t *)list_search(g_masks, mask);
	if (!m)
		stuff_quit_msg("Unable to find mask : %s\n", mask);
	return m;
}

static wave_desc_t *g_search_wave_desc(char *wave)
{
	wave_desc_t *w;
	
	w = (wave_desc_t *)list_search(g_waves_desc, wave);
	if (!w)
		stuff_quit_msg("Unable to find wave : %s\n", wave);
	return w;
}

static enemy_desc_t *g_search_enemy_desc(char *desc)
{
	enemy_desc_t *d;
	
	d = (enemy_desc_t *)list_search(g_enemies_desc, desc);
	if (!d)
		stuff_quit_msg("Unable to find enemy description : %s\n", desc);
	return d;
}

static enemy_move_desc_t *g_search_enemy_move_desc(char *move)
{
	enemy_move_desc_t *m;
	
	m = (enemy_move_desc_t *)list_search(g_enemy_moves_desc, move);
	if (!m)
		stuff_quit_msg("Unable to find enemy move description : %s\n", move);
	return m;
}

static sprite_desc_t *g_search_sprite_desc(char *sprite)
{
	sprite_desc_t *s;
	
	s = (sprite_desc_t *)list_search(g_sprites, sprite);
	if (!s)
		stuff_quit_msg("Unable to find sprite description : %s\n", sprite);
	return s;
}

static music_desc_t *g_search_music_desc(char *music)
{
	music_desc_t *m;
	
	m = (music_desc_t *)list_search(g_musics, music);
	if (!m)
		stuff_quit_msg("Unable to find music description : %s\n", music);
	return m;
}

static void g_read_trace_rectiline(xmlNode *node, trace_t *trace)
{
	trace->type = TRACE_RECTILINE;
	for (; node; node = node->next) {
		char *name;
		char *content;
		
		name = (char *)node->name;
		if (!node->children) 
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "angle")) {
			double angle;

			angle = atoi(content);
			angle = DEG2RAD(angle);
			trace->tr.rectiline.dx = sin((double)angle);
			trace->tr.rectiline.dy = cos((double)angle);
		}
		else if (COMPARE(name, "speed")) {
			int speed;
			
			speed = atoi(content);
			trace->tr.rectiline.speed = (double) speed;
		}
	}		
}

static void g_read_trace_circular(xmlNode *node, trace_t *trace)
{
	trace->type = TRACE_CIRCULAR; 
	
	// configure CENTER_FIX on default
	trace->tr.circular.rot_center = CENTER_FIX; 
	for(; node; node = node->next) {
		char *name;
		char *content;
		
		name = (char *)node->name;
		if (!node->children) 
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "speed")) {
			int speed;
			
			speed = atoi(content);
			trace->tr.circular.speed = (double) speed/100;
		}
		else if (COMPARE(name, "angle")) {
			double rotation;
						
			rotation = (double) atoi(content);
			rotation = DEG2RAD(rotation);
			trace->tr.circular.rot_y[0] = sin(rotation);
			trace->tr.circular.rot_y[1] = cos(rotation);
			trace->tr.circular.rot_x[0] = cos(rotation);
			trace->tr.circular.rot_x[1] = -sin(rotation);
		}		
		else if (COMPARE(name, "center")) {
			if (!strcmp(content, "ENEMY"))
				trace->tr.circular.rot_center = CENTER_ENEMY;
			else if (!strcmp(content, "FIX"))
				trace->tr.circular.rot_center = CENTER_FIX;
		}


	}
}

static void g_read_trace(xmlNode *node, trace_t *trace)
{
	for (; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children) 
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name")) 
			strncpy(trace->name, content, TRACE_NAME_MAX);
		else if (COMPARE(name, "type")) {
			if (COMPARE(content, "TRACE_RECTILINE"))
				g_read_trace_rectiline(node->next, trace);
			else if(COMPARE(content, "TRACE_CIRCULAR"))
				g_read_trace_circular(node->next, trace);
			else if(COMPARE(content, "TRACE_AUTOAIM")) {
				g_read_trace_rectiline(node->next, trace);
				trace->type = TRACE_AUTOAIM;
			}
		}		
	}
}

static void g_read_traces(xmlNode *node)
{
	for(; node; node=node->next) {
		if (COMPARE("trace", (char *)node->name)) {
			trace_t *tr;
			
			tr = (trace_t *) mem_alloc(sizeof *tr);
			g_read_trace(node->children, tr);
			list_add(g_traces, tr->name, tr);
		}
	}
}

static void g_read_bullet(xmlNode *node, bullet_desc_t *b)
{
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children)
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name"))
			strncpy(b->name, content, BULLET_NAME_MAX);
		else if (COMPARE(name, "x"))
			b->pos.x = atoi(content);		
		else if (COMPARE(name, "y"))
			b->pos.y = atoi(content);
		else if (COMPARE(name, "trace_name")) 
			b->trace = g_search_trace(content);
		else if (COMPARE(name, "bullet_type")) 
			b->type = g_search_bullet_type(content);			
	}
}


static void g_read_bullets(xmlNode *node)
{
	for(; node; node = node->next) {
		if (COMPARE("bullet", (char *)node->name)) {
			bullet_desc_t *b;
			
			b = (bullet_desc_t *)mem_alloc(sizeof *b);
			g_read_bullet(node->children, b);
			list_add(g_bullets_desc, b->name, b);
		}
	}
}

static void g_read_bullet_type(xmlNode *node, bullet_type_t *b)
{
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children)
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name"))
			strncpy(b->name, content, BULLET_TYPE_NAME_MAX);
		else if (COMPARE(name, "surface")) {
			char path[PATH_LENGTH_MAX];
			
			sprintf(path, "%s%s", IMAGE_PATH, content);
			b->surface = stuff_load_img(path);
		}
	}
}

static void g_read_bullets_type(xmlNode *node)
{
	for(; node; node = node->next) {
		if (COMPARE("bullet_type", (char *)node->name)) {
			bullet_type_t *b;
			
			b = (bullet_type_t *)mem_alloc(sizeof *b);
			g_read_bullet_type(node->children, b);
			list_add(g_bullets_type, b->name, b);
		}
	}
}

static void g_read_wave(xmlNode *node, wave_desc_t *w)
{
	int i = 0;
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children) 
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name"))
			strncpy(w->name, content, WAVE_NAME_MAX);
		else if (COMPARE(name, "delay"))
			w->delay = atoi(content);
		else if (COMPARE(name, "bullet_count")) {
			w->bullet_count = atoi(content);
			w->bullets = (bullet_desc_t **)mem_alloc(w->bullet_count * sizeof(bullet_desc_t *));
		}
		else if (COMPARE(name, "bullet_name")) {			
			if ( i < w->bullet_count) {
				w->bullets[i] = g_search_bullet(content);
				i++;
			}
		}
	}
}

static void g_read_waves(xmlNode *node)
{
	for(; node; node=node->next) {
		if (COMPARE((char *)node->name, "wave")) {
			wave_desc_t *w;
			
			w = (wave_desc_t *)mem_alloc(sizeof *w);
			g_read_wave(node->children, w);
			list_add(g_waves_desc, w->name, w);
		}
	}
}

static void g_read_move_rectiline(xmlNode *node, move_t *m)
{
	m->type = MOVE_RECTILINE;
	for(; node; node=node->next) {
		char *name;
		char *content;
		
		name = (char *)node->name;
		if (!node->children) 
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "x")) {
			int x;
			
			x = atoi(content);
			m->m.rectiline.pos.x = x;
		}
		else if (COMPARE(name, "y")) {
			int y;
			
			y = atoi(content);
			m->m.rectiline.pos.y = SCREEN_HEIGHT - y;
		}
	}		
}

static void g_read_move(xmlNode *node, move_t *m)
{
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children)
			continue; 
		content = (char *)node->children->content;
		if (COMPARE(name, "name"))
			strncpy(m->name, content, MOVE_NAME_MAX);
		else if (COMPARE(name, "move_relative")) {
			if (COMPARE(content, "TRUE"))
				m->move_relative = TRUE;
			else 
				m->move_relative = FALSE;
		}
		else if (COMPARE(name, "type")) {
			if (COMPARE(content, "MOVE_RECTILINE")) 
				g_read_move_rectiline(node->next, m);			
		}
		else if (COMPARE(name, "speed")) {
			m->speed = atoi(content);
		}
	}
}

static void g_read_moves(xmlNode *node)
{
	for(; node; node=node->next) {
		if (COMPARE((char *)node->name, "move")) {
			move_t *m;
			
			m = (move_t *)mem_alloc(sizeof *m);
			g_read_move(node->children, m);
			list_add(g_moves, m->name, m);
		}
	}
}

static void g_read_move_desc(xmlNode *node, enemy_move_desc_t *m)
{
	int i = 0;
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children)
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name"))
			strncpy(m->name, content, MOVE_DESC_NAME_MAX);
		else if (COMPARE(name, "count")) {
			int count;
			
			count = atoi(content);
			m->move_count = count;
			m->moves = (move_t **)mem_alloc(count * sizeof(move_t *));
		}
		else if (COMPARE(name, "move_name")) {		
			if (i < m->move_count) { 
				m->moves[i] = g_search_move(content);
				i++;
			}
		}
	}
}

static void g_read_moves_desc(xmlNode *node)
{
	for(; node; node=node->next) {
		if (COMPARE((char *)node->name, "moves")) {
			enemy_move_desc_t *m;
			
			m = (enemy_move_desc_t *)mem_alloc(sizeof *m);
			g_read_move_desc(node->children, m);
			list_add(g_enemy_moves_desc, m->name, m);
		}
	}
}

void g_read_mask(xmlNode *node, mask_t *m)
{
	for(; node; node=node->next) {
		char *name;
		char *content;		

		name = (char *)node->name;
		if (!node->children) 
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name")) 
			strncpy(m->name, content, MASK_NAME_MAX);
		else if (COMPARE(name, "width"))
			m->surface.w = atoi(content);
		else if (COMPARE(name, "height"))
			m->surface.h = atoi(content);
	}
}

static void g_read_masks(xmlNode *node)
{
	for(; node; node=node->next) {
		if (COMPARE((char *)node->name, "mask")) {
			mask_t *m;

			m = (mask_t *)mem_alloc(sizeof *m);
			g_read_mask(node->children, m);
			list_add(g_masks, m->name, m);
		}			
	}
}

static void g_read_enemy(xmlNode *node, enemy_desc_t *e)
{
	int i = 0;
	e->mask_position.x = 0;
	e->mask_position.y = 0;
	
	/* load default enemy's sprite of the dead */
	e->sprite_dead = g_search_sprite_desc("enemy_default_dead");
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children) 
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name"))
			strncpy(e->name, content, ENEMY_NAME_MAX);		
		else if (COMPARE(name, "surface")) {
			char path[PATH_LENGTH_MAX];
			
			sprintf(path, "%s%s", IMAGE_PATH, content);
			e->surface_init = stuff_load_img(path);
		}
		else if (COMPARE(name, "surface_when_touch")) {
			char path[PATH_LENGTH_MAX];
			
			sprintf(path, "%s%s", IMAGE_PATH, content);
			e->surface_when_touch = stuff_load_img(path);
		}
		else if (COMPARE(name, "life"))
			e->initial_life = atoi(content);
		else if (COMPARE(name, "score"))
			e->score = atoi(content);
		else if (COMPARE(name, "mask"))
			e->mask_collision = g_search_mask(content);
		else if (COMPARE(name, "mask_position_x"))
			e->mask_position.x = atoi(content);
		else if (COMPARE(name, "mask_position_y"))
			e->mask_position.y = atoi(content);
		else if (COMPARE(name, "wave_count")) {
			e->wave_count = atoi(content);
			e->wave_desc = (wave_desc_t **)mem_alloc(e->wave_count * sizeof(wave_desc_t *));
		}
		else if (COMPARE(name, "wave_name")) {
			if (i < e->wave_count) {
				e->wave_desc[i] = g_search_wave_desc(content);
				i++;
			}
		}
	}
}

static void g_read_enemies(xmlNode *node)
{
	for(; node; node=node->next) {	
		if (COMPARE((char *)node->name, "enemy")) {
			enemy_desc_t *e;
			
			e = (enemy_desc_t *)mem_alloc(sizeof *e);
			g_read_enemy(node->children, e);
			list_add(g_enemies_desc, e->name, e);
		}
	}
}

static void g_read_entity(xmlNode *node, enemy_t *e)
{
	e->popup = FALSE;
	e->surface_rot = NULL;
	e->surface_touch_rot = NULL;
	
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children)  
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "enemy")) 
			e->desc = g_search_enemy_desc(content);
		else if (COMPARE(name, "trigger_y"))
			e->trigger_y = atoi(content);
		else if (COMPARE(name, "move_desc"))
			e->move_desc = g_search_enemy_move_desc(content);
		else if (COMPARE(name, "x"))
			e->pos_init.x = atoi(content);
		else if (COMPARE(name, "y"))
			e->pos_init.y = atoi(content);
	}
}

static void g_read_entities(xmlNode *node)
{
	for(; node; node=node->next) {
		if (COMPARE((char *)node->name, "entity")) {
			enemy_t *e;

			e = (enemy_t *)mem_alloc(sizeof *e);
			e->touch = FALSE;
			g_read_entity(node->children, e);
			list_add(g_enemies, &e->trigger_y, e);
		}
	}
	
}

static void g_read_enemy_part(xmlNode *node, enemy_part_t *e)
{
	e->message_start[0] = '\0';
	e->message_end[0] = '\0';	
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children)  
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "delay")) {
			e->delay = atoi(content);		
			e->delay = SEC2FRAME(e->delay);
		}
		else if (COMPARE(name, "msg_start")) {
			strncpy(e->message_start, content, MESSAGE_LENGTH_MAX);
		}
		else if (COMPARE(name, "msg_end"))
			strncpy(e->message_end, content, MESSAGE_LENGTH_MAX);
		else if (COMPARE(name, "enemy"))
			e->desc = g_search_enemy_desc(content);
	}
}

static void g_read_boss(xmlNode *node, boss_t *b)
{
	int i = 0;
	b->enemy_part_current = 0;
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children)  
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name")) 
			strncpy(b->name, content, BOSS_NAME_MAX);
		else if (COMPARE(name, "music"))
			b->music_current.desc = g_search_music_desc(content);
		else if (COMPARE(name, "music_after")) 
			b->music_after.desc = g_search_music_desc(content);
		else if (COMPARE(name, "x"))
			b->pos_initial.x = atoi(content);
		else if (COMPARE(name, "y"))
			b->pos_initial.y = atoi(content);
		else if (COMPARE(name, "trigger_y"))
			b->trigger_y = atoi(content);
		else if (COMPARE(name, "trigger_y_end"))
			b->trigger_y_end = atoi(content);
		else if (COMPARE(name, "enemy_part_count")) {
			b->enemy_part_count = atoi(content);
			b->enemy_part = (enemy_part_t *)mem_alloc(b->enemy_part_count * sizeof(enemy_part_t));
		}
		else if (COMPARE(name, "enemy_part")) {
			if (i >= b->enemy_part_count ) 
				stuff_quit_msg("Unexpected enemey part : current : %i max : %i\n", i,
					       b->enemy_part_count);
			g_read_enemy_part(node->children, &b->enemy_part[i]);
			i++;
		}
	}
}

static void g_read_bosses(xmlNode *node)
{
	for(; node; node=node->next) {
		if (COMPARE((char *)node->name, "boss")) {
			boss_t *b;

			b = (boss_t *)mem_alloc(sizeof *b);
			g_read_boss(node->children, b);
			list_add(g_boss, &b->trigger_y, b);
		}
	}	
}

static void g_read_music(xmlNode *node, music_desc_t *music)
{
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children)  
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name")) 
			strncpy(music->name, content, MUSIC_DESC_NAME_MAX);
		else if (COMPARE(name, "file"))
			strncpy(music->file, content, MUSIC_FILE_NAME_MAX);
	}
}

static void g_read_musics(xmlNode *node)
{
	for(; node; node=node->next) {
		if (COMPARE((char *)node->name, "music")) {
			music_desc_t *m;

			m = (music_desc_t *)mem_alloc(sizeof *m);
			g_read_music(node->children, m);
			list_add(g_musics, m->name, m);
		}
	}	
}

static void g_read_sprite(xmlNode *node, sprite_desc_t *s)
{
	int i = 0;
	for(; node; node=node->next) {
		char *name;
		char *content;
			
		name = (char *)node->name;
		if (!node->children)  
			continue;
		content = (char *)node->children->content;
		if (COMPARE(name, "name")) 
			strncpy(s->name, content, SPRITE_NAME_MAX);
		else if (COMPARE(name, "delay")) 
			s->delay = atoi(content);
		else if (COMPARE(name, "frame_count")) {
			s->frame_count = atoi(content);
			s->frames = (surface_t *)mem_alloc(s->frame_count * sizeof(*s->frames));
		}
		else if (COMPARE(name, "frame")) {
			char path[PATH_LENGTH_MAX];
			surface_t *t;
			
			if (i >= s->frame_count ) 
				stuff_quit_msg("Unexpected frame : current : %i max : %i\n", i,
					       s->frame_count);
			sprintf(path, "%s%s", IMAGE_PATH, content);
			t = stuff_load_img(path);
			/* initialize color key */
			stuff_set_color_transparence(t, colors[COLOR_BLACK]);
			memcpy(&s->frames[i], t, sizeof(*t));
			i++;
		}
	}
	if (i != s->frame_count) {
		stuff_quit_msg("Unexpected value of i, it's not equal to s->frame_count : %d != %d\n",
			       i, s->frame_count);
	}	
}

static void g_read_sprites(xmlNode *node)
{
	for(; node; node=node->next) {
		if (COMPARE((char *)node->name, "sprite")) {
			sprite_desc_t *s;

			s = (sprite_desc_t *)mem_alloc(sizeof *s);
			g_read_sprite(node->children, s);
			list_add(g_sprites, &s->name, s);
		}
	}
	
}

static void g_read_root_child_node(xmlNode *node)
{
	for(; node; node=node->next) {
		char *name;
		char *content;
		xmlNode *child;
		
		name = (char *)node->name;
		child = node->children;
		if (!child) 
			continue;
		content = (char *)child->content;
		if (COMPARE(name, "name"))
			/* name of level */
			strncpy(g_level->name, content, LEVEL_NAME_MAX);
		else if (COMPARE(name, "traces"))
			/* definition of traces */
			g_read_traces(child);	
		else if (COMPARE(name, "bullets_type"))
			/* definition of bullets type */
			g_read_bullets_type(child);
		else if (COMPARE(name, "bullets"))
			/* definition of bullets */
			g_read_bullets(child);
		else if (COMPARE(name, "waves"))
			/* definition of waves */
			g_read_waves(child);
		else if (COMPARE(name, "moves"))
			/* definition of moves */
			g_read_moves(child);
		else if (COMPARE(name, "moves_desc"))
			/* definition of move descriptor */
			g_read_moves_desc(child);
		else if (COMPARE(name, "masks"))
			/* definition of masks */
			g_read_masks(child);
		else if (COMPARE(name, "enemies"))
			/* definition of enemies */
			g_read_enemies(child);
		else if (COMPARE(name, "entities"))
			/* definition of entities */			
			g_read_entities(child);				
		else if (COMPARE(name, "musics"))
			/* definition of musics */
			g_read_musics(child);
		else if (COMPARE(name, "bosses"))
			/* definition of boss */
			g_read_bosses(child);
		else if (COMPARE(name, "sprites"))
			/* definition of sprites */
			g_read_sprites(child);
		
	}
}

/* initialize the level */
void g_level_xml(char *xml_file)
{
	xmlDoc *doc;
	xmlNode *root;
	
	LIBXML_TEST_VERSION;
	doc = xmlReadFile(xml_file, NULL, 0);
	if (!doc) 
		stuff_quit_msg("Unable to open XML : %s\n", xml_file);
	root = xmlDocGetRootElement(doc);
	g_read_root_child_node(root->children);
	xmlFreeDoc(doc);
	xmlCleanupParser();	
	
	/* search bullet_desc_snowball */
	bullet_desc_snowball = g_search_bullet(PLAYER_BULLET_DESC_SNOWBALL);
	if (!bullet_desc_snowball)
		stuff_quit_msg("Unable to find bullet description : %s\n", PLAYER_BULLET_DESC_SNOWBALL);			
}
