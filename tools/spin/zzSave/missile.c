/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * im.c -- contains the image vectors for the guided missile
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
			red1[] = {	/* porthole */
		{ -1.6, -3.4 }, 	/* move */
		{ -1.6, -2.4 }, 	/* draw... */
		{  1.6, -2.4 },
		{  1.6, -3.4 },
	},
			blue1[] = {	/* ship */
		{  0.6,  3.4 }, 	/* move */
		{  0.6, -3.4 }, 	/* draw... */
		{ -0.6, -3.4 },
		{ -0.6,  3.4 },
	};

	cos_a = cos(angle);
	sin_a = sin(angle);

	draw_area(3, sin_a, cos_a, blue1, NEL(blue1));
	draw_area(1, sin_a, cos_a, red1, NEL(red1));
}

