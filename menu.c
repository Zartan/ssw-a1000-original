/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * menu.c -- solar system wars program menus
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

#ident "$Id: menu.c 1.3 1997/01/19 07:23:04 jamesc Exp jamesc $"

#include "ssw.h"
#include <graphics/gfxmacros.h>
#include <intuition/intuition.h>
#include <proto/graphics.h>
#include <proto/intuition.h>

#ifndef NULL
#define NULL		(0)
#endif

#define PTR_SPR0	0		/* pointer state codes	*/
#define PTR_INTUI	1
#define PTR_PAUSE	2
#define PTR_INVIS	3

#define SM_AST		6		/* asteroid index in system_menu */
#define OM_WRAP 	2		/* wrap index in options_menu	*/
#define PT_XTRA_OFF	3		/* xtra opts offset in ports_menu */

#define MASS_LIMIT	4		/* most mass in beginner mode */

#define ITEXTI(text)	{ 0, 1, JAM2, LOWCHECKWIDTH + 1, 1, NULL, text }

Static struct IntuiText itext[] = {
  ITEXTI("Pause?"),			/* 0: project menu */
  ITEXTI("New"),
  ITEXTI("About..."),
  ITEXTI("Screen To Front"),
  ITEXTI("Screen To Back"),
  ITEXTI("Quit..."),

  ITEXTI("No Primaries"),		/* 6: system menu */
  ITEXTI("One Primary    \xBB"),
  ITEXTI("Two Primaries  \xBB"),
  ITEXTI("Three Primaries\xBB"),
  ITEXTI("Misc. Systems  \xBB"),
  ITEXTI("Asteroid?"),

  ITEXTI("Ship Thrust      \xBB"),	/* 12: options_menu */
  ITEXTI("Torp Velocity    \xBB"),
  ITEXTI("Torp Lifetime    \xBB"),
  ITEXTI("Screen Wrap?"),
  ITEXTI("Green Ship Opts  \xBB"),
  ITEXTI("Purple Ship Opts \xBB"),

  ITEXTI("Low"),			/* 18: ship/torp thrust menus */
  ITEXTI("Medium"),
  ITEXTI("High"),

  ITEXTI("Short"),			/* 21: torp_menu */
  ITEXTI("Long"),			/* use medium from thrust menus */

  ITEXTI("Seeker"),			/* 23: former bound_menu */
  ITEXTI("Random Game?"),

  ITEXTI("Joyport 0"),			/* 25: port menus */
  ITEXTI("Joyport 1"),
  ITEXTI("Keyboard"),
  ITEXTI("Cloaking"),
  ITEXTI("Anomalizer"),
  ITEXTI("Plasma Bolt"),
  ITEXTI("Quad Thrust"),
  ITEXTI("Cluster Pod"),

  ITEXTI("Red Dwarf"),			/* 33: pri1 menu */
  ITEXTI("Yellow Star"),
  ITEXTI("Blue Giant"),
  ITEXTI("Small Black Hole"),
  ITEXTI("Medium Black Hole"),
  ITEXTI("Large Black Hole"),

  ITEXTI("Red Dwarves"),		/* 39: pri2 menu */
  ITEXTI("Red / Yellow"),
  ITEXTI("Red / Blue"),
  ITEXTI("Red / Sml BH"),
  ITEXTI("Red / Med BH"),
  ITEXTI("Red / Lrg BH"),
  ITEXTI("Yellow Stars"),
  ITEXTI("Yellow / Blue"),
  ITEXTI("Yellow / Sml BH"),
  ITEXTI("Yellow / Med BH"),
  ITEXTI("Yellow / Lrg BH"),
  ITEXTI("Blue Giants"),
  ITEXTI("Blue / Sml BH"),
  ITEXTI("Blue / Med BH"),
  ITEXTI("Blue / Lrg BH"),
  ITEXTI("Small Black Holes"),
  ITEXTI("Sml BH / Med BH"),
  ITEXTI("Sml BH / Lrg BH"),
  ITEXTI("Medium Black Holes"),
  ITEXTI("Med BH / Lrg BH"),
  ITEXTI("Large Black Holes"),

  ITEXTI("Red / Yel / Yel"),		/* 60: pri3 menu */
  ITEXTI("Yel / Blue / Blue"),
  ITEXTI("Yel / SBH / SBH"),
  ITEXTI("Blue / MBH / MBH"),
  ITEXTI("BH: S / M / M"),
  ITEXTI("BH: M / L / L"),

  ITEXTI("Black Ellipse"),		/* 66: misc systems */
  ITEXTI("Wide Red Binary"),
  ITEXTI("Wide Yellow Binary"),
  ITEXTI("Wide Blue Binary"),
  ITEXTI("Wide Sml BH Binary"),
  ITEXTI("Wide Med BH Binary"),
  ITEXTI("Wide Lrg BH Binary"),
  ITEXTI("Asteroid Swarm"),
  ITEXTI("Four Primaries \xBB"),	/* 74: pri4 menu item */

  ITEXTI(""),				/* 75: null IText */

  ITEXTI("Guided Torp"),		/* 76: new xtra weapons */
  ITEXTI("Disruptor"),

  ITEXTI("Beginner Mode?"),		/* 78: new options */
};

