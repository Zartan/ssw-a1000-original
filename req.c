/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * req.c -- solar system wars program requester code
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

#ident "$Id: req.c 1.2 1997/01/19 07:04:58 jamesc Exp jamesc $"

#include "ssw.h"
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#ifndef NULL
#define NULL		(0)
#endif

#define YES_ID		1		/* gadget IDs			*/
#define NO_ID		0

#define GAD_HEIGHT	(MENU_HEIGHT+4) /* gadget and border height	*/
#define GAD_UNDER_Y	(GAD_HEIGHT-3)  /* equiv key underline Y offset */
#define SIDE_OFF	2		/* side border offset		*/
#define TOP_OFF 	3		/* text offset from req top	*/
#define MID_OFF 	4		/* middle spacer offset 	*/
#define BOT_OFF 	2		/* bottom border offset 	*/
#define R_BORDER_OFF	(TOP_OFF + MID_OFF + BOT_OFF)
#define NUM_XY		10		/* number of SHORTs to do a box */

extern struct Screen	*screen;
extern struct Window	*win;
extern struct ViewPort	*vp;
extern struct RastPort	*rp;

extern void		text(char *);

extern int		strlen(char *);

extern UBYTE		ssw_str[];

UBYTE			copyw_str[] = "Copyright \xA9 1994-6 James Cleverdon.";
UBYTE			allrr_str[] = "All rights reserved.";
UBYTE			free_str[]  = "THIS version may be freely copied!";

Static SHORT		g_xy_0[NUM_XY], g_xy_1[NUM_XY];
Static SHORT		g_xy_under[] = { 2, GAD_UNDER_Y, 8, GAD_UNDER_Y };

#define B_INIT(xy, next) { 0, 0, 0, 1, JAM1, NEL(xy) / 2, xy, next }

Static struct Border	g_under = B_INIT(g_xy_under, NULL);

Static struct Border	g_border[] = {
 B_INIT(g_xy_0, &g_under),
 B_INIT(g_xy_1, &g_under),
};

#define T_INIT()	{ 0, 1, JAM1, 2, 2, NULL, NULL }

Static struct IntuiText g_text[] = {
 T_INIT(),
 T_INIT()
};

#define G_INIT(next, num, id) { next, 0, 0, 0, GAD_HEIGHT, GADGHCOMP, RELVERIFY | ENDGADGET, BOOLGADGET | REQGADGET, &g_border[num], NULL, &g_text[num], 0L, NULL, id }

Static struct Gadget	req_gad[] = {
 G_INIT(&req_gad[1], 0, YES_ID),
 G_INIT(NULL, 1, NO_ID)
};

Static SHORT		r_xy[NUM_XY];

Static struct Border	req_border[] = {
 B_INIT(r_xy, NULL)
};

Static struct Requester req = {
 NULL, 0, 0, 0, 0,			/* old_r, left, top, width, height */
 0, 0, req_gad, req_border,		/* rleft, rtop, gadgets, border */
 NULL, NOISYREQ, 1			/* text, flags, backfill	*/
};

Static uint		req_up = 0;	/* true if requester on screen	*/


/*
 * set_xy -- set up the XY coords for a closed box in a Border structure
 */

void
set_xy(register SHORT *xy, register int left, register int top,
	int width, int height)
{
	register int	x, y;

	*xy++ = left;			/* x0 */
	*xy++ = top;			/* y0 */
	*xy++ = (x = left + width - 1); /* x1 */
	*xy++ = top;			/* y1 */
	*xy++ = x;			/* x2 */
	*xy++ = (y = top + height - 1); /* y2 */
	*xy++ = left;			/* x3 */
	*xy++ = y;			/* y3 */
	*xy++ = left;			/* x4 */
	*xy   = top;			/* y4 */
}


/*
 * gad_init
 */

Static void
gad_init(int num, char *text, int left, int txt_height, SHORT *xy)
{
	register struct Gadget	*gp;
	register int		n;

	gp = &req_gad[num];
	gp->GadgetText->IText = (UBYTE *)text;
	n = strlen(text) * (MENU_HEIGHT / 2) + SIDE_OFF;
	gp->LeftEdge = left - n;
	gp->TopEdge = txt_height + TOP_OFF + MID_OFF - 1;
	gp->Width = (n <<= 1);
	set_xy(xy, 0, 0, n, GAD_HEIGHT);
}


/*
 * my_req -- interface to Request
 */

Static void
my_req(struct IntuiText *btxt, char *ptxt, char *ntxt, int width, int height)
{
	register int	n;

	SetWrMsk(rp, 0x1);		/* PF1 only */

	if (req_up)
		EndRequest(&req, win);
	else
		do_pointer(1, STOP_WAIT);

	req_up = 1;
	req.ReqText = btxt;
	req.Height = (n = height + GAD_HEIGHT + R_BORDER_OFF + SIDE_OFF);
	req.Width = width;
	req.LeftEdge = (WIDTH - width) >> 1;
	req.TopEdge = (y_height - n) >> 1;

	set_xy(r_xy, 1, 1, width - SIDE_OFF, n - BOT_OFF);

	if (ptxt) {
		gad_init(0, ptxt, width / 3, height, g_xy_0);
		req.ReqGadget = &req_gad[0];
		n = 2 * width / 3;
	}
	else {
		req.ReqGadget = &req_gad[1];
		n = width / 2;
	}
	gad_init(1, ntxt, n, height, g_xy_1);

	Request(&req, win);
	WaitBlit();
}

