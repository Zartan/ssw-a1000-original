/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * ssw.h -- defines and declarations for Solar System Wars
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

#ident "$Id: ssw.h 1.3 1997/01/19 07:24:19 jamesc Exp jamesc $"

#define __USE_SYSBASE	/* jsr through SysBase rather than AbsExecBase */
#undef _USEOLDEXEC_	/* ditto */

#include <exec/types.h>
#include <sys/types.h>
#include <exec/io.h>
#include <graphics/sprite.h>

/**** Stolen from <proto/exec.h> ****/
#ifndef	PROTO_EXEC_H
#define	PROTO_EXEC_H

#include <clib/exec_protos.h>
extern struct ExecBase *SysBase;
#include <pragmas/exec_sysbase_pragmas.h>

/*------ Common support library functions ---------*/
#include <clib/alib_protos.h>
#endif
/**** End <proto/exec.h> ****/

#define CASE		break;case
#define DEFAULT 	break;default
#define NEL(x)		(sizeof(x) / sizeof(*x))
#define ALIGN(adr, bnd)	(((ulong)(adr) + (bnd) - 1) & ~((bnd) - 1))

#define Static		/* static /* CPR doesn't deal with statics well	*/

#ifndef min
#define min(a, b)	((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b)	((a) > (b) ? (a) : (b))
#endif

#define RND(x)		(rand64() % (x))	/* random number from 0 to x-1*/
#define ARND(x)		(rand64() & ((x) - 1))	/* x must be a power of 2*/

#define TO_IOR(p)	((struct IORequest *)(p))
#define TO_MSG(p)	((struct Message *)(p))

#define MDEV_OPEN	1		/* device is open	*/
#define MDEV_SET	2		/* device has been set	*/

/* mon_type types	*/

#define IS_NTSC		0		/* regular or double NTSC	*/
#define IS_PAL		1		/* regular or double PAL	*/
#define IS_VGA		2		/* VGALORESDBL_KEY [320 x 240]	*/

/* screen and sprite defines */

#define DEPTH		2		/* screen bitplane depth	*/
#define WIDTH		312		/* screen width 		*/
#define RWIDTH		320		/* raster width 		*/
#define NTSC_HEIGHT	200		/* NTSC screen height		*/
#define PAL_HEIGHT	256		/* PAL screen height		*/
#define VGA_HEIGHT	240		/* VGA screen height		*/

#define SPRITE_WIDTH	15
#define SPRITE_HEIGHT	15
#define SPRITE_WIDTH_2	(SPRITE_WIDTH / 2)
#define SPRITE_HEIGHT_2	(SPRITE_HEIGHT / 2)
#define MENU_HEIGHT	9

#define BP_ALL_MASK	((1 << DEPTH) - 1) /* all bitplane drawing mask	*/
#define BP_TORP_MASK	(0x1)		/* torp bitplane drawing mask	*/
#define BP_PF2_MASK	(0x2)		/* PF2 bitplane drawing mask	*/

#define REV		34		/* minumum library revision num */

#define GAME_RUN	0		/* game running 		*/
#define GAME_HIT	1		/* one player destroyed 	*/
#define GAME_BEFORE	2		/* before a round		*/
#define GAME_AFTER	3		/* show scores after a round	*/
#define GAME_OVER	4		/* game over			*/
#define GAME_NEW	5		/* new game			*/
#define GAME_PING1	6		/* ping # 1			*/
#define GAME_PING2	7		/* ping # 2			*/

#define HIT_DELAY	300		/* delay after first player hit */
#define PING1_DELAY	60		/* delay until first ping	*/
#define PING2_DELAY	30		/* delay from ping1 to ping2	*/
#define BEFORE_DELAY	30		/* time from ping2 to new round */
#define AFTER_DELAY	144		/* delay after a round		*/
#define BLINK_DELAY	40		/* score blink delay		*/

#define MAX_SCORE	20		/* score to win game		*/