#define MLEFT	50
#define MOFF	4
#define MENUI(next, top, width, flags, mut, text, cmd, submenu) { next, MLEFT, top * MENU_HEIGHT - MOFF, width, MENU_HEIGHT, flags, mut, (APTR)&itext[text], NULL, cmd, submenu, MENUNULL }
#define MENUF	(ITEMTEXT | ITEMENABLED | HIGHCOMP)
#define MENUC	(MENUF | COMMSEQ)
#define MENUT	(MENUF | CHECKIT | MENUTOGGLE)
#define MENUCT	(MENUT | COMMSEQ)
#define MENUX	(MENUF | CHECKIT)
#define MENUCX	(MENUX | COMMSEQ)
#define MENUNIL (ITEMTEXT | HIGHNONE)
#define SYS_W	190			/* system submenus width */

Static struct MenuItem
			pri1_menu[] = {
  MENUI(&pri1_menu[1],-1, SYS_W, MENUCX, 0, 33, 'E', NULL),
  MENUI(&pri1_menu[2], 0, SYS_W, MENUCX, 0, 34, 'Y', NULL),
  MENUI(&pri1_menu[3], 1, SYS_W, MENUCX, 0, 35, 0x13, NULL),
  MENUI(&pri1_menu[4], 2, SYS_W, MENUCX, 0, 36, 'T', NULL),
  MENUI(&pri1_menu[5], 3, SYS_W, MENUCX, 0, 37, 'U', NULL),
  MENUI(NULL,	       4, SYS_W, MENUCX, 0, 38, 'I', NULL),
},
			pri2_menu[] = {
  MENUI(&pri2_menu[ 1],-2, SYS_W, MENUCX | CHECKED, 0, 39, 'O', NULL),
  MENUI(&pri2_menu[ 2],-1, SYS_W, MENUCX, 0, 40, '{', NULL),
  MENUI(&pri2_menu[ 3], 0, SYS_W, MENUCX, 0, 41, '}', NULL),
  MENUI(&pri2_menu[ 4], 1, SYS_W, MENUCX, 0, 42, 'D', NULL),
  MENUI(&pri2_menu[ 5], 2, SYS_W, MENUCX, 0, 43, 'F', NULL),
  MENUI(&pri2_menu[ 6], 3, SYS_W, MENUCX, 0, 44, 'G', NULL),
  MENUI(&pri2_menu[ 7], 4, SYS_W, MENUCX, 0, 45, 'H', NULL),
  MENUI(&pri2_menu[ 8], 5, SYS_W, MENUCX, 0, 46, 'J', NULL),
  MENUI(&pri2_menu[ 9], 6, SYS_W, MENUCX, 0, 47, 'K', NULL),
  MENUI(&pri2_menu[10], 7, SYS_W, MENUCX, 0, 48, ';', NULL),
  MENUI(&pri2_menu[11], 8, SYS_W, MENUCX, 0, 49, ':', NULL),
  MENUI(&pri2_menu[12], 9, SYS_W, MENUCX, 0, 50, '\'', NULL),
  MENUI(&pri2_menu[13],10, SYS_W, MENUCX, 0, 51, '"', NULL),
  MENUI(&pri2_menu[14],11, SYS_W, MENUCX, 0, 52, 0x0c, NULL),
  MENUI(&pri2_menu[15],12, SYS_W, MENUCX, 0, 53, 'X', NULL),
  MENUI(&pri2_menu[16],13, SYS_W, MENUCX, 0, 54, 'C', NULL),
  MENUI(&pri2_menu[17],14, SYS_W, MENUCX, 0, 55, 'V', NULL),
  MENUI(&pri2_menu[18],15, SYS_W, MENUCX, 0, 56, ',', NULL),
  MENUI(&pri2_menu[19],16, SYS_W, MENUCX, 0, 57, '<', NULL),
  MENUI(&pri2_menu[20],17, SYS_W, MENUCX, 0, 58, '.', NULL),
  MENUI(NULL,	       18, SYS_W, MENUCX, 0, 59, '>', NULL),
},
			pri3_menu[] = {
  MENUI(&pri3_menu[1],-2, SYS_W, MENUCX, 0, 60, '/', NULL),
  MENUI(&pri3_menu[2],-1, SYS_W, MENUCX, 0, 61, '?', NULL),
  MENUI(&pri3_menu[3], 0, SYS_W, MENUCX, 0, 62, 0x7f, NULL),
  MENUI(&pri3_menu[4], 1, SYS_W, MENUCX, 0, 63, 0x1b, NULL),
  MENUI(&pri3_menu[5], 2, SYS_W, MENUCX, 0, 64, 0x02, NULL),
  MENUI(NULL,	       3, SYS_W, MENUCX, 0, 65, 0x07, NULL),
},
			pri4_menu[] = {
  MENUI(&pri4_menu[1],-1, SYS_W, MENUCX, 0, 33, 0x0f, NULL),
  MENUI(&pri4_menu[2], 0, SYS_W, MENUCX, 0, 34, 0x10, NULL),
  MENUI(&pri4_menu[3], 1, SYS_W, MENUCX, 0, 35, 0x11, NULL),
  MENUI(NULL,	       2, SYS_W, MENUCX, 0, 36, 0x12, NULL),
},
			misc_menu[] = {
  MENUI(&misc_menu[1],-2, SYS_W, MENUCX, 0, 66, 0x08, NULL),
  MENUI(&misc_menu[2],-1, SYS_W, MENUCX, 0, 67, 0x0a, NULL),
  MENUI(&misc_menu[3], 0, SYS_W, MENUCX, 0, 68, 0x0e, NULL),
  MENUI(&misc_menu[4], 1, SYS_W, MENUCX, 0, 69, '`', NULL),
  MENUI(&misc_menu[5], 2, SYS_W, MENUCX, 0, 70, '~', NULL),
  MENUI(&misc_menu[6], 3, SYS_W, MENUCX, 0, 71, ' ', NULL),
  MENUI(&misc_menu[7], 4, SYS_W, MENUCX, 0, 72, '\t', NULL),
  MENUI(NULL,	       5, SYS_W, MENUCX, 0, 73, 0x0b, NULL),
},
#undef MLEFT
#undef MOFF
#define MLEFT	75
#define MOFF	1
			ship_thrust_menu[] = {
  MENUI(&ship_thrust_menu[1],0, 106, MENUCX, 0x06, 18, '_', NULL),
  MENUI(&ship_thrust_menu[2],1, 106, MENUCX | CHECKED, 0x05, 19, '+', NULL),
  MENUI(NULL,		     2, 106, MENUCX, 0x03, 20, '|', NULL),
},
			torp_thrust_menu[] = {
  MENUI(&torp_thrust_menu[1],0, 106, MENUCX, 0x06, 18, '-', NULL),
  MENUI(&torp_thrust_menu[2],1, 106, MENUCX | CHECKED, 0x05, 19, '=', NULL),
  MENUI(NULL,		     2, 106, MENUCX, 0x03, 20, '\\', NULL),
},
			torp_life_menu[] = {
  MENUI(&torp_life_menu[1], 0, 106, MENUCX | CHECKED, 0x06, 21, 'S', NULL),
  MENUI(&torp_life_menu[2], 1, 106, MENUCX, 0x05, 19, 'M', NULL),
  MENUI(NULL,		    2, 106, MENUCX, 0x03, 22, 'L', NULL),
},
#undef MLEFT
#define MLEFT	45
			port0_menu[] = {
  MENUI(&port0_menu[1], 0, 136, MENUCX, 0x02, 25, '1', NULL),
  MENUI(&port0_menu[2], 1, 136, MENUCX | CHECKED, 0x01, 27, '2', NULL),
  MENUI(&port0_menu[3], 2, 136, MENUNIL, 0, 75, 0, NULL),
  MENUI(&port0_menu[4], 3, 136, MENUCX | CHECKED, 0x7f0, 28, '3', NULL),
  MENUI(&port0_menu[5], 4, 136, MENUCX, 0x7e8, 77, '4', NULL),
  MENUI(&port0_menu[6], 5, 136, MENUCX, 0x7d8, 29, '5', NULL),
  MENUI(&port0_menu[7], 6, 136, MENUCX, 0x7b8, 30, '6', NULL),
  MENUI(&port0_menu[8], 7, 136, MENUCX, 0x778, 31, '7', NULL),
  MENUI(&port0_menu[9], 8, 136, MENUCX, 0x6f8, 23, '8', NULL),
  MENUI(&port0_menu[10],9, 136, MENUCX, 0x5f8, 76, '9', NULL),
  MENUI(NULL,	       10,136, MENUCX, 0x3f8, 32, '0', NULL),
},
			port1_menu[] = {
  MENUI(&port1_menu[1], 0, 136, MENUCX | CHECKED, 0x02, 26, '!', NULL),
  MENUI(&port1_menu[2], 1, 136, MENUCX, 0x01, 27, '@', NULL),
  MENUI(&port1_menu[3], 2, 136, MENUNIL, 0, 75, 0, NULL),
  MENUI(&port1_menu[4], 3, 136, MENUCX | CHECKED, 0x7f0, 28, '#', NULL),
  MENUI(&port1_menu[5], 4, 136, MENUCX, 0x7e8, 77, '$', NULL),
  MENUI(&port1_menu[6], 5, 136, MENUCX, 0x7d8, 29, '%', NULL),
  MENUI(&port1_menu[7], 6, 136, MENUCX, 0x7b8, 30, '^', NULL),
  MENUI(&port1_menu[8], 7, 136, MENUCX, 0x778, 31, '&', NULL),
  MENUI(&port1_menu[9], 8, 136, MENUCX, 0x6f8, 23, '*', NULL),
  MENUI(&port1_menu[10],9, 136, MENUCX, 0x5f8, 76, '(', NULL),
  MENUI(NULL,	       10,136, MENUCX, 0x3f8, 32, ')', NULL),
};
#undef	MLEFT
#undef	MOFF
#define MLEFT	0
#define MOFF	0
Static struct MenuItem	project_menu[] = {
  MENUI(&project_menu[1], 0, 185, MENUCT, 0, 0, 'P', NULL),
  MENUI(&project_menu[2], 1, 185, MENUC,  0, 1, 'N', NULL),
  MENUI(&project_menu[3], 2, 185, MENUC,  0, 2, '\01', NULL),
  MENUI(&project_menu[4], 3, 185, MENUC,  0, 3, '[', NULL),
  MENUI(&project_menu[5], 4, 185, MENUC,  0, 4, ']', NULL),
  MENUI(NULL,		  5, 185, MENUC,  0, 5, 'Q', NULL),
},
			system_menu[] = {
  MENUI(&system_menu[1], 0, 155, MENUCX, 0,  6, 'Z', NULL),
  MENUI(&system_menu[2], 1, 155, MENUF,  0,  7,   0, pri1_menu),
  MENUI(&system_menu[3], 2, 155, MENUF,  0,  8,   0, pri2_menu),
  MENUI(&system_menu[4], 3, 155, MENUF,  0,  9,   0, pri3_menu),
  MENUI(&system_menu[5], 4, 155, MENUF,  0, 74,   0, pri4_menu),
  MENUI(&system_menu[6], 5, 155, MENUF,  0, 10,   0, misc_menu),
  MENUI(NULL,		 6, 155, MENUCT, 0, 11, 'A', NULL),
},
			options_menu[] = {
  MENUI(&options_menu[1],0, 160, MENUCT | CHECKED, 0, 78, 'B', NULL),
  MENUI(&options_menu[2],1, 160, MENUCT, 0, 24, 'R', NULL),
  MENUI(&options_menu[3],2, 160, MENUCT | CHECKED, 0, 15, 'W', NULL),
  MENUI(&options_menu[4],3, 160, MENUF, 0, 12, 0, ship_thrust_menu),
  MENUI(&options_menu[5],4, 160, MENUF, 0, 13, 0, torp_thrust_menu),
  MENUI(&options_menu[6],5, 160, MENUF, 0, 14, 0, torp_life_menu),
  MENUI(&options_menu[7],6, 160, MENUF, 0, 16, 0, port0_menu),
  MENUI(NULL,		 7, 160, MENUF, 0, 17, 0, port1_menu),
};

