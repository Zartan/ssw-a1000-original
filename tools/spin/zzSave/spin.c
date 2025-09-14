/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * spin.c -- screen handling for sprite image rotator
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

#include <exec/types.h>
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <libraries/dos.h>
#include <stdio.h>
#include "spin.h"

#define REV		33
#define WIDTH		320
#define HEIGHT		200

#define NUM_IMAGES	32		/* number of images in 360 degrees */

#define MULT		7
#define S_MIN		(2 * MULT)
#define S_MAX		(S_MIN + S_SIZE * MULT - 1)
#define S_OFFSET	(S_MIN + S_SIZE * MULT / 2)

char			image[S_SIZE + 4];

struct IntuitionBase	*IntuitionBase, *OpenLibrary();
struct GfxBase		*GfxBase;
struct Screen		*Screen, *OpenScreen();
struct Window		*win, *OpenWindow();
struct ViewPort 	*vp;
struct RastPort 	*rp;

struct TextAttr topaz_60 = {
	"topaz.font", TOPAZ_SIXTY, FS_NORMAL, FPF_ROMFONT
};

struct NewScreen	NewScreen = {
	0, 0, WIDTH, HEIGHT,		/* left/top edge, width, height */
	2, 0, 1,			/* depth, DetailPen, BlockPen	*/
	0,				/* display modes		*/
	CUSTOMSCREEN | SCREENBEHIND,	/* screen type			*/
	&topaz_60, NULL,		/* default font, screen title	*/
	NULL, NULL			/* gadgets, custom bitmap	*/
};

struct NewWindow	NewWindow = {
	0, 0, WIDTH, HEIGHT,		/* left/top edge, width, height */
	0, 1,				/* DetailPen, BlockPen		*/
	INTUITICKS,			/* IDCMPFlags			*/
					/* Window flags 		*/
	SMART_REFRESH | NOCAREREFRESH | BACKDROP | BORDERLESS | ACTIVATE,
	NULL, NULL,			/* gadgets, checkmark image	*/
	NULL,				/* window title 		*/
	NULL,				/* put screen pointer here	*/
	NULL,				/* super bitmap 		*/
	0, 0, 0, 0,			/* min/max width/heigth 	*/
	CUSTOMSCREEN			/* screen type			*/
};

static FILE			*outf;
PLANEPTR			planeptr, AllocRaster();

#define AREA_SIZE		200

static WORD			areabuffer[AREA_SIZE];
static struct TmpRas		tmpras;
static struct AreaInfo		areainfo;

extern struct IntuiMessage	*GetMsg();


/*
 * die -- shutdown and exit
 */

static void
die(int exval)
{
	if (outf)
		fclose(outf);
	if (planeptr)
		FreeRaster(planeptr, WIDTH, HEIGHT);
	if (win)
		CloseWindow(win);
	if (Screen)
		CloseScreen(Screen);
	if (GfxBase)
		CloseLibrary(GfxBase);
	if (IntuitionBase)
		CloseLibrary(IntuitionBase);

	exit(exval);
}

/*
 * initialize -- set things up
 */

static void
initialize()
{
	static USHORT	colors[] = {
		0x000, 0xf00, 0x0f0, 0x00f
	};

	/* open/init screen, and window */

	if (!(IntuitionBase = OpenLibrary("intuition.library", REV)))
		exit(1);
	if (!(GfxBase = (struct GfxBase*)OpenLibrary("graphics.library", REV)))
		die(2);
	if (!(Screen = OpenScreen(&NewScreen)))
		die(3);
	vp = &Screen->ViewPort;
	LoadRGB4(vp, colors, NEL(colors));
	ShowTitle(Screen, FALSE);
	ScreenToFront(Screen);
	NewWindow.Screen = Screen;
	if (!(win = OpenWindow(&NewWindow)))
		die(4);
	rp = win->RPort;

	/* allocate & init stuff for area fill functions */

	if (!(planeptr = AllocRaster(WIDTH, HEIGHT)))
		die(5);
	InitArea(&areainfo, areabuffer, (AREA_SIZE * 2) / 5);
	rp->AreaInfo = &areainfo;
	InitTmpRas(&tmpras, planeptr, RASSIZE(WIDTH, HEIGHT));
	rp->TmpRas = &tmpras;

	/* open output file */

	if ((outf = fopen("RAM:image.c", "w")) == NULL)
		die(6);
	fprintf(outf, "\n");
}

static void
scan_image()
{
	register long	x, y;
	register long	row, col, max;
	register char	*commp;
	static union cnt {
		ulong	l;
		uchar	c[sizeof(ulong)];
	} cnt;

	for (row = S_MIN; row <= S_MAX; row += MULT) {
		commp = image;
		for (col = S_MIN; col <= S_MAX; col += MULT) {
			/* count the instances of each color in the square */
			cnt.l = 0L;
			for (x = col + MULT; --x >= col; )
				for (y = row + MULT; --y >= row; )
					cnt.c[ReadPixel(rp, x, y)] += 1;

			/* find the color with the max pixles */
			for (max = y = 0, x = sizeof(ulong); --x >= 0; )
				if (cnt.c[x] > y) {
					max = x;
					y = cnt.c[x];
				}

			*commp++ = max + '0';
		}
		fprintf(outf, "%s\n", image);
	}
	fprintf(outf, "\n");
}

void
draw_ring(int color, real_t radius, real_t width)
{
	register real_t r;

	SetAPen(rp, color);
	r = MULT * radius;
	DrawCircle(rp, S_OFFSET, S_OFFSET, (long)r);
	r += MULT * width;
	DrawCircle(rp, S_OFFSET, S_OFFSET, (long)r);
	SetOPen(rp, color);
	r -= MULT/2 * width;
	Flood(rp, 0, S_OFFSET + (long)r, S_OFFSET);
}

void
draw_area(int color, register real_t sin_a, register real_t cos_a,
	  register coord_t *cp, register int n)
{
	register long	x, y;
	register int	flag;

	SetAPen(rp, color);
	SetOPen(rp, color);
	sin_a *= MULT;
	cos_a *= MULT;
	for (flag = -1; --n >= 0; cp++) {
		x = S_OFFSET + (long)(cp->x * cos_a - cp->y * sin_a);
		if (x < 0L)
			x = 0L;
		else if (x >= WIDTH)
			x = WIDTH - 1L;
		y = S_OFFSET + (long)(cp->x * sin_a + cp->y * cos_a);
		if (y < 0L)
			y = 0L;
		else if (y >= HEIGHT)
			y = HEIGHT - 1L;

		if (++flag == 0)
			AreaMove(rp, x, y);
		else
			AreaDraw(rp, x, y);
	}

	AreaEnd(rp);
}

void
main()
{
	register real_t 		angle, stop;
	register ULONG			waitbits, sigbits, win_bit;
	register struct MsgPort 	*userport;
	register struct IntuiMessage	*mp;

	initialize();

	userport = win->UserPort;
	win_bit = 1 << userport->mp_SigBit;
	sigbits = win_bit | SIGBREAKF_CTRL_C;

	angle = 0.0;
	stop = 2 * PI;
	for (; angle < stop; angle += (2 * PI) / NUM_IMAGES) {
		SetRast(rp, 0);
		draw_image(angle - PID2);
		scan_image();
		waitbits = Wait(sigbits);
		if ((waitbits & win_bit) && (mp = GetMsg(userport)))
			ReplyMsg(mp);
		if (waitbits & SIGBREAKF_CTRL_C)
			break;
	}

	die(0);
}