#define C_X		(WIDTH / 2)
#define FITS(x, y)	(x >= 0 && y >= 0 && x < WIDTH && y < y_height)
#define HIDE_X		-32		/* sprite hiding coords */
#define HIDE_Y		-32

#define VEL_LIMIT	(4 * CD_UNIT)	/* .... It's the law!		*/

#define NUM_OBJ 	4		/* maximum number of standard obj */
#define NUM_TORPS	12		/* maximum torps per player */
#define NUM_DEBRIS	8		/* number of explosion debris */

#define THRUST_SHIFT	9
#define TORP_VEL	1		/* shift of p->sine, p->cosine	*/

#define SHIP_IM_HEIGHT	11
#define SHIP_IM_H_2	(SHIP_IM_HEIGHT / 2)
#define SHIP_IM_S	(SHIP_IM_HEIGHT * 2)

/* #define hide_sprite(s) (*((ulong *)((s)->posctldata)) = 0) */
#define hide_sprite(s)	((s)->posctldata[1] = 0, (s)->posctldata[0] = 0)
#define show_sprite(s,v)(*((ulong *)((s)->posctldata)) = v)
#define FIRST_SPR_COLOR	17		/* first sprite color register	*/
#define FIRST_SHIP_COLOR 25		/* first ship sprite color	*/
#define NUM_COLORS	32
#define SPRITE_COLORS	(NUM_COLORS - FIRST_SHIP_COLOR)

				/* reload a ship's image into its sprite */
#define ship_image(p)	spr_image((p)->obj.o_sprptr, &(p)->obj.o_image[SHIP_IM_S * (p)->angle])

#define COLORS		(changed_color = 1, colors)

#define ENGINE_COUNT	13		 /* engine color cycle counter	 */
#define TORP_LIFE	(WIDTH - (SHIP_IM_HEIGHT + 1))
#define SHIELD_FLARE	14		/* shield hit flare timer/index */

#define HYPER_DELAY	50		/* hyperspace states/delays */
#define HYPER_PRE	17		/* Note that HYPER_DELAY is halved */
#define HYPER_JUMP	16		/* in do_player's state machine */
#define HYPER_GLOW	0

#define COLLIDE_MASK	0x7e1e		/* hw collisions we check */

#define NUM_ANGLES	16
#define MAX_ANGLES	(NUM_ANGLES - 1)

#define NUM_A32 	32		/* A32: better angular resolution */
#define MAX_A32 	(NUM_A32 - 1)

#define OTYPE_TORP	0	/* obj types, in ascending invunerability */
#define OTYPE_SHIP	1
#define OTYPE_EXPLOSION	2
#define OTYPE_ANOM	3
#define OTYPE_SEEKER	4
#define OTYPE_DISRUPTOR	5
#define OTYPE_CLUSTER	6
#define OTYPE_GUIDED	7
#define OTYPE_PLASMA	8
#define OTYPE_ASTEROID	9
#define OTYPE_RED_STAR	10
#define OTYPE_YELLOW_STAR 11
#define OTYPE_BLUE_STAR	12
#define OTYPE_SM_HOLE	13
#define OTYPE_MED_HOLE	14
#define OTYPE_LRG_HOLE	15

#define OT_SPRITE(ty)	((ty) >= OTYPE_ANOM && (ty) <= OTYPE_PLASMA)

#define M_RED		4		/* star masses (less is more) */
#define M_YELLOW	3
#define M_BLUE		2
#define M_SM_HOLE	2
#define M_MED_HOLE	1
#define M_LRG_HOLE	0
#define M_NONE		(-1)		/* massless */

	/* extra button options */
#define XO_CLOAK	0		/* cloaking device	*/
#define XO_DISRUPTOR	1		/* disruptor blast	*/
#define XO_ANOM 	2		/* anomalizer blast	*/
#define XO_PLASMA	3		/* plasma bolt		*/
#define XO_QUAD_THRUST	4		/* quad thrust		*/
#define XO_SEEKER	5		/* seeker missile	*/
#define XO_GUIDED	6		/* guided missile	*/
#define XO_CLUSTER	7		/* guided missile	*/
#define NUM_XOPTS	8		/* number of extra options */

	/* stop_it byte index codes -- see do_pointer */