#undef MENUI
#undef MENUF
#undef MENUC
#undef MENUT
#undef MLEFT

struct Menu		menu[] = {
{ &menu[1],				/* next menu			*/
  0, 0, 64, MENU_HEIGHT,		/* left/top edge, width, height */
  MENUENABLED,				/* menu flags			*/
  "Project",				/* menu name			*/
  project_menu				/* first menu item		*/
},
{ &menu[2], 125, 0, 66, MENU_HEIGHT, MENUENABLED, "Options", options_menu },
{ NULL,      67, 0, 58, MENU_HEIGHT, MENUENABLED, "System", system_menu },
};

BYTE			*xo_list[] = {
	"Cloaking",
	"Disruptor",
	"Anomalizer",
	"Plasma Bolt",
	"Quad Thrust",
	"Seeker",
	"Guided Torp",
	"Cluster Pod",
	"?",
};

extern S_SYSTEM 	pri0_sys[], pri0a_sys[], pri1_sys[], pri1a_sys[],
			pri2_sys[], pri3_sys[], pri4_sys[], misc_sys[];

	/* used to keep track of which system was selected from the menu */
S_SYSTEM		*s_sys = pri2_sys;
int			rand_system;
Static struct MenuItem	*sys_mp = pri2_menu;
Static uint		sys_item = 2,
			sys_sub = 0;

