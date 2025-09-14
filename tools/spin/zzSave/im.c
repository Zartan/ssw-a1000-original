/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * im.c -- contains the image vectors
 *
 * This file is part of Solar System Wars (SSW).
 *
 * This version of SSW is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or (at your
 * option) any later version.
 *
 * SSW is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with SSW.  If not, see <https://www.gnu.org/licenses/>. 
 */

#include "spin.h"

extern real_t			sin(), cos();

void
draw_image(register real_t angle)
{
	register real_t sin_a, cos_a;

	static coord_t
			red1[] = {
		{ -0.6, -0.6 }, 	/* move */
		{  0.6, -0.6 }, 	/* draw... */
		{  0.6,  0.6 },
		{ -0.6,  0.6 },
	},
			red2[] = {
		{ -2.6, -3.6 }, 	/* move */
		{  2.6, -3.6 }, 	/* draw... */
		{  2.6, -2.5 },
		{ -2.6, -2.5 },
	},
			red3[] = {
		{ -0.6,  5.5 }, 	/* move */
		{  0.6,  5.5 }, 	/* draw... */
		{  0.6,  7.0 },
		{ -0.6,  7.0 },
	},
			blue1[] = {
		{  0.0,  4.9 }, 	/* move */
		{ -3.6, -2.6 }, 	/* draw... */
		{  3.6, -2.6 },
	};

	cos_a = cos(angle);
	sin_a = sin(angle);

	draw_area(3, sin_a, cos_a, blue1, NEL(blue1));
	draw_ring(2, 5.6, 1.2);
	draw_area(1, sin_a, cos_a, red1, NEL(red1));
	draw_area(1, sin_a, cos_a, red2, NEL(red2));
	draw_area(1, sin_a, cos_a, red3, NEL(red3));
}

