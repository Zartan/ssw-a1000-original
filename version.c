/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * version.c -- Solar System Wars program version strings
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

#ident "$Id: version.c 1.2 1997/01/19 07:04:58 jamesc Exp jamesc $"

#include <exec/types.h>

#define VERSION 	"1.43"

static char const	version_str[] = "\0$VER: SSW " VERSION " (19.01.97)\0";

UBYTE const		ssw_str[] = "Solar Systems Wars -- Version " VERSION;