extern SS_LIST		ss_list[];
extern uint		num_ss_list;
extern uint		max_systems;
extern uchar		beginner_mode;

extern struct Screen	*screen;
extern struct Window	*win;
extern struct RastPort	*rp;

extern void		new_game(int);
extern void		about_req(void), quit_req(void);
extern void		set_custom(void);
extern void		rev_text(long, long, BYTE *);
extern void		bound_box(void);
extern void		mouse_ctl(int);


Static ushort	xtr_x[] = { WIDTH - 50, 50 };
#define XTR_Y	(y_mid + 40)

/*
 * rand_sys -- pick a random solar system, allowing for beginner_mode
 */

Static SS_LIST *
rand_sys(long *subp)
{
	register SS_LIST	*ssl;
	register IOBJ		*iop;
	register long		sub;
	register long		num;
	register int		mass;

	do {
		sub = RND(max_systems);
		for (ssl = ss_list; sub >= (num = ssl->num_ss); ssl++)
			sub -= num;

		if (!beginner_mode)
			break;

		mass = 0;
		iop = ssl->ssys[sub].iobj;
		num = ssl->ssys[sub].num_obj;
		for ( ; --num >= 0; iop++) {
			switch (iop->otype) {
			case OTYPE_RED_STAR:
				mass += 1;
			CASE OTYPE_YELLOW_STAR:
				mass += 2;
			CASE OTYPE_BLUE_STAR:
			case OTYPE_SM_HOLE:
				mass += 3;
			CASE OTYPE_MED_HOLE:
				mass += 4;
			CASE OTYPE_LRG_HOLE:
				mass += 5;
			DEFAULT:
				continue;
			}
			if (mass > MASS_LIMIT)
				break;			/* enough */
		}
	} while (mass > MASS_LIMIT);

	*subp = sub;
	return (ssl);
}

