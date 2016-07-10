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
#include "common.h"

double m_vector_length(vector_t *v)
{
	return sqrt(v->x * v->x + v->y * v->y);
}

double m_length_btw_two_points(int x1, int y1, int x2, int y2)
{
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

void m_vector_normalize(vector_t *v, vector_t *result)
{
	double l;
	
	l = m_vector_length(v);
	if (!l)
		l = 1.0;
	result->x = v->x/l;
	result->y = v->y/l;
}

/* result = a - b */
void m_vector_substract(vector_t *a, vector_t *b, vector_t *result)
{
	result->x = a->x - b->x;
	result->y = a->y - b->y;
}

/* compute B(x2, y2) - A(x1, y1)
   source : B
   destination : A
   do normalize and put all in result */
void m_vector_normalize_btw_2_points(int x1, int y1, int x2, int y2, vector_t *result)
{
	vector_t v1, v2;
	
	v1.x = x1;
	v1.y = y1;
	v2.x = x2;
	v2.y = y2;
	m_vector_substract(&v2, &v1, result);
	m_vector_normalize(result, result);
}

/* compute angle of rotation between axis_Y(0; 1) and (x, y)
 * length between (0; 0) and (x, y) must be equal to 1
 */
double m_angle_rot_Y(double x, double y)
{
	double angle;
	
	angle = DOT_PRODUCT(x, y, axis_Y.x, axis_Y.y);
	angle = acos(angle);
	angle = RAD2DEG(angle);
	if (x > 0 )
		angle = 0 - angle;
	angle = 0 - angle;  
	return angle;
}
