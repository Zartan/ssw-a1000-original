/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * init.c -- solar system wars program initialization code
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

#ident "$Id: init.c 1.2 1997/01/19 07:04:58 jamesc Exp jamesc $"

#include "ssw.h"
#include <exec/memory.h>
#include <exec/interrupts.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <graphics/gfxbase.h>
#include <libraries/dos.h>
#include <libraries/dosextens.h>
#include <hardware/cia.h>
#include <hardware/custom.h>
#include <hardware/intbits.h>
#include <dos/dos.h>
#include <string.h>
#include <stdlib.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/dos.h>
#include <proto/timer.h>

#ifndef NULL
#define NULL		(0)
#endif

#define MEM_PC		(ULONG)(MEMF_PUBLIC | MEMF_CLEAR)
#define MEMALLOC(v, t)  ((v) = (t) memalloc((ULONG)sizeof(*(v))))

#define SCR_LEFT_OFFSET 4			/* screen left offset */


ulong			*gravtbl[GRAV_SIZE];

struct Library		*TimerBase;

Static struct timerequest *tm_req;
Static uchar		tm_open;
char			copper_signum = -1;
Static uchar		copper_open;
Static uchar		reseed_way;
COPPER_DATA		copper_data;
extern void		ssw_copper(void);
Static struct Interrupt copper_interrupt = {
	{ NULL, NULL, NT_INTERRUPT, 10, "ssw_cop" },
	(APTR)(&stop_it), ssw_copper
};

extern ulong		clxdat_junk;		/* move elsewhere */

long			y_height;
long			y_mid;
long			y_spr_max;
long			y_spr_cdf;
long			screen_x, screen_y;	/* scr X,Y offset (prefs) */

struct IntuitionBase	*IntuitionBase;
struct GfxBase		*GfxBase;
struct Screen		*screen;
struct Window		*win;
struct MsgPort		*userport;
struct ViewPort 	*vp;
struct RastPort 	*rp , *rp2;
struct RasInfo		*rinfo2;
struct BitMap		*bmap2;
uint			dualpf_flag;
struct UCopList 	*u_cop_list;

Static char		dots[] = " ...";

/* list of display IDs that will work */

#if 1
Static ULONG	disp_id_list[] = {
	VGALORESDBL_KEY,			IS_VGA,
	PAL_MONITOR_ID | LORESSDBL_KEY,		IS_PAL,
	NTSC_MONITOR_ID | LORESSDBL_KEY,	IS_NTSC,
	PAL_MONITOR_ID | LORES_KEY,		IS_PAL,
	NTSC_MONITOR_ID | LORES_KEY,		IS_NTSC,
};
#else
Static ULONG	disp_id_list[] = {
	VGALORESDPF_KEY,		IS_VGA,
	DBLPALLORESDPF_KEY,		IS_PAL,
	DBLNTSCLORESDPF_KEY,		IS_NTSC,
	PAL_MONITOR_ID | LORESDPF_KEY,	IS_PAL,
	NTSC_MONITOR_ID | LORESDPF_KEY,	IS_NTSC,
};
#endif

Static ushort	ship1_image[NEL(ship_image)], ship2_image[NEL(ship_image)];

#if 0
struct TextAttr topaz_60 = {
	"topaz.font", TOPAZ_SIXTY, FS_NORMAL, FPF_ROMFONT
};
#endif

struct TextAttr topaz_80 = {
	"topaz.font", TOPAZ_EIGHTY, FS_NORMAL, FPF_ROMFONT
};

Static LONG	extns_err_code;
Static UWORD	pens[] = { 0xFFFF, 0xFFFF };

Static struct TagItem	scr_taglist[] = {
	/* V37 tags */
	{ SA_DisplayID, NTSC_MONITOR_ID | LORES_KEY },
	{ SA_Pens, (ULONG)pens },
	{ SA_ErrorCode, (ULONG)&extns_err_code },
	{ SA_AutoScroll, FALSE },
	{ SA_ShowTitle, FALSE },
	/* V39 tags */
	{ SA_Draggable, FALSE },
	{ SA_Exclusive, TRUE },
	{ TAG_DONE },
};

Static struct ExtNewScreen NewScreen = {
	SCR_LEFT_OFFSET, 0, RWIDTH, NTSC_HEIGHT, /* left/top edge, width, height */
	1 /* DEPTH */, 0, 1,		/* depth, DetailPen, BlockPen	*/
	SPRITES /* | DUALPF */,		/* display modes		*/
	CUSTOMSCREEN | SCREENBEHIND | SCREENQUIET | NS_EXTENDED, /* scr type */
	&topaz_80, NULL,		/* default font, screen title	*/
	NULL, NULL,			/* gadgets, custom bitmap	*/
	scr_taglist,			/* v36+ tag item list		*/
};