/*
 * rand_game -- pick a random solar system, random screen boundary, and
 *		random xtra_opts for each player
 */

void
rand_game(void)
{
	register struct MenuItem	*mi;
	register SS_LIST		*ssl;
	register int			idx;
	long				sub;
	uchar				old_xo[4];

	for (idx = 2; --idx >= 0; ) {
		old_xo[idx] = plr[idx].xtra_opt;
		do {
			plr[idx].xtra_opt = RND(NUM_XOPTS);
		} while (plr[idx].xtra_opt == old_xo[idx]);
		rev_text(xtr_x[idx], XTR_Y, xo_list[plr[idx].xtra_opt]);
	}
	bound_type = rand64() & 1;
	ssl = rand_sys(&sub);
	mi = ItemAddress(menu, FULLMENUNUM(2, ssl->item_num, sub));
	rev_text(WIDTH / 2, y_mid + 65,
	  ((struct IntuiText *)mi->ItemFill)->IText);
	Disable();
	sys_mp->Flags &= ~CHECKED;		/* Mutual Exclude */
	mi->Flags |= CHECKED;			/* pick new	*/
	if (ssl->asteroid)			/* do asteroid  */
		system_menu[SM_AST].Flags |= CHECKED;
	else
		system_menu[SM_AST].Flags &= ~CHECKED;
	if (bound_type)
		options_menu[OM_WRAP].Flags |= CHECKED;
	else
		options_menu[OM_WRAP].Flags &= ~CHECKED;
	port0_menu[old_xo[0] + PT_XTRA_OFF].Flags &= ~CHECKED;
	port0_menu[plr[0].xtra_opt + PT_XTRA_OFF].Flags |= CHECKED;
	port1_menu[old_xo[1] + PT_XTRA_OFF].Flags &= ~CHECKED;
	port1_menu[plr[1].xtra_opt + PT_XTRA_OFF].Flags |= CHECKED;
	Enable();
	s_sys = &ssl->ssys[sub];
	sys_mp = mi;
	sys_item = ssl->item_num;
	sys_sub = sub;
}


