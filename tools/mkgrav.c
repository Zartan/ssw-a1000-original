/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * mkgrav.c -- build a gravity table for SSW table lookup stuff
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
 *
 */

#ident "$Id: mkgrav.c 1.2 1997/01/19 07:04:58 jamesc Exp jamesc $"

#include <sys/types.h>
#include <stdio.h>
#include "ssw.h"

#define BUFFER_SIZE	65536		/* output buffer size		*/

typedef union my_buf_u {
	char	c[BUFFER_SIZE];
	long	l[1];
} my_buf_t;

#define COL8(val)	(((val) & 7) == 7)

#define G_M	(64.0)			/* G * sun_mass			*/
#define FACTOR	(1L << CD_FRAC)		/* output scale factor		*/
#define MAX	(0x7fffffffL)		/* largest value that will fit	*/
#define INCR	(1. / (1L << GRAV_RES)) /* table resolution		*/

my_buf_t __far	bbuf;

extern int	errno;

extern double	sqrt(double);

void
main()
{
	register ulong	l;
	register long	iy, ix;
	register double v, rx, rx_sq, ry;
	register FILE	*bfp;
	ulong		lng;

	bfp = fopen("SSW.grv", "w");
	if (bfp == NULL) {
		printf("unable to open SSW.grv (err # %d)\n", errno);
		exit(2);
	}
	setvbuf(bfp, bbuf.c, sizeof(bbuf), _IOFBF);

	printf("Building SSW.grv...\n");

	for (rx = 0.0, ix = 0; ix < GRAV_SIZE; ix++, rx += INCR) {
		printf("%d,", ix);
		rx_sq = rx * rx;
		for (ry = 0.0, iy = 0; iy < GRAV_SIZE; iy++, ry += INCR) {
			if (ix) {
				v = rx_sq + ry * ry;
				/* use similar triangles to avoid an atan */
				v = (FACTOR * G_M * rx) / (v * sqrt(v));
				if (v >= (double)MAX)
					l = MAX;
				else
					l = (ulong)v;
			}
			else
				l = 0;
			lng = l;
			if (fwrite((char *)&lng, sizeof(lng), 1, bfp) <= 0) {
				printf("fwrite failed! (err # %d)\n", errno);
				fclose(bfp);
				exit(2);
			}
		}
	}
	fclose(bfp);
	printf("\nSSW.grv done!\n");

	exit(0);
}

