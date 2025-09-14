/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * main.c -- solar system wars program
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

#ident "$Id: main.c 1.3 1997/01/19 07:22:19 jamesc Exp jamesc $"

#include "ssw.h"
#include <exec/io.h>
#include <exec/devices.h>
#include <devices/inputevent.h>
#include <devices/timer.h>
#include <intuition/intuition.h>
#include <graphics/gfxmacros.h>
#include <libraries/dos.h>
#include <proto/graphics.h>

	/* signals I'm interested in catching */
#define C_SIGS	(SIGBREAKF_CTRL_C | SIGBREAKF_CTRL_D | SIGBREAKF_CTRL_E)

	/* raw key codes */
#define KEY_Q		0x10
#define KEY_P		0x19
#define KEY_V		0x34
#define KEY_B		0x35
#define KEY_Y		0x15
#define KEY_N		0x36
#define KEY_O		0x18
#define KEY_ESC 	0x45
#define KEY_PAD0	0x0f
#define KEY_PAD2	0x1e
#define KEY_PAD4	0x2d
#define KEY_PAD5	0x2e
#define KEY_PAD6	0x2f
#define KEY_PADDOT	0x3c
#define KEY_PAD7	0x3d
#define KEY_PAD8	0x3e
#define KEY_PAD9	0x3f
#define KEY_ENTER	0x43
#define KEY_RET 	0x44
#define KEY_UP		0x4c
#define KEY_DOWN	0x4d
#define KEY_RIGHT	0x4e
#define KEY_LEFT	0x4f
#define KEY_L_SHIFT	0x60
#define KEY_R_SHIFT	0x61
#define KEY_L_ALT	0x64
#define KEY_R_ALT	0x65
#define KEY_HELP	0x5f
#define KEY_S		0x21

#define KEYUP		IECODE_UP_PREFIX
#define SHIFTED 	(IEQUALIFIER_LSHIFT | IEQUALIFIER_RSHIFT)

long			game_cnt;

ulong			clxdat_junk;		/* dummy write location */

extern char		copper_signum;

extern struct MenuItem	project_menu[];
extern struct MsgPort	*userport;
extern struct ViewPort	*vp;
extern struct Window	*win;

extern void		domenu(USHORT), quit_req(void);
extern void		init_game_port(void);
extern ULONG		init_audio(void);
extern void		game_cycle(void), single_step(void);
extern void		process_audio(void);
extern void		about_req(void);
extern void		blink_scores(void), show_scores(void),
			clear_scores(void), do_overtime(void);
extern void		initialize(int, char **);
extern void		set_scr(int);
extern void		screentoback(void), screentofront(void);
extern void		do_pause(void);


/*
 * SAS-C's startup will guarantee a stack at least this big:
 */
long			__stack = 8192;

long			__priority = 2;

char			*__procname = "SSW";


/*
 * process_user -- deal with stuff from the user port
 */