#define STOP_PAUSE	0		/* stop for pause key/menu	*/
#define STOP_WAIT	1		/* like pause, but no menu change */
#define STOP_SCREEN	2		/* hide sprites for screen/setup */
#define STOP_GAME	(-1)		/* like pause, doesn't stop main */
#define STOP_PTR	(-2)		/* intuition pointer state	*/
#define STOP_SPR0	(-3)		/* true if using sprite 0	*/
#define STOP_JOY0	(-4)		/* true if using joyport 0	*/
	/* stop_it word index codes -- see do_pointer */
#define STOP_WJS0	(-2)		/* true if joyport 0 or sprite 0 */

#define GRAV_SIZE	176		/* size of grav look-up table	*/
#define GRAV_RES	3L		/* gravtbl resolution (in bits) */
#define ULGRAV_SIZE	(sizeof(ulong) * GRAV_SIZE)	/* one row of gravtbl*/

	/* signed fixed integer math defines */
#define CD_COORD	(32 - CD_FRAC)	/* size of the coord part	*/
#define CD_FRAC 	22		/* size of the frac part	*/
#define CD_UNIT 	(1 << CD_FRAC)	/* one integer unit		*/

#define uchar		unsigned char
#define ushort		unsigned short
#define uint		unsigned int
#define ulong		unsigned long

typedef union stop_it_u {
	ulong	l;
	ushort	s[sizeof(ulong) / sizeof(ushort)];
	uchar	c[sizeof(ulong)];
} STOP_IT;

typedef struct torp_dat_s {
	ushort			*t_ptr; /* screen memory pointer or NULL*/
	ushort			t_mask; /* memory bit mask		*/
	ushort			t_pad;
} TORP_DAT;

typedef struct spr_dat_s {
	ulong			s_sprd; /* sprite position control data */
	long			s_time; /* collision time stamp 	*/
} SPR_DAT;

typedef union obj_dat_u {
	SPR_DAT 		d_sprt; /* sprite position control data */
	TORP_DAT		d_torp; /* torpedo position data	*/
} OBJ_DAT;

typedef struct object_s {
	struct object_s 	*o_next;	/* link pointer 	*/
	long			o_x, o_y;	/* cur fixed prec coords */
	long			o_vx, o_vy;	/* cur f-p velocity	*/
	long			o_ix, o_iy;	/* cur integer coords	*/
	OBJ_DAT 		o_cur;		/* current object data	*/
	long			o_nx, o_ny;	/* new fixed prec coords */
	long			o_nvx, o_nvy;	/* new f-p velocity	*/
	long			o_nix, o_niy;	/* new integer coords	*/
	OBJ_DAT 		o_new;		/* new object data	*/
	struct SimpleSprite	*o_sprptr;	/* sprite ptr or NULL	*/
	ushort			*o_image;	/* image (if sprite)	*/
	struct player_s 	*o_ship;	/* parent ship (if torp)*/
	long			o_energy;	/* energy counter	*/
	char			o_moveme;	/* movement flag	*/
	uchar			o_offset;	/* sprite center offset */
	uchar			o_type; 	/* object type code	*/
	char			o_mass; 	/* object mass (or -1)	*/
	uchar			o_color;	/* obj's first color reg */
	uchar			o_pad1[3];
} OBJ;

	/* for convenience... */
#define o_sprd		o_cur.d_sprt.s_sprd
#define o_torp		o_cur.d_torp
#define o_nsprd 	o_new.d_sprt.s_sprd
#define o_ntorp 	o_new.d_torp
#define o_tptr		o_torp.t_ptr
#define o_tmask 	o_torp.t_mask
#define o_ntptr 	o_ntorp.t_ptr
#define o_ntmask	o_ntorp.t_mask
#define o_time		o_cur.d_sprt.s_time
#define o_ntime 	o_new.d_sprt.s_time