/*
 * menu_xo -- change extra weapon checkmarks in option menu
 */

void
menu_xo(register PLAYER *p)
{
	struct MenuItem *mip;
	register int	i;

	mip = ((p == &plr[0]) ? port0_menu : port1_menu);
	mip += PT_XTRA_OFF;
	Disable();
	for (i = 0; mip; i++, mip = mip->NextItem) {
		if (i == p->xtra_opt)
			mip->Flags |= CHECKED;
		else
			mip->Flags &= ~CHECKED;
	}
	Enable();
}


/*
 * do_pointer -- maintain stop_it flags and intuition pointer sync
 */

void
do_pointer(ulong set, int code)
{
	register uchar		*up;
	register OBJ		*op;
	register uchar		val;
	static ushort __chip	pause_im[] = {		/* pause pointer */
	  0, 0,
	  0x8000, 0xBFFF, /*3022222222222222*/
	  0x3FFE, 0x4001, /*0211111111111112*/
	  0x4002, 0x8001, /*2100000000000012*/
	  0x5FC2, 0x9FE1, /*2103333333200012*/
	  0x5022, 0x9831, /*2103200000320012*/
	  0x5012, 0x9819, /*2103200000032012*/
	  0x5022, 0x9831, /*2103200000320012*/
	  0x5FC2, 0x9FE1, /*2103333333200012*/
	  0x5002, 0x9FC1, /*2103222222000012*/
	  0x5002, 0x9801, /*2103200000000012*/
	  0x5002, 0x9801, /*2103200000000012*/
	  0x5002, 0x9801, /*2103200000000012*/
	  0x5002, 0x9801, /*2103200000000012*/
	  0x4002, 0x8001, /*2100000000000012*/
	  0x7FFE, 0x8001, /*2111111111111112*/
	  0x0000, 0xFFFF, /*2222222222222222*/
	  0, 0
	};
	static ushort __chip	normal_im[] = { 	 /* normal pointer */
	  0, 0,
	  0xE44E, 0x07C0, /*111003222300111*/
	  0xC286, 0x0380, /*110000323000011*/
	  0xA28A, 0x0380, /*101000323000101*/
	  0x1290, 0x0380, /*000100323001000*/
	  0x0AA0, 0x0380, /*000010323010000*/
	  0x86C2, 0x8382, /*300001323100003*/
	  0x7C7C, 0xFD7E, /*233333020333332*/
	  0x0000, 0xFEFE, /*222222202222222*/
	  0x7C7C, 0xFD7E, /*233333020333332*/
	  0x86C2, 0x8382, /*300001323100003*/
	  0x0AA0, 0x0380, /*000010323010000*/
	  0x1290, 0x0380, /*000100323001000*/
	  0xA28A, 0x0380, /*101000323000101*/
	  0xC286, 0x0380, /*110000323000011*/
	  0xE44E, 0x07C0, /*111003222300111*/
	  0, 0
	};
	extern SPR_IM __far	spr_im[];
	extern struct SimpleSprite sprite[];

	up = &stop_it.c[code];
	if (set)
		*up += 1;				/* set */
	else if (*up)
		*up -= 1;				/* clear */
	val = *up;

	switch (code) {
	/*
	 * Paused -- update pause menu
	 */
	case STOP_PAUSE:
		if (val > 1)
			*up = 1;			/* clip to one */
		Disable();
		if (val)
			project_menu->Flags |= CHECKED;
		else
			project_menu->Flags &= ~CHECKED;
		Enable();
	/*
	 * If stopped because the screen isn't being displayed, or if we
	 * are setting up the game, then hide the sprites, else show them.
	 */
	CASE STOP_SCREEN:
		if (val) {
			for (op = ohead; op; op = op->o_next) { /* hide */
				if (op->o_sprptr && op->o_sprptr->num)
					hide_sprite(op->o_sprptr);
			}
		}
		else {
			for (op = ohead; op; op = op->o_next) { /* show */
				if (!op->o_sprptr)
					continue;
				show_sprite(op->o_sprptr, op->o_sprd);
			}
		}
	}

	/*
	 * if screen inactive or no players using joyport 0 and sprite 0
	 * not in use, then turn on mouse control
	 */

	mouse_ctl(stop_it.c[STOP_SCREEN] || stop_it.s[STOP_WJS0] == FALSE);

	/*
	 * If paused, show the pause pointer, else if using menu or
	 * requester, go back the Intuition pointer, else use sprite[0].
	 */

	if (stop_it.c[STOP_PAUSE]) {
		if (stop_it.c[STOP_PTR] != PTR_PAUSE) {
			stop_it.c[STOP_PTR] = PTR_PAUSE;
			SetPointer(win, pause_im, NEL(pause_im) / 2 - 2,
					16, 0, 0);
		}
	}
	else if (stop_it.c[STOP_WAIT]) {
		if (stop_it.c[STOP_PTR] != PTR_INTUI) {
			stop_it.c[STOP_PTR] = PTR_INTUI;
			SetPointer(win, normal_im, NEL(normal_im) / 2 - 2,
					15, -7, -7); 		/* show */
		}
	}
	else if (stop_it.c[STOP_SPR0]) {
		if (stop_it.c[STOP_PTR] != PTR_SPR0) {
			stop_it.c[STOP_PTR] = PTR_SPR0; 	/* sprite0 */
			SetPointer(win, spr_im->start,
					sprite->height, sprite->height, 0, 0);
			/*
			 * Sprite 0's object had better be at ohead
			 */
			op = ohead;
			spr_image(sprite, op->o_image);
			show_sprite(sprite, op->o_sprd);
		}
	}
	else {
		if (stop_it.c[STOP_PTR] != PTR_INVIS) {
			stop_it.c[STOP_PTR] = PTR_INVIS;	/* invis */
			*((ulong *)&spr_im->image[0]) = 0;
			SetPointer(win, spr_im->start, 0, 0, 0, 0);
		}
	}

	if (stop_it.l || stop_it.c[STOP_GAME]) {
		flush_sound(NULL);
	}
	else {
		Disable();
		if (copper_data.c_counter < 5)
			copper_data.c_counter = 5;
		Enable();
		copper_data.c_clxdat = 0;
		set_custom();
	}
}


