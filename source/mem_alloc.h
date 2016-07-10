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

#include <unistd.h>

/* memory management */

/* une entry designe un buffer, allouer ou non
   une entry non allouer peut etre subdiviser en plusieurs entries non allouer
*/
typedef struct mem_entry_s{
	char allocated; // flag si allocated
	size_t size;
	void *start;	
	struct mem_entry_s *previous;
	struct mem_entry_s *next;
} mem_entry_t;


typedef struct mem_alloc_s {
	size_t size;
	mem_entry_t *head;
} mem_alloc_t;
	

mem_alloc_t *mem_alloc_create(size_t size);
void *mem_alloc_malloc(mem_alloc_t *mem, size_t size);
void mem_alloc_free(mem_alloc_t *mem, void *ptr);

