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


list_t *list_create(int (*cmp)(void *e1, void *e2))
{
	list_t *l;
	
	l = (list_t *)mem_alloc(sizeof *l);
	l->cmp = cmp;
	l->head = NULL;
	return l;
}

void list_destroy(list_t *l)
{
	list_entry_t *e;
	
	e = l->head;
	while (e) {
		list_entry_t *t;
		
		t = e;
		e = e->next;
		mem_free(t);      
	}
	mem_free(l);
}

// add and sort
void list_add(list_t *l, void *key, void *e)
{
	list_entry_t *new_entry;
	list_entry_t *next; // entry suivant l'entry qui va etre insere 
	list_entry_t **p = &l->head; // pointe sur l'adresse devant etre changer

	next = l->head;
	while(1) {
		if (!next || l->cmp(next->key, key) >= 0)
			break;
		p = &next->next;
		next = next->next;
	}		
	new_entry = (list_entry_t *)mem_alloc(sizeof *new_entry);	
	new_entry->key = key;
	new_entry->e = e;
	new_entry->next = next;
	*p = new_entry;
}

void *list_rm(list_t *l, void *key)
{
	list_entry_t **p;
	list_entry_t *e;
	void *element;

	if (!l->head)
		return NULL;
	p = &l->head;
	e = l->head;
	while (e && l->cmp(key, e->key)) {
		p = &e->next;
		e = e->next;
	}
	if (!e)
		return NULL;
	element = e->e;
	*p = e->next;
	mem_free(e);
	return element;
}

void *list_search(list_t *l, void *key)
{
	list_entry_t *e;
	
	e = l->head;
	while (e && l->cmp(key, e->key)) 
		e = e->next;
	if (!e)
		return NULL;	
	return e->e;
}

void list_parcour_init(list_t *l)
{
	l->parcour = l->head;
	l->current = NULL;
	l->ptr_next = &l->head;
}

void *list_parcour_next(list_t *l)
{
	void *e;
	
	if (!l->parcour) {
		e = NULL;
		l->current = NULL;
	}
	else {		
		e = l->parcour->e;
		l->current = l->ptr_next;
		l->ptr_next = &l->parcour->next;
		l->parcour = l->parcour->next;
	}
	return e;
}

void list_parcour_rm_current(list_t *l)
{
	if (l->current && (*l->current)) {
		list_entry_t *c;
		
		c = *l->current;
		*l->current = c->next;
		l->ptr_next = l->current;
		mem_free(c);
	}
}

int list_count(list_t *l)
{
	int count;
	list_entry_t *e;

	e = l->head;
	for(count=0; e; count++)
		e = e->next;
	return count;
}