#define ICF_NOV (RAWKEY | MENUPICK | GADGETUP)
#define ICF_VER (ICF_NOV | ACTIVEWINDOW | INACTIVEWINDOW | MENUVERIFY | REQVERIFY)

Static struct NewWindow NewWindow = {
	0, 0, WIDTH, NTSC_HEIGHT,	/* left/top edge, width, height */
	0, 1,				/* DetailPen, BlockPen		*/
	ICF_NOV,			/* IDCMPFlags			*/
					/* Window flags 		*/
	SMART_REFRESH | NOCAREREFRESH | BACKDROP | BORDERLESS | ACTIVATE,
	NULL, NULL,			/* gadgets, checkmark image	*/
	NULL,				/* window title 		*/
	NULL,				/* put screen pointer here	*/
	NULL,				/* super bitmap 		*/
	0, 0, 0, 0,			/* min/max width/heigth 	*/
	CUSTOMSCREEN			/* screen type			*/
};

#define BKGND_COLOR	0x743		/* background star color (game)	*/

USHORT	colors[NUM_COLORS] = {
/*00*/	0x000, 0xDDD, 0xAAA, 0x466, 0x000, 0x000, 0x000, 0x000,
/*08*/	0x888, 0x743, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000,
/*10*/	0x000, 0xBB3, 0xEE3, 0x883,
};
#if 0
USHORT	def_colors[NUM_COLORS] = {
	0x000, 0xDDD, 0x888, 0xAAA, 0, 0, 0, 0, 0x888, 0x743
};
USHORT	gam_colors[NUM_COLORS] = {
	0x000, 0xDDD, 0xAAA, 0x466, 0, 0, 0, 0, 0, BKGND_COLOR
};
USHORT	sav_colors[NUM_COLORS] = { 0 };
#endif

struct SimpleSprite		sprite[8];
SPR_IM __chip			spr_im[8];
Static uint			sprite_bits;

extern struct Menu		menu[];

	/* used to keep track of which system was selected from the menu */
extern S_SYSTEM 		*s_sys;

PLAYER				plr[2];

extern struct Custom far	custom;

extern ushort			*plane[];	/* bitplane kludge	*/

extern char			*strnadd(char *dest, char *src, int size);
extern void			end_audio(void);
extern void			free_sounds(void);
extern void			end_game_port(void);
/*#define _exit(val)	XCEXIT(val)	/* use quick exit */
/*#define _exit(val)		exit(val)	/* use default exit */
extern void			verify_on(void);
extern void			wait1(void);
extern void			mouse_ctl(int);
extern void			parse_file(BPTR file);


/*
 * die -- shutdown and exit
 */

void
die(long exval)
{
	register long	i;

	do_pointer(1, STOP_SCREEN);	/* hide sprites */

	for (i = GRAV_SIZE; --i >= 0; ) {
		if (gravtbl[i])
			FreeMem(gravtbl[i], ULGRAV_SIZE);
	}
	end_game_port();
	if (u_cop_list) {
		FreeVPortCopLists(vp);
		RemakeDisplay();
	}
	for (i = 1; i < 8; i++) {
		if (sprite_bits & (1 << i))
			FreeSprite(i);
	}
	end_audio();
	free_sounds();
#if 1
	if (dualpf_flag) {
		Forbid();
		vp->RasInfo->Next = NULL;
		vp->Modes &= ~DUALPF;
		Permit();
		MakeScreen(screen);
		RethinkDisplay();
	}
#endif /* 0 */
	if (win) {
		/* ClearPointer(win); */
		ClearMenuStrip(win);
		CloseWindow(win);
	}
	if (screen)
		CloseScreen(screen);
	if (copper_open)
		RemIntServer(INTB_COPER, &copper_interrupt);
	if (copper_signum >= 0)
		FreeSignal(copper_signum);
	if (rp2)
		FreeMem(rp2, (ULONG)sizeof(*rp2));
#if 1
	if (bmap2) {
		if (bmap2->Planes[0])
			FreeRaster(bmap2->Planes[0], WIDTH, y_height);
		FreeMem(bmap2, (ULONG)sizeof(*bmap2));
	}
	if (rinfo2)
		FreeMem(rinfo2, (ULONG)sizeof(*rinfo2));
#endif /* 0 */
	(void) OpenWorkBench(); /* make "Intuition: Style" pg 219 happy */
	if (GfxBase)
		CloseLibrary((struct Library *)GfxBase);
	if (IntuitionBase)
		CloseLibrary((struct Library *)IntuitionBase);
	if (tm_open)
		close_dev(tm_open, TO_IOR(tm_req));

	exit(exval);
}

void
text(char *str)
{
	Text(rp, str, strlen(str));
}

Static char	numstr[12];