Static void
process_user(void)
{
	register struct IntuiMessage	*msg;
	int				doa = 0;
	APTR				iaddr;
	ULONG				class;
	USHORT				code, qualifier;
	extern int			do_gadget(APTR), do_req(int);

	while (msg = (struct IntuiMessage *)GetMsg(userport)) {
		class = msg->Class;
		code = msg->Code;
		qualifier = msg->Qualifier;
		iaddr = msg->IAddress;
		ReplyMsg(TO_MSG(msg));

		switch (class) {
		case REQCLEAR:		/* requesters gone */
			;
		CASE ACTIVEWINDOW:	/* we're active again */
			screentofront();
		CASE MENUPICK:		/* menu entry selected	*/
			domenu(code);
			do_pointer(0, STOP_WAIT);	/* done with menu */
		CASE REQVERIFY: 	/* requester op requested */
			;
		CASE INACTIVEWINDOW:	/* other window active */
			screentoback();
		CASE MENUVERIFY:	/* menu op requested */
			do_pointer(1, STOP_WAIT);	/* show pointer */
		CASE GADGETUP:
			doa += do_gadget(iaddr);
		CASE RAWKEY:
			switch (code) {
			case KEY_Q:	case KEY_ESC:
				quit_req();
			CASE KEY_P:
				do_pause();
			CASE KEY_V:
				if (!(qualifier & IEQUALIFIER_LCOMMAND))
					break;
				/* fall through! */
			case KEY_Y:
				doa += do_req(1);	/* gadget key equiv*/
			CASE KEY_B:
				if (!(qualifier & IEQUALIFIER_LCOMMAND))
					break;
				/* fall through! */
			case KEY_N:	case KEY_O:
				(void) do_req(0);	/* gadget key equiv*/
			CASE KEY_HELP:
				about_req();
/* FIX: DEBUG!!! */
CASE KEY_S:
if (stop_it.c[STOP_PAUSE])
	single_step();

			/*
			 * get raw keys & fake a joyport in KEYP
			 */
			CASE KEY_PAD5:	case KEY_DOWN:
				KEYP |= JP_FIRE;	/* fire torp */
			CASE KEY_PAD5 | KEYUP:	case KEY_DOWN | KEYUP:
				KEYP &= ~JP_FIRE;
			CASE KEY_RET:	case KEY_PAD0:
				KEYP |= JP_BACK;	/* hyper-space */
				KEYP &= ~JP_FORW;
			CASE KEY_RET | KEYUP:	case KEY_PAD0 | KEYUP:
				KEYP &= ~JP_BACK;
			CASE KEY_PAD4:	case KEY_LEFT:
				KEYP |= JP_LEFT;	/* left */
				KEYP &= ~JP_RIGHT;
			CASE KEY_PAD4 | KEYUP:	case KEY_LEFT | KEYUP:
				KEYP &= ~JP_LEFT;
			CASE KEY_PAD6:	case KEY_RIGHT:
				KEYP |= JP_RIGHT;	/* right */
				KEYP &= ~JP_LEFT;
			CASE KEY_PAD6 | KEYUP: case KEY_RIGHT | KEYUP:
				KEYP &= ~JP_RIGHT;
			CASE KEY_PAD8:	case KEY_UP:
				KEYP |= JP_FORW;	/* thrust */
				KEYP &= ~JP_BACK;
			CASE KEY_PAD8 | KEYUP:	  case KEY_UP | KEYUP:
				KEYP &= ~JP_FORW;
			CASE KEY_PAD7:
				KEYP = (KEYP & JP_BUTTONS) |
				  JP_LEFT | JP_FORW;
			CASE KEY_PAD7 | KEYUP:
				KEYP &= ~(JP_LEFT | JP_FORW);
			CASE KEY_PAD9:
				KEYP = (KEYP & JP_BUTTONS) |
				  JP_RIGHT | JP_FORW;
			CASE KEY_PAD9 | KEYUP:
				KEYP &= ~(JP_RIGHT | JP_FORW);
			/*
			 * add JP_KEY_BUT2 for other two joyports
			 */
			CASE KEY_ENTER: case KEY_PAD2:	case KEY_R_SHIFT:
				KEYP |= JP_KEY_BUT2;
				joyport[0] |= JP_KEY_BUT2;
			CASE KEY_ENTER | KEYUP:
			case KEY_PAD2 | KEYUP:	case KEY_R_SHIFT | KEYUP:
				KEYP &= ~JP_KEY_BUT2;
				joyport[0] &= ~JP_KEY_BUT2;
			CASE KEY_R_ALT: case KEY_PADDOT:
				KEYP |= JP_KEY_BUT3;
				joyport[0] |= JP_KEY_BUT3;
			CASE KEY_R_ALT | KEYUP: case KEY_PADDOT | KEYUP:
				KEYP &= ~JP_KEY_BUT3;
				joyport[0] &= ~JP_KEY_BUT3;
			CASE KEY_L_SHIFT:
				joyport[1] |= JP_KEY_BUT2;
			CASE KEY_L_SHIFT | KEYUP:
				joyport[1] &= ~JP_KEY_BUT2;
			CASE KEY_L_ALT:
				joyport[1] |= JP_KEY_BUT3;
			CASE KEY_L_ALT | KEYUP:
				joyport[1] &= ~JP_KEY_BUT3;
			}
		}
	}

	if (doa)
		die(0);
}

