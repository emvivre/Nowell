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

#include <malloc.h>
#include "mem_alloc.h"


/* aligner sur un multiple de 32 superieur */
#define ALIGN_32(SIZE) ((SIZE+31)&(~31))

/* taille minimum d'une entry */
#define ENTRY_MIN_SIZE (sizeof (mem_entry_t) +1)

enum { ENTRY_NO_ALLOCATED, ENTRY_ALLOCATED };

mem_alloc_t *mem_alloc_create(size_t size)
{
	mem_alloc_t *m;
	mem_entry_t *e;

	m = (mem_alloc_t *)memalign(32, size);
	m->head = (mem_entry_t *) (m+1);	
	m->size = size;
	e = m->head;
	e->allocated = ENTRY_NO_ALLOCATED;
	e->size = size - (sizeof *m);
	e->start = (void *) (e+1);
	e->previous = NULL;
	e->next = NULL;
	
	return m;
}

void *mem_alloc_malloc(mem_alloc_t *mem, size_t size)
{
	mem_entry_t *e;

	/* align size */
	size = ALIGN_32(size);
	e = mem->head;
	while (e) { 
		mem_entry_t *next;

		if (e->allocated || e->size < size) {
			/* we can't allocated in this entry */
			e = e->next;
			continue;
		}
		
		if (e->size < size + ENTRY_MIN_SIZE) {
			/* use this entry and not allocated entry */
			e->allocated = ENTRY_ALLOCATED;
			return e->start;
		}

		/* we need to subdivised */
		/* initialize new entry */
		next = ((void *) e->start) + size;
		next->previous = e;
		next->next = e->next;
		next->allocated = ENTRY_NO_ALLOCATED;
		next->size = e->size - sizeof(*next);				
		next->start = (void *)(next+1);
		
		/* change previous of the next next entry */
		if (next->next) 
			next->next->previous = next;			
		e->allocated = ENTRY_ALLOCATED;
		e->size = size;
		e->next = next;
		return e->start;
	}
	return NULL;       
}

void mem_alloc_free(mem_alloc_t *mem, void *ptr)
{
	mem_entry_t *e;
	
	e = mem->head;
	while (e) {
		if (e->allocated && e->start == ptr) {
			mem_entry_t *prev;
			mem_entry_t *next;
			e->allocated = ENTRY_NO_ALLOCATED;			


			/* check neighboor flag allocated */
			prev = e->previous;
			next = e->next;
#if 0

			if (prev && !prev->allocated) { 
				/* merge previous with current */
				prev->size += e->size + sizeof(*e);
				prev->next = next;
				if (next) 
					next->previous = prev;
				e = prev;
			}
			if (next && !next->allocated) {
				/* merge next with current */
				e->size += next->size + sizeof(*next);
				e->next = next->next;
				if (next->next) 
					next->next->previous = e;			
			}
#endif
			break;
		}
		e = e->next;
	}
}