#define Y_OFFSET			0
#define ITEXTI(left, top, text, next)	{ 0, 1, JAM2, left, Y_OFFSET + top * MENU_HEIGHT + 3, NULL, text, next }

Static char ok_str[] = "OK";

/*
 * about_req -- put up an "About" requester
 */

void
about_req(void)
{
	static struct IntuiText rtext[] = {
	 ITEXTI(19, 1, ssw_str,					&rtext[ 1]),
	 ITEXTI(15, 2, copyw_str,				&rtext[ 2]),
	 ITEXTI(19, 3, free_str,				&rtext[ 3]),
#undef Y_OFFSET
#define Y_OFFSET	5	/* skip a half line */
	 ITEXTI(5,  4, "Joystick control:",			&rtext[ 4]),
	 ITEXTI(5,  5, " Button: Fire     L/R   : Left/Right",	&rtext[ 5]),
	 ITEXTI(5,  6, " Forwrd: Thrust   Back  : Hyperspace",	&rtext[ 6]),
	 ITEXTI(5,  7, " Buttn2: ExtraWeapon (or LShift)",	&rtext[ 7]),
	 ITEXTI(5,  8, " Buttn3: FullShields (or LAlt)",	&rtext[ 8]),
	 ITEXTI(5,  9, "Keyboard control (numbers on keypad):",	&rtext[ 9]),
	 ITEXTI(5, 10, " ESC, Q: Quit",				&rtext[10]),
	 ITEXTI(5, 11, " P     : Pause    HELP  : This help",	&rtext[11]),
	 ITEXTI(5, 12, " DOWN,5: Fire     RET,0 : Hyperspace",	&rtext[12]),
	 ITEXTI(5, 13, " LEFT,4: Left     RIGHT,6: Right",	&rtext[13]),
	 ITEXTI(5, 14, " UP,  8: Thrust   RShift,2,ENTER:Extra",&rtext[14]),
	 ITEXTI(5, 15, " 7     : Left&Thr 9     : Right&Thrust",&rtext[15]),
	 ITEXTI(5, 16, " RAlt, Pad .: FullShields",		NULL),
	};
#undef Y_OFFSET
#define Y_OFFSET			0

	my_req(rtext, NULL, ok_str, WIDTH, (NEL(rtext) + 2) * MENU_HEIGHT);
}

/*
 * quit_req -- "Really Quit?" requester
 */

void
quit_req(void)
{
	static struct IntuiText rtext[] = {
	  ITEXTI(30,  1, "Really Quit?",	NULL)
	};

	my_req(rtext, "Yes", "No", WIDTH / 2, (NEL(rtext)+1) * MENU_HEIGHT);
}


/*
 * err -- printf-like error message requestor
 *
 *	Uses RawDoFmt, so remember to use %ld, %lx, and %lc.  (%-08.lx)
 */

void __stdargs
err(char *fmt, ...)
{
	register int		n, m;
	register char		c;
	register char		*p, *last;
	char			buf[256];	/* better not be bigger */
	struct IntuiText	*itp;
#undef Y_OFFSET
#define Y_OFFSET		4
	static struct IntuiText rtext[] = {
	  ITEXTI(7,  0, NULL, &rtext[1]),
	  ITEXTI(7,  1, NULL, &rtext[2]),
	  ITEXTI(7,  2, NULL, &rtext[3]),
	  ITEXTI(7,  3, NULL, &rtext[4]),
	  ITEXTI(7,  4, NULL, &rtext[5]),
	  ITEXTI(7,  5, NULL, &rtext[6]),
	  ITEXTI(7,  6, NULL, &rtext[7]),
	  ITEXTI(7,  7, NULL, NULL),
	};
	extern void		stuff_char();

	/*
	 * RawDoFmt does the printf thing.
	 */
	buf[0] = '\0';
	(void) RawDoFmt(fmt, (APTR)(&fmt + 1), stuff_char, buf);

	/*
	 * scan buf and build the rtext array
	 */
	m = 8;				/* minimum width */
	itp = rtext;
	p = last = buf;
	for (;; ++p) {
		while ((c = *p) != '\n') {
			if (c == '\0')
				goto out;
			p++;
		}
		*p = '\0';		/* overwrite \n with null char */
		itp->IText = last;
		if ((n = p - last) > m) {
			m = n;
		}
		last = p + 1;
		itp = (itp->NextText = &itp[1]);
		if (itp >= &rtext[NEL(rtext) - 1]) {
out:
			itp->IText = last;
			n = strlen((char *)last);
			if (n > m)
				m = n;
			break;		/* jam rest onto one line */
		}
	}
	itp->NextText = NULL;

	n = m * MENU_HEIGHT + 2 * SIDE_OFF;
	n = min(n, WIDTH);
	my_req(rtext, NULL, ok_str, n,
		((itp - rtext) + 1) * MENU_HEIGHT + MENU_HEIGHT / 2);
}


/*
 * do_gadget -- process a GADGETUP event, returns non-zero for exit
 */

int
do_gadget(struct Gadget *gp)
{
	req_up = 0;
	do_pointer(0, STOP_WAIT);
	WaitBlit();
	set_custom();
	return ((int)gp->GadgetID);
}


/*
 * do_req -- process a gadget equivalent key, returns non-zero for exit
 */

do_req(int drop_dead)
{
	if (!req_up || (drop_dead && req.ReqGadget != req_gad))
		return (0);

	EndRequest(&req, win);
	req_up = 0;
	do_pointer(0, STOP_WAIT);
	WaitBlit();
	set_custom();

	return (drop_dead);
}

