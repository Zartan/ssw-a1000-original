/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * img.c -- read a file of images and produce a sprite_image array
 *	invoked as a filter:  stdin converted to stdout
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

#include <stdio.h>

char	line[1024], buf[4096];

extern void	exit();
extern char	*strchr(), *memcpy();

void
main()
{
	register char	*p;
	register long	c, n, mask, low, high;

	setvbuf(stdout, buf, sizeof(buf), _IOFBF);

	while (fgets(line, sizeof(line), stdin)) {
		if (p = strchr(line, '\n'))
			*p = '\0';                      /* zap newline */
		else {
			printf("\n");
			continue;
		}
		if ((c = line[0]) < '0' || c > '3') {
			printf("%s\n", line);
			continue;			/* pass through */
		}
		memset(p, 0, 16);                       /* zero 16 bytes */
		low = high = 0;
		p = line;
		for (n = 16, mask = 0x8000; --n >= 0; mask >>= 1) {
			if ((c = *p++ - '0') < 0 || c > 3)
				c = 0;

			if (c & 1)
				low |= mask;
			if (c & 2)
				high |= mask;
		}

		printf(" 0x%04x, 0x%04x, /*%s*/\n", low, high, line);
	}
	fflush(stdout);
	exit(0);
}

