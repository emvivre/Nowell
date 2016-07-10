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

void r_draw_background(surface_t *s, rect_t *r)
{
	rect_t dst;
	if (!r)
		stuff_blit_surface(s, NULL, &screen_surface, NULL);	
	dst.pos.x = 0;
	dst.pos.y = 0;
	stuff_blit_surface(s, r, &screen_surface, &dst);
}

void r_draw_surface(surface_t *s, int x, int y)
{
	r_blit_surface(s, &screen_surface, x, y);
}

void r_blit_surface(surface_t *s, surface_t *d, int x, int y)
{
	rect_t src, dst;
	
	src.pos.x = 0;
	src.pos.y = 0;
	src.w = s->w;
	src.h = s->h;
	dst.pos.x = x;
	dst.pos.y = y;
	stuff_blit_surface(s, &src, d, &dst);
}
