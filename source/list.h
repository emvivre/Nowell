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

#ifndef __LIST_H__
#define __LIST_H__

typedef struct list_entry_s {
	void *key; // key
	void *e; // element
	struct list_entry_s *next;
} list_entry_t;

typedef struct {
	list_entry_t *head; // premier element 
	list_entry_t *parcour; // pour le parcour iteratif, l'entry courrante
	list_entry_t **current; // pointe vers l'objet current
	list_entry_t **ptr_next; // pointe vers l'objet suivant
	int (*cmp)(void *k1, void *k2);
} list_t;

enum {LIST_NO_ERROR = 0,
      LIST_NO_KEY_FOUND = -1
};

list_t *list_create(int (*cmp)(void *e1, void *e2));
void list_destroy(list_t *l);
void list_add(list_t *l, void *key, void *e);
void *list_rm(list_t *l, void *key);
void *list_search(list_t *l, void *key);
void list_parcour_init(list_t *l);
void *list_parcour_next(list_t *l);
void list_parcour_rm_current(list_t *l);
void list_sort(list_t *l);
int list_count(list_t *l);

#endif /* not __LIST_H__ */