char *
my_i2d(register long num)
{
	register char	*p;
	int		neg;

	if (num < 0) {
		num = -num;
		neg = 1;
	}
	else
		neg = 0;

	p = &numstr[sizeof(numstr) - 1];
	*p = '\0';
	do {
		*--p = (num % 10) + '0';
	} while (num /= 10);

	if (neg)
		*--p = '-';

	return (p);
}


/*
 * rev_text -- print reverse video text centered on the X axis around x,y
 */

void
rev_text(long x, long y, BYTE *txt)
{
	int	len;

#if 0
	SetWrMsk(rp, BP_ALL_MASK);
#endif /* 0 */
	SetDrMd(rp, JAM1);
	SetBPen(rp, 1);
	SetAPen(rp, 1);
	len = strlen(txt) * 4;
	x -= len;
	RectFill(rp, x - 1, y - 7, x + 2 * len, y + 1);
	SetAPen(rp, 0);
	Move(rp, x, y);
	text(txt);
	WaitBlit();
}


Static ushort	score_x[] = { WIDTH - 20, 20 };
#define SCORE_Y (y_mid + 20)

/*
 * blink_scores -- blink any winning scores
 */

void
blink_scores(void)
{
	register int	n;

#if 0
	SetWrMsk(rp, BP_TORP_MASK);
#endif /* 0 */
	SetDrMd(rp, COMPLEMENT);
	SetAPen(rp, 0);
	SetBPen(rp, 1);
	n = (plr_1.score < plr_2.score);
	Move(rp, score_x[n] - 8, SCORE_Y);
	text(my_i2d(plr[n].score));
}


/*
 * show_scores -- print the player's scores
 */

void
show_scores(void)
{
	register int	n;

	flush_sound(NULL);
	for (n = 2; --n >= 0; )
		rev_text(score_x[n], SCORE_Y, my_i2d(plr[n].score));
}

/*
 * clear_scores -- remove the player's scores
 */

void
clear_scores(void)
{
	SetRast(rp, 0);
	WaitBlit();
}


/*
 * do_overtime -- print the overtime message
 */

void
do_overtime(void)
{
	rev_text(WIDTH / 2, y_mid + 40, "Overtime!!");
}


/*
 * memalloc -- alloc memory with flags MEM_PC
 */

void *
memalloc(ULONG size)
{
	void	*ptr;

	if ((ptr = AllocMem(size, MEM_PC)) == NULL) {
		err("Can't AllocMem! size=%ld", size);
		wait1();
		die(1);
	}
	return (ptr);
}


/*
 * delay -- delay the given number of 50 Hz units
 */

void
delay(long timeout)
{
	tm_req->tr_node.io_Command = TR_ADDREQUEST;
	tm_req->tr_time.tv_secs = timeout / 50;
	tm_req->tr_time.tv_micro = (timeout % 50) * (1000000L / 50);
	DoIO(TO_IOR(tm_req));
}


/*
 * reseed -- call srand64 with the current time and other semi-random values
 */

void
reseed(void)
{
	extern volatile struct CIA far	ciaa, ciab;

	switch (os_1_3 ? reseed_way % 3 : reseed_way % 5) {
	case 0:
		tm_req->tr_node.io_Command = TR_GETSYSTIME;
		DoIO(TO_IOR(tm_req));
	CASE 1:
		/* use CIA VSync and E-clock counters */
		tm_req->tr_time.tv_micro = ((ULONG)ciaa.ciatodhi << 16) +
			((ULONG)ciaa.ciatodmid << 8) + ciaa.ciatodlow;
		tm_req->tr_time.tv_secs = ((ULONG)ciab.ciatodhi << 16) +
			((ULONG)ciab.ciatodmid << 8) + ciab.ciatodlow;
	CASE 2:
		/* use beam position counter & analog joystick regs */
		tm_req->tr_time.tv_micro = ((ULONG)custom.vposr << 16) +
			(ULONG)custom.vhposr;
		tm_req->tr_time.tv_secs = ((ULONG)custom.pot0dat << 16) +
			(ULONG)custom.pot1dat;
	/*
	 * In OS 2.0+, can use the GetSysTime and ReadEClock funcs.
	 */
	CASE 3:
		GetSysTime(&tm_req->tr_time);
	CASE 4:
		ReadEClock((struct EClockVal *)&tm_req->tr_time);
	}

	if ((os_1_3 ? reseed_way / 3 : reseed_way / 5) & 1)
		srand64(tm_req->tr_time.tv_micro, tm_req->tr_time.tv_secs);
	else
		srand64(tm_req->tr_time.tv_secs, tm_req->tr_time.tv_micro);
	++reseed_way;
	(void) rand64();
}


/*
 * set_custom -- isolate hardware tinkering
 */