void __stdargs
main(int argc, char **argv)
{
	register ULONG	waitbits, sigbits;
	register ULONG	copper_bit, win_bit, audio_bit;
	int		over;

	initialize(argc, argv);

	copper_bit = 1L << copper_signum;
	win_bit = 1L << userport->mp_SigBit;
	init_game_port();
	audio_bit = init_audio();
	sigbits = win_bit | copper_bit | audio_bit | C_SIGS;

	for (;;) {
		waitbits = Wait(sigbits);
#if 0
{
extern ushort	*plane[];
*(ulong *)(plane[199]) = stop_it.l;
*(ulong *)(plane[198]) = 0xaaaaaaaa;
}
#endif
		if (waitbits & copper_bit) {
			if (game_state == GAME_RUN) {
				game_cycle();
				goto other;
			}

#define game_cdctr	copper_data.c_counter	/* shorter name */
#define PING(n)		(&sounds[2 * NUM_SOUNDS_PER_PLAYER + (n)])

			switch (game_state) {
			case GAME_RUN:
				game_cycle();
			CASE GAME_HIT:
				game_cycle();
				if (--game_cnt <= 0) {
					show_scores();
					do_pointer(1, STOP_GAME);
					game_cdctr = AFTER_DELAY;
					game_state = GAME_AFTER;
					/*
					 * May have got a signal
					 * from the copper before
					 * setting game_cdctr to
					 * AFTER_DELAY, so clear it.
					 */
					(void)SetSignal(0, copper_bit);
				}
			CASE GAME_BEFORE:
				clear_scores();
				do_pointer(0, STOP_GAME);
				play_sound(PING(2));
				play_sound(PING(3));
				game_state = GAME_RUN;
			CASE GAME_PING1:
				play_sound(PING(0));
				game_cdctr = PING2_DELAY;
				game_state = GAME_PING2;
			CASE GAME_PING2:
				play_sound(PING(1));
				game_cdctr = BEFORE_DELAY;
				game_state = GAME_BEFORE;
			CASE GAME_AFTER:
				over = 0;
				if (plr_1.score >= MAX_SCORE ||
				    plr_2.score >= MAX_SCORE) {
					if (plr_1.score != plr_2.score) {
						game_cnt = 8;
						game_cdctr = BLINK_DELAY;
						game_state = GAME_OVER;
						break;
					}
					over = 1;
				}
				set_scr(1);	/* next round */
				if (over)
					do_overtime();
				game_cdctr = PING1_DELAY;
				game_state = GAME_PING1;
			CASE GAME_OVER:
			default:
				blink_scores();
				game_cdctr = BLINK_DELAY;
				game_state = GAME_OVER;
				if (--game_cnt <= 0) {
					err("New Game?");
					plr_1.score = 0;
					plr_2.score = 0;
					game_cdctr = 5;
					game_state = GAME_AFTER;
					break;
				}
			CASE GAME_NEW:
				clear_scores(); /* did_set_scr */
				game_cdctr = PING1_DELAY;
				game_state = GAME_PING1;
			}
		}
other:
		if (waitbits & audio_bit)
			process_audio();

		if (waitbits & win_bit)
			process_user();

		if (waitbits & C_SIGS)
			die(0);
	}
}


/*
 * wait1 -- wait for one user event or ^C & friends
 */

void
wait1(void)
{
	Wait((1L << userport->mp_SigBit) | C_SIGS);
	process_user();
}