typedef struct ssw_image_s {
	ushort			*image;
	uchar			offset;
	uchar			size;
	ushort			color[3];
} SSW_IMAGE;

typedef struct iobj_s {
	SSW_IMAGE		*ssw_im;
	short			x, y;
	long			vx, vy;
	ushort			otype;
	short			sprite;
} IOBJ;

typedef struct s_system_s {
	IOBJ			*iobj;
	uint			num_obj;
} S_SYSTEM;

typedef struct ss_list_s {
	S_SYSTEM		*ssys;
	uchar			num_ss;
	uchar			item_num;
	uchar			asteroid;
} SS_LIST;

typedef ushort		joyport_t;

/* for joyport_t convenience... */
#define JP_BACK 	0x001
#define JP_RIGHT	0x002
#define JP_FORW 	0x004
#define JP_LEFT 	0x008
#define JP_FIRE 	0x010
#define JP_BUT3 	0x020
#define JP_KEY_BUT3	0x040
#define JP_BUT2 	0x080
#define JP_KEY_BUT2	0x100
#define JP_DIRS 	(JP_LEFT | JP_FORW | JP_RIGHT | JP_BACK)
#define JP_BUTTONS	(JP_FIRE | JP_BUT2 | JP_KEY_BUT2 | JP_BUT3 | JP_KEY_BUT3)
#define JP_BUT23	(JP_BUT2 | JP_BUT3)
#define JP_KEY_BUT23	(JP_KEY_BUT2 | JP_KEY_BUT3)
#define JP_BK_2 	(JP_BUT2 | JP_KEY_BUT2)
#define JP_BK_3 	(JP_BUT3 | JP_KEY_BUT3)
#define JP_BUT2_SHIFT	7
#define JP_BUT3_SHIFT	5
#define JP_FIRE_SHIFT	4
#define JP_BUT_MASK	0x3
#define KEYP		joyport[2]

#define plr_1		plr[0]		/* player struct defs */
#define plr_2		plr[1]

#define TO_PLR(p)	((PLAYER *)(p))

#define NUM_SOUNDS_PER_PLAYER 9 	/* sound constants		*/
#define NUM_SOUNDS	(2 * NUM_SOUNDS_PER_PLAYER + 4) /* all sounds */
#define fire_snd	p_sounds[0]	/* sound array psuedo-members	*/
#define explode_snd	p_sounds[1]
#define shield_snd	p_sounds[2]
#define torp_die_snd	p_sounds[3]
#define thrust_snd	p_sounds[4]
#define hyper_out_snd	p_sounds[5]
#define hyper_in_snd	p_sounds[6]
#define bounce_snd	p_sounds[7]
#define quad_snd	p_sounds[8]
#define SND_BOUNCE	7		/* bounce sound index		*/

/*
 * sound management structure -- duplicates fields in IOAudio
 */
typedef struct sound_s {
	UBYTE			*data;
	ULONG			length;
	UWORD			unit;
	UWORD			volume;
	UWORD			period;
	UWORD			cycles;
} SOUND;