void
set_custom(void)
{
	/* collision control reg (see 1.3 RKM Hardware, pg 209):
	 * enable sprites 1, 3, 5, and 7, bit plane 1, match val 1 for bp 1
	 */
	custom.clxcon = 0xf041;

	/* read clxdat to clear it after mode change above */
	clxdat_junk = custom.clxdat;

	/* enable joystick button 2 detection by setting all bits to output
	 * and the output data to 1
	 */
	custom.potgo = 0xff00;
}


/*
 * init_player
 */

Static void
init_player(register PLAYER *p, register int pnum)
{
	register int	n;
	register OBJ	*torp;

	/* zero p, except for preserved fields at structure end */

	(void) memset((char *)p, 0,
  sizeof(*p) - sizeof(p->jp) - sizeof(p->score) - sizeof(p->xtra_opt));

	p->ec_cnt = ENGINE_COUNT;
	p->angle = NUM_ANGLES / 4;

	if (pnum) {
		pnum = NUM_SOUNDS_PER_PLAYER;
		p->plsk.o_sprptr = &sprite[7];
		p->plsk.o_ship = &plr_1;
	}
	else {
		p->plsk.o_sprptr = &sprite[5];
		p->plsk.o_ship = &plr_2;
	}

	for (n = 0; n < NUM_SOUNDS_PER_PLAYER; n++, pnum++)
		p->p_sounds[n] = &sounds[pnum];

	torp = p->torps;
	for (n = NUM_TORPS; --n >= 0; torp++) {
		torp->o_next = p->free_torps;
		p->free_torps = torp;
		torp->o_type = OTYPE_TORP;
		torp->o_mass = M_NONE;
		torp->o_ship = p;
	}

	for (n = NUM_DEBRIS; --n >= 0; torp++) {
		torp->o_type = OTYPE_TORP;
		torp->o_mass = M_NONE;
		torp->o_ship = p;
	}

	p->plsk.o_mass = M_NONE;
	p->plsk.o_energy = -1;
}


/*
 * bound_box -- draw a box if bounce system, else erase it
 */

void
bound_box(void)
{
	static SHORT	poly_array[] = {
		0,		0,
		WIDTH - 1,	0,
		WIDTH - 1,	99,
		0,		99,
		0,		0
	};

#if 0
	SetWrMsk(rp, BP_PF2_MASK);
#endif /* 0 */
	SetDrMd(rp2, JAM1);
	SetAPen(rp2, bound_type ? 0 : 1);
	poly_array[5] = poly_array[7] = y_height - 1;
	PolyDraw(rp2, NEL(poly_array) / 2, poly_array);
	WaitBlit();
}


/*
 * reverse_ohead -- invert the object linked list
 */

Static void
reverse_ohead(void)
{
	register OBJ	*op, *next;

	otail = op = ohead;
	ohead = NULL;
	for ( ; op; op = next) {
		next = op->o_next;
		op->o_next = ohead;
		ohead = op;
	}
}


/*
 * set_scr -- clear the window and redraw the objects
 */

