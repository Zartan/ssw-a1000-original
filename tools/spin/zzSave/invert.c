/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * invert.c -- swap left for right in sprite images
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

#define ulong	unsigned long

char	line[1024], comm[64], buf[4096];
ulong	low, high;

extern void	exit();

static ulong
reverse_word(register ulong src)
{
	register ulong	dest, src_mask, dest_mask;
	register int	n;

	src_mask = 0x8000;
	dest_mask = 0x0008;
	dest = 0;
	for (n = 15; --n >= 0; src_mask >>= 1, dest_mask <<= 1) {
		if (src & src_mask)
			dest |= dest_mask;
	}
	return (dest);
}

static char *
reverse_comm(register char *src)
{
	register char	*dest;
	register int	n;
	static char	tmp[16];

	dest = &tmp[sizeof(tmp) - 1];
	for (n = sizeof(tmp) - 1; --n >= 0; )
		*--dest = *src++;

	return (tmp);
}

void
main()
{
	register int	r;

	setvbuf(stdout, buf, sizeof(buf), _IOFBF);

	while (fgets(line, sizeof(line), stdin)) {
		r = sscanf(line, " 0x%lx, 0x%lx, /*%s*/", &low, &high, comm);
		if (r < 3) {
			fputs(line, stdout);
			continue;		/* skip other lines */
		}

		printf(" 0x%04lx, 0x%04lx, /*%s*/\n",
		  reverse_word(low), reverse_word(high), reverse_comm(comm));
	}
	fflush(stdout);
	exit(0);
}