typedef struct player_s {
	OBJ			obj;			/* ship object	*/
	OBJ			plsk;			/* plasma/seeker */
	OBJ			torps[NUM_TORPS + NUM_DEBRIS];
	OBJ			*free_torps;		/* free torp head */
	ushort			*color_ptr;		/* ship's pallete */
	long			ax, ay; 		/* accelerations */
	long			cos, sin;
	SOUND			*p_sounds[NUM_SOUNDS_PER_PLAYER];
	long			sk_ax, sk_ay;		/* seeker accel */
	long			gd_xacc, gd_yacc;	/* guided accel */
	short			rot;			/* rotation direct */
	uchar			rot_cnt;		/* rotation timer */
	uchar			angle;			/* current angle */
	char			thrust; 		/* thrusting flag */
	uchar			fired;			/* fired flag	*/
	uchar			color;			/* color 1 reg	*/
	uchar			ec_idx; 		/* engine clr anim */
	char			ec_cnt;
	uchar			shield_hit;		/* shield flag/cnt */
	uchar			is_hyper;		/* warp flag/cntr */
	uchar			hyper_recharge; 	/* recharge timer */
	uchar			xtra_timer;		/* xtra timer	*/
	uchar			is_cloaked;		/* is invisible */
	uchar			full_shields;		/* shields up	*/
	uchar			shields_blown;		/* shields toast */
	uchar			shield_timer;		/* shield recharge */
	uchar			sk_guided;		/* is guiding torp */
	uchar			sk_angle;		/* sk's angle	*/
	char			sk_rot; 		/* sk's rotate dir */
	uchar			pad[2];
	/* the following members aren't cleared by init_player */
	uchar			xtra_opt;		/* BUT2 option	*/
	uchar			score;
	joyport_t		*jp;			/* control jp	*/
} PLAYER;

typedef struct sprite_image_s {
	ushort			start[2];
	ushort			image[SPRITE_HEIGHT * 2];
	ushort			end[2];
} SPR_IM;

typedef void		(*PFV)();		/* pointer to void func */

/*
 * copper interrupt server data structure
 *   Note that ssw_copper() must be updated if VB_DATA is changed.
 */
typedef struct copper_data_s {
	STOP_IT 		c_stop_game;	/* holds STOP_GAME	*/
	STOP_IT 		c_stop_it;	/* stop_it's true home	*/
	UWORD			*c_clxdat_ptr;	/* addr of custom.clxdat*/
	long			c_counter;	/* # of vblanks 	*/
	UWORD			c_clxmask;	/* clxdat mask value	*/
	UWORD			c_clxdat;	/* clxdat goes here	*/
	APTR			c_signal_ptr;	/* exec Signal routine	*/
	ulong			c_signal_mask;	/* signal(s) to send	*/
	ulong			*c_task_ptr;	/* my task pointer	*/
	ulong			*c_exec_base;	/* my copy of ExecBase	*/
} COPPER_DATA;

#define stop_it 	copper_data.c_stop_it


extern joyport_t joyport[];

extern USHORT	colors[NUM_COLORS], changed_color;
extern OBJ	obj[], *ohead, *otail;
extern ushort	bound_type;
extern uchar	ship_thrust, torp_vel;
extern ushort	torp_life;
extern uchar	game_state;
extern long	game_cnt;
extern uchar	os_1_3;
extern uchar	os_ver;
extern uchar	mon_type;

extern COPPER_DATA copper_data;
extern long	y_spr_max, y_height, y_mid;

extern PLAYER	plr[];

extern SOUND	sounds[];

extern ushort	ship_image[NUM_ANGLES * SHIP_IM_S];

extern ulong	*gravtbl[];

extern OBJ	*ob_find_xy(long, long);
extern long	mult64(long, long), sine(long), cosine(long);
extern long	cos32(long), sin32(long), atan2(long, long);
extern long	rand64(void);
extern void	srand64(ulong, ulong);
extern ulong	move_sprite(struct SimpleSprite *, long, long);
extern void	spr_image(struct SimpleSprite *, USHORT *);
extern void	ob_unlink(OBJ *);
extern void	rotate_ship(PLAYER *);
extern void __stdargs err(char *, ...);
extern void	collide(void);
extern void	play_sound(SOUND *), flush_sound(SOUND *),
		no_flush_sound(SOUND *);
extern void	die(long);
extern void	delay(long);
extern void	do_pointer(ulong, int);
extern void	update_colors(void);
extern void	set_custom(void);

extern struct IORequest *create_req(char *, long);
extern void	delete_req(struct IORequest *);

extern int	open_dev(char *, long, struct IORequest **, long);
extern void	close_dev(int, struct IORequest *);