/*
 * do_pause -- toggle the game pause flag
 */

void
do_pause(void)
{
	do_pointer((project_menu->Flags & CHECKED) ^ CHECKED, STOP_PAUSE);
}


/*
 * screentoback -- do a ScreenToBack and a do_pointer
 */

void
screentoback(void)
{
	ScreenToBack(screen);
	do_pointer(1, STOP_SCREEN);
}

/*
 * screentofront -- do a ScreenToFront and a do_pointer
 */

void
screentofront(void)
{
	ScreenToFront(screen);
	do_pointer(0, STOP_SCREEN);
}


/*
 * pushscreenback -- do a WBenchToFront, and activate its first window
 *
 * Kludge Alert!  There is no guarantee that the workbench will open,
 *		  or that there will be a window to activate.  Also, the
 *		  window may not be in MEMF_PUBLIC, so this may fail in
 *		  the future.
 *		  This is probably a misuse of the GetScreenData function.
 */

void
pushscreenback(void)
{
	register struct Window	*wp;
	struct Screen		wb_scr;

	if (!WBenchToFront()) {
		ScreenToBack(screen);		/* got no workbench */
		return;
	}
	Forbid();
	if (GetScreenData(&wb_scr, sizeof(wb_scr), WBENCHSCREEN, NULL) &&
	    (wp = wb_scr.FirstWindow) != NULL)
		ActivateWindow(wp);
	Permit();
}