void
set_scr(int do_rand)
{
	register IOBJ		*iop;
	register OBJ		*op;
	register SSW_IMAGE	*ssw;
	register int		n, i, c;
	extern int		rand_system;
	extern void		rand_game(void);

	flush_sound(NULL);

	SetDrMd(rp, JAM2);
	SetRast(rp, 0);
	SetRast(rp2, 0);

	WaitTOF();				/* hide sprites off screen */
	sprite->height = 0;
	*(ulong *)spr_im->start = 0;
	*(ulong *)spr_im->image = 0;
	do_pointer(1, STOP_SCREEN);		/* hide all obj sprites */
	stop_it.c[STOP_SPR0] = 0;

	init_player(&plr_1, 0);			/* do some work to use up */
	init_player(&plr_2, 1);			/* some time that isn't */
	plr_2.angle += NUM_ANGLES / 2;		/* synched to the v-beam */

	reseed();				/* might use vhposr */

	if (rand_system && do_rand)
		rand_game();			/* pick a random game? */

	iop = s_sys->iobj;
	ohead = otail = NULL;
	op = obj;
	for (n = s_sys->num_obj; --n >= 0; op++, iop++) {
		if (iop->otype == OTYPE_SHIP) {
			/* ships always come at the end of the list, so
			 * we can safely move op from the obj array to
			 * the private objects in each player structure.
			 */
			op = (plr_1.obj.o_energy ? &plr_2.obj : &plr_1.obj);
		}
		op->o_next = ohead;
		ohead = op;

		op->o_x = op->o_nx =
		  (op->o_ix = op->o_nix = iop->x) << CD_FRAC;
		op->o_y = op->o_ny =
		  (op->o_iy = op->o_niy = iop->y + y_mid) << CD_FRAC;
		op->o_vx = op->o_nvx = iop->vx;
		op->o_vy = op->o_nvy = iop->vy;
		op->o_energy = 0x7fffffff;
		if ((uint)(c = iop->sprite) >= 8 || !(ssw = iop->ssw_im)) {
			DisplayBeep(screen);
			op->o_mass = M_NONE;
			op->o_offset = 0;
			op->o_sprptr = NULL;
			op->o_image = NULL;
			op->o_energy = 0;
			op->o_type = OTYPE_TORP;
			continue;
		}
		op->o_offset = i = ssw->offset;
		op->o_image = ssw->image;
		op->o_sprptr = &sprite[c];
		op->o_sprptr->height = ssw->size;
		spr_image(op->o_sprptr, op->o_image);
		op->o_sprd = op->o_nsprd =
		  move_sprite(op->o_sprptr, op->o_ix - i, op->o_iy - i);
		if (c)
			MoveSprite(NULL, op->o_sprptr, HIDE_X, HIDE_Y);
		else
			stop_it.c[STOP_SPR0] = TRUE;
		c = ((c >> 1) << 2) + FIRST_SPR_COLOR; /* spr pair colors */
		op->o_color = c;

		switch (op->o_type = iop->otype) {
		case OTYPE_ASTEROID:
		default:
			op->o_mass = M_NONE;
		CASE OTYPE_SHIP:
			if (op == &plr_1.obj) {
				plr_1.color_ptr = ssw->color;
				plr_1.color = c;
				op->o_image = ship1_image;
			}
			else {
				plr_2.color_ptr = ssw->color;
				plr_2.color = c;
				op->o_image = ship2_image;
			}
			CopyMem(ship_image, op->o_image,
			  sizeof(ship1_image));
			op->o_mass = M_NONE;
		CASE OTYPE_RED_STAR:
			op->o_mass = M_RED;
		CASE OTYPE_YELLOW_STAR:
			op->o_mass = M_YELLOW;
		CASE OTYPE_BLUE_STAR:
			op->o_mass = M_BLUE;
		CASE OTYPE_SM_HOLE:
			op->o_mass = M_SM_HOLE;
		CASE OTYPE_MED_HOLE:
			op->o_mass = M_MED_HOLE;
		CASE OTYPE_LRG_HOLE:
			op->o_mass = M_LRG_HOLE;
		}

		for (i = 0; i < 3; i++, c++) {
			colors[c] = ssw->color[i];
		}
	}

	rotate_ship(&plr_1);
	rotate_ship(&plr_2);

	LoadRGB4(vp, colors, NEL(colors));
	changed_color = 1;
	update_colors();

	SetDrMd(rp2, JAM1);
	SetAPen(rp2, 1);
#if 0
	SetDrMd(rp, JAM1);
	SetWrMsk(rp, BP_PF2_MASK);
	SetAPen(rp, 8);
#endif /* 0 */
	for (n = (WIDTH * y_height) >> 8; --n >= 0; ) {
		WritePixel(rp2, RND(WIDTH), RND(y_height));  /* draw star */
	}
	if (bound_type == 0)
		bound_box();
	SetWrMsk(rp, BP_TORP_MASK);

	/*
	 * reverse the object list to put the suns at the head
	 */
	reverse_ohead();
	/*
	 * if stationary asteroid, do random velocity
	 */
	for (op = ohead; op && op->o_type == OTYPE_ASTEROID &&
	     (op->o_vx | op->o_vy) == 0; op = op->o_next) {
		op->o_nvx = ARND(CD_UNIT >> 1) - (CD_UNIT >> 2);
		op->o_nvy = ARND(CD_UNIT >> 1) - (CD_UNIT >> 2);
	}

	reseed();				/* not synched to v beam */

	WaitBlit();
	WaitBOVP(vp);

	do_pointer(0, STOP_SCREEN);	/* show sprites */
	if (stop_it.c[STOP_SPR0])
		show_sprite(sprite, ohead->o_sprd);
}


/*
 * new_game -- start a new game
 */

void
new_game(int do_rand)
{
	plr_1.score = plr_2.score = 0;
	if (!stop_it.c[STOP_GAME])
		do_pointer(1, STOP_GAME);
	set_scr(do_rand);
	show_scores();
	copper_data.c_counter = AFTER_DELAY / 2 + 2;
	game_state = GAME_NEW;
}


/*
 * screen_offset -- find screen offset from sprite # 1 at 0,0
 *
 * Note the truly bizarre layout of the Sprite Control Words -- the hardware
 * designers seem to have gone out of their way to make things difficult:
 *
 * 31		24 23		16 15		 8 7 6 5 4 3 2 1 0
 *+---------------------------------------------------------------+
 *| VSTART low	  | HSTART high   | VSTOP low	  |A|0 0 0 0|X|Y|Z|
 *+---------------------------------------------------------------+
 * A = Attach sprite bit		X = VSTART high bit -^ ^ ^
 *					Y = VSTOP high bit ----+ |
 *					Z = HSTART low bit ------+
 */

