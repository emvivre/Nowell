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

#include <stdlib.h>
#include "common.h"
#include "mem_alloc.h"

#define MEMORY_SIZE (10*1024*1024)
//static mem_alloc_t *memory;

void mem_init(void)
{
	//	memory = mem_alloc_create(MEMORY_SIZE);
	//if (!memory)
	//stuff_quit_msg("Unable to allocated %d\n", MEMORY_SIZE);
}

void *mem_alloc(size_t size)
{
	char *ptr;
	
	// ptr = (char *)mem_alloc_malloc(memory, size);
	ptr = (char *)malloc(size);
	if (!ptr)
		stuff_quit_msg("Unable to allocated %d from mem_alloc_malloc", size);
	return ptr;	       
}

void mem_free(void *ptr)
{
	// mem_alloc_free(memory, ptr);
	free(ptr);
}
