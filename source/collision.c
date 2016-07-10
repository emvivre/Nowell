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

// calcule la pente entre 2 points
double c_stribe(int x1, int y1, int x2, int y2)
{
	if (!(x2 - x1)) {
		return 0;
	}
	return (double) (y2 - y1) / (x2 - x1);
}

// check si les 3 points sont defini dans le sens des aiguilles d'une montre
// determine le cote du troisieme point par rapport au deux premier
int c_clock(int x1, int y1, int x2, int y2, int x3, int y3)
{
	double coef1, coef2;
	printf("(%d %d) (%d %d) (%d %d)\n", x1, y1, x2, y2, x3, y3);
	coef1 = c_stribe(x1, y1, x2, y2);
	coef2 = c_stribe(x1, y1, x3, y3);
	printf("coef : %lf %lf\n", coef1, coef2);
	if ( coef1 < coef2)
		return FALSE;
	return TRUE;
}

// check la collision de 2 lignes
int c_line(int x1, int y1, int x2, int y2, int x3, int y3, int x4, int y4)
{
	if (c_clock(x1, y1, x2, y2, x3, y3) != c_clock(x1, y1, x2, y2, x4, y4))
		if (c_clock(x3, y3, x4, y4, x1, y1) != c_clock(x3, y3, x4, y4, x2, y2))
			return TRUE;
	return FALSE;
}

// check si le point est dans le carrer
// x1,y1 et x2, y2 sont les 2 extremiter du carre
int c_point_in_square(int x1, int y1, int x2, int y2, int p_x, int p_y)
{
	if (p_x > x1 && p_x < x2)
		if (p_y > y1 && p_y < y2)
			return TRUE;
	return FALSE;
}

// check la collision de 2 surfaces
int c_surface(surface_t *s, int s_x, int s_y, surface_t *d, int d_x, int d_y)
{
	if ( s_x < d_x + d->w
	     && s_x + s->w > d_x
	     && s_y < d_y + d->h
	     && s_y + s->h > d_y)
		return TRUE;
	
	return FALSE;
}

/* return TRUE si l'enemie est en dehors de l'ecran */
int c_out_screen(pos_t *p, surface_t *s)
{
	int x, y;
	int w_half, h_half;
	x = p->x;
	y = p->y;	
	w_half = s->w/2;
	h_half = s->h/2;
	if (x + w_half < 0
	    || x - w_half > SCREEN_WIDTH
	    || y + h_half < 0
	    || y - h_half > SCREEN_HEIGHT)
		return TRUE;
	return FALSE;
}