Static void
screen_offset(void)
{
	register struct sprite_control {
		uint	vstart_low  : 8;
		uint	hstart_high : 8;
		uint	vstop_low   : 8;
		uint	attach	    : 1;
		uint	unused	    : 4;
		uint	vstart_high : 1;
		uint	vstop_high  : 1;
		uint	hstart_low  : 1;
	} *spr_ctl;

	spr_ctl = (struct sprite_control *) sprite[1].posctldata;
	screen_y = spr_ctl->vstart_low | (spr_ctl->vstart_high << 7);
	screen_x = spr_ctl->hstart_low | (spr_ctl->hstart_high << 1);
	screen_x += SCR_LEFT_OFFSET;
}


/*
 * mv_text -- move and write text on rp
 */

void
mv_text(long x, long y, BYTE *str)
{
	Move(rp, x, y);
	Text(rp, str, strlen(str));
	WaitBlit();
}


#define TEXT_X	28		/* opening text X offset		*/
#define TEXT_Y	10		/* opening text Y offset per line	*/

/*
 * opening -- print a string that says "Opening %s ..."
 */

Static void
opening(char *path, int y)
{
	register char	*p;
	char		str[40];

	if (y <= 0)
		return; 	/* no text */

	str[sizeof(str) - 1] = '\0';
	p = strnadd(str, "Opening ", sizeof(str) - 1);
	p = strnadd(p, path, sizeof(str) - 1 + (str - p));
	(void) strnadd(p, dots, sizeof(str) - 1 + (str - p));
	mv_text(TEXT_X, y, str);
}


/*
 * open_ssw -- try to Open the file, then try to Open SSW:file, else die
 *		Remembers where it found the last file opened and tries
 *		there next time.
 */

Static BPTR
open_ssw(char *path, int y)
{
	register BPTR	fd;
	register int	n;
	register char	*p;
	char		file[12];		/* increase me as needed */
	static int	add_ssw_flag = -1;

	file[sizeof(file) - 1] = '\0';
	(void) strnadd(file, path, sizeof(file) - 1);

	for (n = 2; --n >= 0; ) {
		if (n == 0 || add_ssw_flag == 0) {
			/* Add SSW: prefix */
			n = 0;
			p = strnadd(file, "SSW:", sizeof(file) - 1);
			(void) strnadd(p, path,
					sizeof(file) - 1 - (p - file));
		}

		opening(file, y);
		if ((fd = Open(file, MODE_OLDFILE)) != 0) {
			add_ssw_flag = n;
			if (y > 0)
				text(" OK");
			return (fd);
		}
		if (y > 0) {
			y += TEXT_Y;
			text(" Failed!");
		}
	}
	err("Can't open '%s'!\nError = %ld", file, IoErr());
	wait1();
	die(1);
	/* NOTREACHED */
	return (0L);
}


/*
 * chk_disp_modes -- check out some more advanced display modes.
 *
 * Sets scr_taglist->ti_Data and mon_type accordingly.
 */

Static void
chk_disp_modes(int argc, char **argv)
{
	int	i;
	ULONG	disp_id;
	uchar	best[2];

	best[0] = NEL(disp_id_list) - 2;
	best[1] = 0;

	/* use a diplay ID from arguments, if it is on the list */

	if (--argc > 0) {
		disp_id = strtol(*++argv, NULL, 0);
		for (i = NEL(disp_id_list) - 1; i >= 0; i -= 2) {
			if (disp_id == disp_id_list[i]) {
				break;
			}
		}
		if (i >= 0 && !ModeNotAvailable(disp_id)) {
			best[0] = (uchar) i;
			goto out;
		}
	}

	/*
	 * Search the list for supported display IDs.  The list is biased to
	 * favor VGA, double speed, and PAL modes, in that order.
	 */
	for (i = NEL(disp_id_list) - 1; i >= 0; i -= 2) {
		disp_id = disp_id_list[i];
		if (!ModeNotAvailable(disp_id)) {
			best[1] = best[0];
			best[0] = (uchar) i;
		}
	}

out:
	/*
	 * If we end with a PAL and a NTSC screen, use the DisplayFlags for a
	 * tie breaker.
	 */
	i = best[0];
	if (i > 0 && i + 1 == best[1]) {
		if (!(GfxBase->DisplayFlags & PAL)) {
			i = best[1];
		}
	}
	disp_id = disp_id_list[i];
	scr_taglist->ti_Data = disp_id;

	switch (disp_id & MONITOR_ID_MASK) {
	case PAL_MONITOR_ID:
		mon_type = IS_PAL;
	CASE VGA_MONITOR_ID:
		mon_type = IS_VGA;
	DEFAULT:
		mon_type = IS_NTSC;
	}
}