/*
 * domenu -- process menu picks
 */

void
domenu(register USHORT mnum)
{
	register struct MenuItem	*mi;
	register uint			item, sub;
	register struct MenuItem	*mp;
	register S_SYSTEM		*sys;

	for (; mnum != MENUNULL && (mi = ItemAddress(menu, mnum));
			  mnum = mi->NextSelect) {
		item = ITEMNUM(mnum);
		if (item == NOITEM)
			continue;		/* didn't pick anything */
		sub = SUBNUM(mnum);
		switch (MENUNUM(mnum)) {
		case 0: 	/* project menu */
			switch (item) {
			case 0: 	/* Pause */
				do_pointer(mi->Flags & CHECKED, STOP_PAUSE);
			CASE 1: 	/* New */
				new_game(1);
			CASE 2: 	/* About */
				about_req();
			CASE 3: 	/* Screen to Front */
				screentofront();
			CASE 4: 	/* Screen to Back */
				pushscreenback();
			CASE 5: 	/* Quit */
				quit_req();
			}
		CASE 2: 	/* systems menu */
			mp = mi;
			/*
			 * if you toggled "Asteroid", redo the last choice
			 */
			if (item == SM_AST) {
				item = sys_item;
				if (item > 1)
					continue;	/* no new_game */
				sub = sys_sub;
				mp = sys_mp;
			}
			else if (sub == NOSUB && item != 0)
				continue;		/* need subitem */
			/*
			 * if the asteroid is checked (system_menu[SM_AST])
			 * then pick from alternate systems
			 */
#define ASTEROID() (system_menu[SM_AST].Flags & CHECKED)
			switch (item) {
			case 0:
				sub = 0;
				sys = (ASTEROID() ? pri0a_sys : pri0_sys);
			CASE 1:
				sys = (ASTEROID() ? pri1a_sys : pri1_sys);
			CASE 2:
				sys = pri2_sys;
			CASE 3:
				sys = pri3_sys;
			CASE 4:
				sys = pri4_sys;
			CASE 5:
				sys = misc_sys;
			DEFAULT:
				continue;		/* huh? */
			}
			Disable();
			sys_mp->Flags &= ~CHECKED;	/* Mutual Exclude */
			mp->Flags |= CHECKED;		/* pick new */
			Enable();
			s_sys = &sys[sub];
			sys_mp = mp;
			sys_item = item;
			sys_sub = sub;
			new_game(0);
		CASE 1: 	/* options menu */
			if (sub == NOSUB && item > OM_WRAP)
				break;
			switch (item) {
			case 0: 	/* beginner mode */
				beginner_mode = mi->Flags & CHECKED;
			CASE 1: 	/* random game */
				rand_system = mi->Flags & CHECKED;
			CASE 2: 	/* screen boundary */
				bound_type = mi->Flags & CHECKED;
				bound_box();
			CASE 3: 	/* ship thrust */
				ship_thrust = THRUST_SHIFT - sub;
			CASE 4: 	/* torp vel */
				torp_vel = TORP_VEL + 1 - sub;
			CASE 5: 	/* torp lifetime */
				torp_life = TORP_LIFE << sub;
			CASE 6: 	/* green ship options */
				if (sub <= 1)
					plr_1.jp =
						(sub ? &KEYP : &joyport[0]);
				else
					plr_1.xtra_opt = sub - PT_XTRA_OFF;
joy0chk:
				stop_it.c[STOP_JOY0] =
				  (plr_1.jp == joyport || plr_2.jp == joyport);
			CASE 7: 	/* purple ship options */
				if (sub <= 1)
					plr_2.jp =
						(sub ? &KEYP : &joyport[1]);
				else
					plr_2.xtra_opt = sub - PT_XTRA_OFF;
				goto joy0chk;
			}
		}
	}
}