/*
 * initialize -- set things up
 */

void
initialize(int argc, char **argv)
{
	register long			i, words_y;
	register struct SimpleSprite	*ssp;
	register ushort 		*usp;
	struct Task			*my_task;
	static char			ssw_grv[] = "SSW.grv";
	static char			ssw_snd[] = "SSW.snd";
	extern APTR			signal_ptr(void);
	extern ulong			*exec_base(void);
	extern BYTE			ssw_str[], copyw_str[], allrr_str[],
					free_str[];
#if 0 /* use their stack check for now */
	extern int			stack_probe(void);

	if (stack_probe())	/* check for potential stack overflow */
		exit(20);
#endif /* 0 */
extern struct ExecBase	*SysBase;

	/* open libraries */

	IntuitionBase =
	  (struct IntuitionBase *)OpenLibrary("intuition.library", REV);
	if (!IntuitionBase)
		exit(2);
	GfxBase = (struct GfxBase *)OpenLibrary("graphics.library", REV);
	if (!GfxBase)
		die(3);
	os_ver = GfxBase->LibNode.lib_Version;
	os_1_3 = (os_ver >= 36) ? 0 : 1;
	mon_type = ((GfxBase->DisplayFlags & PAL) ? IS_PAL : IS_NTSC);
	if (!os_1_3) {
		chk_disp_modes(argc, argv);
	}

	switch (mon_type) {
	case IS_PAL:
		i = PAL_HEIGHT;
	CASE IS_VGA:
		i = VGA_HEIGHT;
	CASE IS_NTSC:
	default:
		i = NTSC_HEIGHT;
	}
	NewScreen.Height = NewWindow.Height = i;
	y_height  = i;
	y_mid	  = i >> 1;
	y_spr_max = i + SPRITE_HEIGHT_2 - 1;
	y_spr_cdf = y_spr_max << CD_FRAC;

	/* Try BestModeID for v3.0+ */
#if 0
	if (os_ver >= 39) {
		i = BestModeIDA(scr_taglist);
		if (i != -1) {
			scr_taglist->ti_Data = (ULONG) i;
		}
	}
#endif
	/* open/init screen and window, then add DUALPF */

	screen = OpenScreen((struct NewScreen *)&NewScreen);
	NewWindow.Screen = screen;
	if (!screen)
		die(4);
	vp = &screen->ViewPort;
	/*
	 * set priorities:
	 * SP01 in front of PF1, in front of all other sprites, in front of PF2
	 */
	vp->SpritePriorities = 0x21;
	/*
	 * To avoid 3.0+ mode promotion from switching to a 160x200 mode,
	 * shrink the viewport from 320 to 312 pixels wide, _after_ opening.
	 */
	if (os_1_3)	/* 1.3 ignores the LeftEdge field in NewScreen */
		vp->DxOffset += SCR_LEFT_OFFSET;	/* center viewport */
	vp->DWidth = WIDTH;				/* shrink viewport */
	LoadRGB4(vp, colors, NEL(colors));
	ShowTitle(screen, FALSE);

	if (!(win = OpenWindow(&NewWindow)))
		die(5);
	rp = win->RPort;
	userport = win->UserPort;
	do_pointer(1, STOP_WAIT);			/* custom pointer */
	ScreenToFront(screen);
	set_custom();

	/* display opening messages */

	SetDrMd(rp, JAM1);
	SetWrMsk(rp, BP_TORP_MASK);
	SetAPen(rp, 1);
	mv_text(17, 20, ssw_str);
	mv_text(13, 30, copyw_str);
	mv_text(77, 40, allrr_str);
	mv_text(17, 50, free_str);

	/* open timer device */

	tm_open = open_dev(TIMERNAME, UNIT_VBLANK,
			(struct IORequest **)&tm_req, sizeof(*tm_req));
	TimerBase = (struct Library *)tm_req->tr_node.io_Device;

	/* for autobackground:	give parent process time to free memory */
	delay(3);

#if 1
	/* alloc stuff for DUALPF */

	MEMALLOC(bmap2, struct BitMap *);
	i = y_height;
	InitBitMap(bmap2, 1, WIDTH, i);
	usp = (ushort *) AllocRaster(WIDTH, i);
	if (usp == NULL) {
		err("Can't alloc PF2");
		goto wait4it;
	}
	bmap2->Planes[0] = (PLANEPTR) usp;
	MEMALLOC(rp2, struct RastPort *);
	InitRastPort(rp2);
	MEMALLOC(rinfo2, struct RasInfo *);
	rinfo2->BitMap = rp2->BitMap = bmap2;
	SetRast(rp2, 0);
	Forbid();					/* jam in DUALPF */
	vp->RasInfo->Next = rinfo2;
	vp->Modes |= DUALPF;
	dualpf_flag = 1;
	Permit();
				/* copy default colors 17-19 for pointer */
	for (i = 17; i <= 19; i++) {
		if ((words_y = GetRGB4(vp->ColorMap, i)) == 0)
			words_y = 0x33;
		colors[i] = words_y;
	}
	LoadRGB4(vp, colors, NEL(colors));
	MakeScreen(screen);
	RethinkDisplay();
#else /* 0 */
  {
	err("rp has %ld planes", rp->BitMap->Depth);
	wait1();
  }
	MEMALLOC(rp2, struct RastPort *);
	InitRastPort(rp2);
	i = 0;
	Forbid();					/* find RasInfo */
	if (vp->RasInfo->Next) {
		rp2->BitMap = vp->RasInfo->Next->BitMap;
		++i;
	}
	Permit();
	if (!i) {
		err("No rp2!");
		goto wait4it;
	}
	SetWrMsk(rp2, BP_TORP_MASK);
#endif /* 0 */

	/* allocate sprites */

	for (ssp = sprite, i = 0; i < 8; i++, ssp++) {
		ssp->posctldata = &spr_im[i].start[0];
		ssp->height = SPRITE_HEIGHT;
		if (GetSprite(ssp, i) != i)
			continue;
		sprite_bits |= 1 << i;
		MoveSprite(NULL, ssp, HIDE_X, HIDE_Y);
	}
	if (sprite_bits != 0xfe) {
		err("Can't alloc all sprites!\ngot 0x%lx", sprite_bits);
		goto wait4it;
	}
	sprite->num = 0;
	MoveSprite(NULL, &sprite[1], 0, 0);	/* for screen offset check */

	/* read the 8SVX file next */

	i = open_ssw(ssw_snd, 70);
	mv_text(TEXT_X, 90, "Reading ...");
	parse_file(i);				/* closes the file */

	/* alloc gravtbl memory */

	for (i = GRAV_SIZE; --i >= 0; ) {
		if (gravtbl[i] = (ulong *) AllocMem(ULGRAV_SIZE, 0))
			continue;
		err("Can't alloc gravtbl!");
wait4it:
		wait1();
		die(1);
	}

	/* delay to show messages and load SSW.grv into array */

	i = open_ssw(ssw_grv, 100);
	mv_text(TEXT_X, 120, "Reading ...");
	for (words_y = 0; words_y < GRAV_SIZE; words_y++) {
		if (Read(i, (char *)gravtbl[words_y], ULGRAV_SIZE) !=
				ULGRAV_SIZE) {
			err("Bad Read of '%s'\nError=%ld", ssw_grv, IoErr());
			goto wait4it;
		}
	}
	Close(i);
	delay(30);

	/* alloc and install copper server */

	if ((copper_signum = AllocSignal(-1)) < 0) {
		err("Can't AllocSignal!");
		goto wait4it;
	}

	my_task = FindTask(NULL);
	srand64((ulong)my_task, (ulong)gravtbl[0]);
	reseed();

	copper_data.c_counter = -1;
	copper_data.c_clxmask = COLLIDE_MASK;
	copper_data.c_clxdat_ptr = &custom.clxdat;
	copper_data.c_signal_ptr = signal_ptr();
	copper_data.c_exec_base = (ulong *)SysBase;
	copper_data.c_task_ptr = (ulong *)my_task;
	copper_data.c_signal_mask = 1L << copper_signum;

	AddIntServer(INTB_COPER, &copper_interrupt);		/* do it */
	copper_open = 1;

	/* add a user copper list for end-of-screen interrupt */

	MEMALLOC(u_cop_list, struct UCopList *);
	CINIT(u_cop_list, 3);
	CWAIT(u_cop_list, y_height - 1, (WIDTH - 12) >> 2);
	CMOVE(u_cop_list, custom.intreq, 0x8010);
	CEND(u_cop_list);
	Forbid();
	vp->UCopIns = u_cop_list;
	Permit();
	RethinkDisplay();

	/* other assignments */

	screen_offset();

	/* init my very own bitplane kludge */

	usp = (ushort *)(rp->BitMap->Planes[0]);
	words_y = (rp->BitMap->BytesPerRow + 1) >> 1;
	for (i = 0; i < y_height; i++, usp += words_y)
		plane[i] = usp;

	plr_1.jp = &KEYP;
	plr_2.jp = &joyport[1];

	ModifyIDCMP(win, ICF_VER);		/* turn on verify messages */
	do_pointer(0, STOP_WAIT);		/* release copper int	*/
	SetMenuStrip(win, menu);		/* activate menus	*/
	new_game(0);
}
