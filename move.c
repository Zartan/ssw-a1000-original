/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * move.c -- solar system wars program move module
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

#ident "$Id: move.c 1.3 1997/01/19 07:23:40 jamesc Exp jamesc $"

#include "ssw.h"
#include <hardware/custom.h>
#include <proto/graphics.h>

#ifndef NULL
#define NULL		(0)
#endif

/* local defines */

#define HYPER_RECHARGE	120		/* hyperdrive recharge time	*/
#define FLIP_RECHARGE	30		/* flip recharge time		*/
#define FULL_RECHARGE	60		/* full shields recharge time	*/
#define FULL_LIMIT	120		/* full shields max. time limit */
#define CLOAK_RECHARGE	60		/* cloaking recharge time	*/

#define ROTATE_COUNT	7		/* ship rotation delay (cycles) */
#define QUAD_COUNT	6		/* quad thrust color cycle delay*/
#define MAX_Q_CLR	(NEL(quad_clr)-1) /* quad thrust color mask	*/

#define EXPLOSION_LIFE	10L		/* ship explosion lifetime	*/

#define TORP_OFF(x)	((x) * 7)	/* launch torps 7 pixels from ship */

#define DEBRIS_LIFE	(TORP_LIFE / 2)	/* debris torps lifetime	*/
#define DEBRIS_OFF_SH	2		/* debris torps offset from ship */
#define DEBRIS_OFF	CD_UNIT 	/* debris torps offset		*/
#define DEBRIS_VEL	(CD_UNIT >> 1)	/* debris random velocity	*/

#define PLSK_RECHARGE	60		/* plasma, seeker, etc. recharge */
#define PLSK_OFF(x)	((x) << 3)	/* launch plasmas 8 from ship	*/

#define ANOM_LIFE	272		/* anomaly blast lifetime	*/
#define ANOM_HEIGHT	11		/* anomaly sprite height	*/
#define ANOM_OFFSET	(ANOM_HEIGHT/2)
#define ANOM_IMAGES	64		/* number of anomaly images	*/

#define CLUSTER_LIFE	148		/* cluster pod lifetime 	*/
#define CLUSTER_HEIGHT	5		/* cluster sprite height	*/
#define CLUSTER_OFFSET	(CLUSTER_HEIGHT/2)

#define PLASMA_LIFE	200		/* plasma bolt lifetime 	*/
#define PLASMA_HEIGHT	SHIP_IM_HEIGHT	/* plasma sprite height 	*/
#define PLASMA_OFFSET	(PLASMA_HEIGHT / 2)

#define SEEKER_LIFE	450		/* seeker missile lifetime	*/
#define SEEKER_THRUST	4		/* seeker acceleration shift val */
#define SEEKER_HEIGHT	5		/* seeker sprite height 	*/
#define SEEKER_OFFSET	(SEEKER_HEIGHT/2)

#define GUIDED_LIFE	500		/* guided missile lifetime	*/
#define GUIDED_THRUST	7		/* guided acceleration shift val */
#define GUIDED_HEIGHT	7		/* guided sprite height 	*/
#define GUIDED_OFFSET	(GUIDED_HEIGHT/2)
#define GUIDED_IMAGE(a) (&guided_image[2 * GUIDED_HEIGHT * (a)])

#define DISRUPTOR_LIFE	83		/* disruptor blast lifetime	*/
#define DISRUPTOR_HEIGHT 7		/* disruptor sprite height	*/
#define DISRUPTOR_OFFSET (DISRUPTOR_HEIGHT/2)
#define DISRUPTOR_SHIFT (2)		/* velocity = "speed of light"	*/

#define BRIGHT_ENG	0xf11		/* bright engine colors 	*/

/* my very own bitmap kludge */
#define XY_WORD(x, y)	(plane[y][(x) >> 4])
#define XY_BIT(x)	(vbit[(x) & 15])
#define CLR_BIT(x, y)	(XY_WORD(x, y) &= ~XY_BIT(x))
#define SET_BIT(x, y)	(XY_WORD(x, y) |= XY_BIT(x))

ushort			*plane[PAL_HEIGHT];

ushort			vbit[] = {
	0x8000, 0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100,
	0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001
};

Static UWORD		hyper_glow[] = {
	0x000, 0x111, 0x222, 0x333, 0x444, 0x555, 0x666, 0x777,
	0x888, 0x999, 0xaaa, 0xbbb, 0xccc, 0xddd, 0xeee, 0xfff
},
			shield_flare[] = {
	0xc5d, 0x555, 0x055, 0x077, 0x099, 0x0bb, 0x0dd, 0x0ff
},
			engine_clr[] = {
#if 1
	0x922, 0xA00, 0xA22, 0xB00, 0xB22, 0xC00, 0xC22, 0xC33,
#else
	0x633, 0x733, 0x833, 0x933, 0xA33, 0xB33, 0xC33, 0xC44,
	0xC44, 0xC33, 0xB33, 0xA33, 0x933, 0x833, 0x733, 0x633,
#endif
},
			quad_clr[] = {
	0xf42, 0xf62, 0xfb2, 0xdf2, 0x8f2, 0x2f2, 0x2f9, 0x2ef,
	0x28f, 0x22f, 0x82f, 0xe2f, 0xf29, 0xf79, 0xfab, 0xf57,
},
			explosion_colors[] = {
	0xf84, 0xffc, 0xf44,
	0xc63, 0xcc9, 0xc33,
	0x942, 0x996, 0x922,
	0x621, 0x663, 0x611,
	0x310, 0x330, 0x300,
	0,     0,     0
};


OBJ			*ohead; 		/* heads of linked list */
OBJ			*otail; 		/* tail of linked list	*/

union was_colors_u {
	ulong	l[ALIGN(sizeof(USHORT) * SPRITE_COLORS, sizeof(ulong)) /
			sizeof(ulong)];
	USHORT	s[SPRITE_COLORS];
} was_colors;

/* keep the following variables long word aligned and padded */
USHORT			changed_color = 0;
ushort			bound_type = 1; 	/* screen boundary type */
ushort			torp_life = TORP_LIFE;	/* lifetime in cycles */
uchar			beginner_mode = 1;
uchar			torp_vel = TORP_VEL;
uchar			ship_thrust = THRUST_SHIFT - 1;
uchar			game_state = 0;
uchar			os_1_3 = 0;
uchar			os_ver = 0;
uchar			mon_type = IS_NTSC;
/*****/

extern ushort		explosion_image[];
extern ushort		plasma_image[], seeker_image[], guided_image[];
extern ushort		cluster_image[], anom_image[], disruptor_image[];
extern ushort		full_shield[];

extern struct Custom __far custom;
extern struct ViewPort	*vp;

extern void		do_joy_ports(void);
extern void		grav_universe(void);
extern void		move_universe(void);
extern void		nova(OBJ *, OBJ *);
extern void		spr_image5(USHORT *dest, USHORT *image);
extern void		spr_image7(USHORT *dest, USHORT *image);
extern void		spr_image9(USHORT *dest, USHORT *image);
extern void		spr_image11(USHORT *dest, USHORT *image);
extern void		spr_image13(USHORT *dest, USHORT *image);


/*
 * rotate_ship -- rotate the ship by p->rot units and re-init rot_cnt
 */

void
rotate_ship(register PLAYER *p)
{
	register int	n;

	p->angle = n = (p->angle + p->rot) & MAX_ANGLES;
	p->ax = (p->cos = cosine(n)) >> ship_thrust;
	p->ay = (p->sin = sine(n)) >> ship_thrust;

	if (p->rot)
		p->rot_cnt = ROTATE_COUNT;

	spr_image11(&p->obj.o_sprptr->posctldata[2],
			&p->obj.o_image[n * SHIP_IM_S]);
}


/*
 * cluster_burst -- burst a cluster bomb pod
 */

Static void
cluster_burst(OBJ *op)
{
	register PLAYER *p;
	register OBJ	*sp;
	register int	n;
	register long	dx, dy;

	/* plsk objects are just after the player object, so.... */
	p = TO_PLR(&op[-1]);

	/* fire off shrapnel torps */

	for (n = NUM_ANGLES; (n -= NUM_ANGLES / NUM_DEBRIS) >= 0; ) {
		if ((sp = p->free_torps) == NULL)
			return; 			/* out of torps?? */
		p->free_torps = sp->o_next;
		sp->o_next = NULL;
		dx = cosine(n);
		dy = sine(n);
		sp->o_vx = sp->o_nvx = p->plsk.o_vx + (dx >> 1) +
		  ARND(DEBRIS_VEL) - (DEBRIS_VEL >> 1);
		sp->o_vy = sp->o_nvy = p->plsk.o_vy + (dy >> 1) +
		  ARND(DEBRIS_VEL) - (DEBRIS_VEL >> 1);
		dx <<= DEBRIS_OFF_SH;
		dx += p->plsk.o_x + ARND(DEBRIS_OFF) - (DEBRIS_OFF >> 1);
		sp->o_x = sp->o_nx = dx;
		sp->o_ix = sp->o_nix = (dx >>= CD_FRAC);
		dy <<= DEBRIS_OFF_SH;
		dy += p->plsk.o_y + ARND(DEBRIS_OFF) - (DEBRIS_OFF >> 1);
		sp->o_y = sp->o_ny = dy;
		sp->o_iy = sp->o_niy = (dy >>= CD_FRAC);
		sp->o_energy = DEBRIS_LIFE / 4 + ARND(64);
		sp->o_moveme = 0;

		otail = (otail->o_next = sp);	/* link at end-o-list */

		if (FITS(dx, dy)) {
			sp->o_tptr = sp->o_ntptr = &XY_WORD(dx, dy);
			sp->o_tmask = sp->o_ntmask = XY_BIT(dx);
		}
	}
}


/*
 * blow_ship -- blow up a ship and replace it with an explosion
 */

Static void
blow_ship(register PLAYER *p)
{
	register OBJ	*dp;
	register int	n;
	register long	dx, dy;

	play_sound(p->explode_snd);

	n = p->color;
	colors[n] = 0xf44;
	colors[++n] = 0xffc;
	changed_color = 1;

	p->obj.o_type = OTYPE_EXPLOSION;
	p->obj.o_energy  = EXPLOSION_LIFE;
	p->obj.o_image = explosion_image;
	spr_image11(&p->obj.o_sprptr->posctldata[2], explosion_image);
	p->color_ptr = explosion_colors;
	p->angle = 0;
	p->thrust = 0;
	p->rot = 0;
	p->is_cloaked = TRUE;		/* seekers don't seek to heaven */
	p->shields_blown = TRUE;	/* DOA */

	/* fire off debris torps */

	dp = &p->torps[NUM_TORPS];
	for (n = NUM_ANGLES; (n -= NUM_ANGLES / NUM_DEBRIS) >= 0; dp++) {
		dx = cosine(n);
		dy = sine(n);
		dp->o_vx = dp->o_nvx = p->obj.o_vx + (dx >> 1) +
		  ARND(DEBRIS_VEL) - (DEBRIS_VEL >> 1);
		dp->o_vy = dp->o_nvy = p->obj.o_vy + (dy >> 1) +
		  ARND(DEBRIS_VEL) - (DEBRIS_VEL >> 1);
		dx <<= DEBRIS_OFF_SH;
		dx += p->obj.o_x + ARND(DEBRIS_OFF) - (DEBRIS_OFF >> 1);
		dp->o_x = dp->o_nx = dx;
		dp->o_ix = dp->o_nix = (dx >>= CD_FRAC);
		dy <<= DEBRIS_OFF_SH;
		dy += p->obj.o_y + ARND(DEBRIS_OFF) - (DEBRIS_OFF >> 1);
		dp->o_y = dp->o_ny = dy;
		dp->o_iy = dp->o_niy = (dy >>= CD_FRAC);
		dp->o_energy = DEBRIS_LIFE + ARND(64);
		dp->o_moveme = 0;

		otail = (otail->o_next = dp);	/* link at end-o-list */

		if (FITS(dx, dy)) {
			dp->o_tptr = dp->o_ntptr = &XY_WORD(dx, dy);
			dp->o_tmask = dp->o_ntmask = XY_BIT(dx);
		}
	}
}


/*
 * ob_unlink -- remove an object from the linked list; if a ship, increment
 *		opponent's score
 */

void
ob_unlink(register OBJ *hit)
{
	register OBJ	*op, *last;

	for (last = NULL, op = ohead; op != hit; last = op, op = op->o_next)
		if (op == NULL)
			return;

	if (op->o_type != OTYPE_SHIP) {
		if (last)			/* unlink */
			last->o_next = op->o_next;
		else
			ohead = op->o_next;
		if (op == otail)
			otail = last;

		op->o_energy = -1;		/* dead */
		op->o_next = NULL;

		if (op->o_sprptr) {		/* hide */
			hide_sprite(op->o_sprptr);
			if (OT_SPRITE(op->o_type)) {
				if (op->o_type == OTYPE_CLUSTER)
					cluster_burst(op);
				no_flush_sound(op->o_ship->torp_die_snd);
			}
			if (op->o_sprptr->num == 0)
				stop_it.c[STOP_SPR0] = FALSE;
		}
		else {
			/* assumes that only torps are spriteless */
			/*
			 * If on-screen, clear pixel.  Check o_moveme to
			 * see if the torp bought the farm in its old
			 * position, or its new one.
			 */
			if (op->o_moveme >= 0) {
				if (last = (OBJ *)op->o_tptr)
					*((ushort *)last) &= ~op->o_tmask;
			}
			else if (last = (OBJ *)op->o_ntptr)
				*((ushort *)last) &= ~op->o_ntmask;
			last = &op->o_ship->obj;
			op->o_next = TO_PLR(last)->free_torps;
			TO_PLR(last)->free_torps = op;
			no_flush_sound(TO_PLR(last)->torp_die_snd);
		}
		op->o_ix = op->o_iy =
		  op->o_nix = op->o_niy = -99;	/* gone */
		return;
	}

	if (&plr_1.obj == op) {			/* score */
		plr_2.score++;
		blow_ship(&plr_1);
	}
	else {
		plr_1.score++;
		blow_ship(&plr_2);
	}

	/*
	 * when first player is hit, start timer for end of round
	 */
	if (game_state == GAME_RUN) {
		game_state = GAME_HIT;
		game_cnt = HIT_DELAY;
	}
}


/*
 * morph_weapon -- transmorgify the player's weapon, hit by an anomalizer
 */

void
morph_weapon(register PLAYER *p)
{
	register long	ix, iy;

	p->sk_ax = p->sk_ay = 0;		/* seeker/guided stuff */
	p->gd_xacc = p->gd_yacc = 0;

	iy = OTYPE_GUIDED;
	do {
		ix = OTYPE_ANOM + RND(OTYPE_PLASMA - OTYPE_ANOM + 1);
	} while (ix == iy);

	if (p->plsk.o_type == OTYPE_DISRUPTOR) {
		p->plsk.o_nvx >>= DISRUPTOR_SHIFT;	/* slow down! */
		p->plsk.o_nvy >>= DISRUPTOR_SHIFT;
	}
	p->plsk.o_type = ix;
	switch (p->plsk.o_type) {
	case OTYPE_PLASMA:
		p->plsk.o_energy = PLASMA_LIFE / 2;
		p->plsk.o_offset = PLASMA_OFFSET;
		p->plsk.o_sprptr->height = PLASMA_HEIGHT;
		p->plsk.o_image = plasma_image;
	CASE OTYPE_ANOM:
		p->plsk.o_energy = ANOM_LIFE / 2 + ARND(256);
		p->plsk.o_offset = ANOM_OFFSET;
		p->plsk.o_sprptr->height = ANOM_HEIGHT;
		p->sk_angle = 0;
		p->sk_rot = (ARND(2) ? 1 : -1);
		p->plsk.o_image = anom_image;
	CASE OTYPE_CLUSTER:
		p->plsk.o_energy = CLUSTER_LIFE / 2 + ARND(64/2);
		p->plsk.o_offset = CLUSTER_OFFSET;
		p->plsk.o_sprptr->height = CLUSTER_HEIGHT;
		p->plsk.o_image = cluster_image;
#if 0
	CASE OTYPE_GUIDED:
		p->plsk.o_energy = GUIDED_LIFE / 2;
		p->plsk.o_offset = GUIDED_OFFSET;
		p->plsk.o_sprptr->height = GUIDED_HEIGHT;
		p->sk_angle = 0;
		p->plsk.o_image = GUIDED_IMAGE(0);
#endif /* 0 */
	CASE OTYPE_SEEKER:
		p->plsk.o_energy = SEEKER_LIFE / 2;
		p->plsk.o_offset = SEEKER_OFFSET;
		p->plsk.o_sprptr->height = 2 * SEEKER_OFFSET + 1;
		p->plsk.o_image = seeker_image;
	CASE OTYPE_DISRUPTOR:
		p->plsk.o_energy = DISRUPTOR_LIFE / 2;
		p->plsk.o_offset = DISRUPTOR_OFFSET;
		p->plsk.o_sprptr->height = 2 * DISRUPTOR_OFFSET + 1;
		p->plsk.o_image = disruptor_image;
	}
	spr_image(p->plsk.o_sprptr, p->plsk.o_image);
	p->plsk.o_ix = (ix = (p->plsk.o_nix += RND(37) - 18));
	p->plsk.o_x = p->plsk.o_nx = (ix << CD_FRAC);
	p->plsk.o_iy = (iy = (p->plsk.o_niy += RND(37) - 18));
	p->plsk.o_y = p->plsk.o_ny = (iy << CD_FRAC);
	ix -= p->plsk.o_offset;
	iy -= p->plsk.o_offset;
	p->plsk.o_sprd = p->plsk.o_nsprd =
	  move_sprite(p->plsk.o_sprptr, ix, iy);
	show_sprite(p->plsk.o_sprptr, p->plsk.o_sprd);
	p->plsk.o_moveme = 1;
	ix = ARND(32);
	p->plsk.o_vx = (p->plsk.o_nvx += cos32(ix) >> 1);
	p->plsk.o_vy = (p->plsk.o_nvy += sin32(ix) >> 1);
	if (p->plsk.o_type == OTYPE_DISRUPTOR) {
		ix = atan2(p->plsk.o_vx, p->plsk.o_vy);
		p->plsk.o_vx = p->plsk.o_nvx = cos32(ix) << DISRUPTOR_SHIFT;
		p->plsk.o_vy = p->plsk.o_nvy = sin32(ix) << DISRUPTOR_SHIFT;
	}
	play_sound(p->bounce_snd);
}


/*
 * fire_plsk -- launch a plasma bolt or seeker missile
 */

Static void
fire_plsk(register PLAYER *p, int otype)
{
	register long	ix, iy, n;
	extern void	wait1(void);

	p->sk_ax = p->sk_ay = 0;		/* seeker/guided stuff */
	p->gd_xacc = p->gd_yacc = 0;
	p->rot = p->rot_cnt = 0;

	switch (p->plsk.o_type = otype) {
	case OTYPE_PLASMA:
		p->plsk.o_energy = PLASMA_LIFE;
		p->plsk.o_offset = PLASMA_OFFSET;
		p->plsk.o_sprptr->height = PLASMA_HEIGHT;
		p->plsk.o_image = plasma_image;
	CASE OTYPE_ANOM:
		p->plsk.o_energy = ANOM_LIFE + ARND(256);
		p->plsk.o_offset = ANOM_OFFSET;
		p->plsk.o_sprptr->height = ANOM_HEIGHT;
		p->sk_angle = 0;
		p->sk_rot = (ARND(2) ? 1 : -1);
		p->plsk.o_image = anom_image;
	CASE OTYPE_CLUSTER:
		p->plsk.o_energy = CLUSTER_LIFE + ARND(64);
		p->plsk.o_offset = CLUSTER_OFFSET;
		p->plsk.o_sprptr->height = CLUSTER_HEIGHT;
		p->plsk.o_image = cluster_image;
	CASE OTYPE_GUIDED:
		p->plsk.o_energy = GUIDED_LIFE;
		p->plsk.o_offset = GUIDED_OFFSET;
		p->plsk.o_sprptr->height = GUIDED_HEIGHT;
		p->sk_angle = n = p->angle;
		p->gd_xacc = cosine(n) >> GUIDED_THRUST;
		p->gd_yacc = sine(n) >> GUIDED_THRUST;
		p->plsk.o_image = GUIDED_IMAGE(n);
	CASE OTYPE_SEEKER:
		p->plsk.o_energy = SEEKER_LIFE;
		p->plsk.o_offset = SEEKER_OFFSET;
		p->plsk.o_sprptr->height = 2 * SEEKER_OFFSET + 1;
		p->plsk.o_image = seeker_image;
	CASE OTYPE_DISRUPTOR:
		p->plsk.o_energy = DISRUPTOR_LIFE;
		p->plsk.o_offset = DISRUPTOR_OFFSET;
		p->plsk.o_sprptr->height = 2 * DISRUPTOR_OFFSET + 1;
		p->plsk.o_image = disruptor_image;
	DEFAULT:
		err("fire_plsk: otype=%ld", otype);
		wait1();
		die(1);
		return;
	}
	spr_image(p->plsk.o_sprptr, p->plsk.o_image);

	play_sound(p->fire_snd);

	ix = p->obj.o_nx + PLSK_OFF(p->cos);
	p->plsk.o_x = p->plsk.o_nx = ix;
	p->plsk.o_ix = p->plsk.o_nix = (ix >>= CD_FRAC);
	iy = p->obj.o_ny + PLSK_OFF(p->sin);
	p->plsk.o_y = p->plsk.o_ny = iy;
	p->plsk.o_iy = p->plsk.o_niy = (iy >>= CD_FRAC);
	if (p->plsk.o_type != OTYPE_DISRUPTOR) {
		p->plsk.o_vx = p->plsk.o_nvx = p->obj.o_nvx + (p->cos >> 1);
		p->plsk.o_vy = p->plsk.o_nvy = p->obj.o_nvy + (p->sin >> 1);
	}
	else {
		p->plsk.o_vx = p->plsk.o_nvx = p->cos << DISRUPTOR_SHIFT;
		p->plsk.o_vy = p->plsk.o_nvy = p->sin << DISRUPTOR_SHIFT;
	}
	p->plsk.o_moveme = 1;
	p->xtra_timer = PLSK_RECHARGE;

	ix -= p->plsk.o_offset;
	iy -= p->plsk.o_offset;
	p->plsk.o_sprd = p->plsk.o_nsprd =
	  move_sprite(p->plsk.o_sprptr, ix, iy);
	show_sprite(p->plsk.o_sprptr, p->plsk.o_sprd);

	p->plsk.o_next = p->obj.o_next; 	/* link after ship */
	p->obj.o_next = &p->plsk;
	if (otail == &p->obj)
		otail = &p->plsk;

	p->ec_cnt = 8;				/* engines cycle later */
	COLORS[p->color] = 0xff8;		/* engines flare */
}


/*
 * fire_torp -- if available, launch a photon torpedo
 *		return non-zero on success, else zero
 */

Static int
fire_torp(register PLAYER *p)
{
	register OBJ	*torp;
	register long	ix, iy;

	if ((torp = p->free_torps) == NULL)
		return (0);			/* out of torps */
	p->free_torps = torp->o_next;

	torp->o_x = torp->o_nx = (ix = p->obj.o_nx + TORP_OFF(p->cos));
	torp->o_ix = torp->o_nix = (ix >>= CD_FRAC);
	torp->o_y = torp->o_ny = (iy = p->obj.o_ny + TORP_OFF(p->sin));
	torp->o_iy = torp->o_niy = (iy >>= CD_FRAC);
	torp->o_vx = torp->o_nvx = p->obj.o_nvx + (p->cos >> torp_vel);
	torp->o_vy = torp->o_nvy = p->obj.o_nvy + (p->sin >> torp_vel);
	torp->o_energy = torp_life;
	torp->o_tptr = torp->o_ntptr = NULL;
	torp->o_next = NULL;
	torp->o_moveme = 0;
	play_sound(p->fire_snd);

	otail = (otail->o_next = torp);		/* link at end-o-list */

	if (FITS(ix, iy)) {
		torp->o_tptr = torp->o_ntptr = &XY_WORD(ix, iy);
		torp->o_tmask = torp->o_ntmask = XY_BIT(ix);
	}

	return (1);
}


extern long	mult16(long, long);
extern long	sqrt16(long);

/*
 * seeker -- aim player's seeker at target
 */

Static void
seeker(PLAYER *p, PLAYER *target)
{
	register long	dx, dy, c;

#define W_WIDTH (WIDTH + 2 * SPRITE_WIDTH_2)
#define WW_2	(W_WIDTH / 2)
#define WH_2	((y_spr_max >> 1) + SPRITE_HEIGHT_2)

	dx = target->obj.o_nix - p->plsk.o_nix;
	dy = target->obj.o_niy - p->plsk.o_niy;
	if (bound_type) {			/* wrap dx and dy */
		if (dx > WW_2)
			dx = W_WIDTH - dx;
		else if (dx < -WW_2)
			dx = W_WIDTH + dx;
		c = WH_2;
		if (dy > c)
			dy = c * 2 - dy;
		else if (dy < -c)
			dy = c * 2 + dy;
	}
	c = mult16(dx, dx);
	c += mult16(dy, dy);			/* c is dist^2 */
#if 1
	c >>= (c >> 14);
#else
	if (c > 65535)
		c = 256;
	else
#endif
	c = sqrt16(c);

	dx += mult16(c, target->obj.o_nvx - p->plsk.o_nvx >> 10) >> 12;
	dy += mult16(c, target->obj.o_nvy - p->plsk.o_nvy >> 10) >> 12;
	c = atan2(dx, dy);
	p->sk_ax = cos32(c) >> SEEKER_THRUST;
	p->sk_ay = sin32(c) >> SEEKER_THRUST;
}


/*
 * do_player -- move plasma/seekers, do joyport logic, rotate ships,
 *		animate engines, etc.
 */

void
do_player(register PLAYER *p)
{
	register long	dx, dy;
	register long	c;
	register ushort *clrp;

	if (p->plsk.o_energy > 0) {
		switch (p->plsk.o_type) {
		case OTYPE_ANOM:
			if (p->plsk.o_energy & 3)
				break;
			p->sk_angle += p->sk_rot;
			p->sk_angle &= ANOM_IMAGES - 1;
			spr_image11(&p->plsk.o_sprptr->posctldata[2],
			    &anom_image[p->sk_angle * 2 * ANOM_HEIGHT]);
		CASE OTYPE_PLASMA:
			if (p->plsk.o_energy & 1) {
				spr_image11(&p->plsk.o_sprptr->posctldata[2],
						p->plsk.o_image);
				p->plsk.o_image += SHIP_IM_S;
			}

		CASE OTYPE_SEEKER: {
			PLAYER	*target;

			if ((p->plsk.o_energy & 7) ||
			    (target = p->plsk.o_ship)->is_cloaked) {
				p->plsk.o_nvx += p->sk_ax;	/* accel */
				p->plsk.o_nvy += p->sk_ay;
				break;
			}
			seeker(p, target);
		 }	/* fall through!  (to accelerate) */

		case OTYPE_GUIDED:
			p->plsk.o_nvx += p->sk_ax;	/* accelerate */
			p->plsk.o_nvy += p->sk_ay;
			break;
		}
	}
	if (p->obj.o_energy <= 0)
		return; 			/* dead and gone */

	if (p->obj.o_type != OTYPE_SHIP) {	/* exploded */
		if ((dx = p->obj.o_energy) >= EXPLOSION_LIFE)
			return; 		/* was done in blow_ship */
		if (dx & 1)
			return; 		/* every other game cycle */

		p->obj.o_image += SHIP_IM_S;
		spr_image11(&p->obj.o_sprptr->posctldata[2], p->obj.o_image);
		c = p->color;
		clrp = p->color_ptr;
		for (dy = 3; --dy >= 0; c++) {
			colors[c] = *clrp++;
		}
		changed_color = 1;
		p->color_ptr = --clrp;
		return;
	}

	/* do hyperspace stuff */

	if (p->is_hyper) {
		if ((dx = --p->is_hyper) & 1) {
			if (dx >= (HYPER_PRE + 12) * 2) {
				goto dis_thrust;
			}
			return; 		/* every other game cycle */
		}
		switch (dx >>= 1) {
		/* the following N cases are for disruptors */
		CASE HYPER_PRE+12: case HYPER_PRE+13: case HYPER_PRE+14:
		case HYPER_PRE+15: case HYPER_PRE+16: case HYPER_PRE+17:
		case HYPER_PRE+18: case HYPER_PRE+19: case HYPER_PRE+20:
		case HYPER_PRE+21: case HYPER_PRE+22: case HYPER_PRE+23:
		case HYPER_PRE+24: case HYPER_PRE+25: case HYPER_PRE+26:
		case HYPER_PRE+27: case HYPER_PRE+28: case HYPER_PRE+29:
			c = p->color;
			dy = (p->ec_idx = (p->ec_idx - 1) & MAX_Q_CLR);
			colors[c] = colors[c + 1] = quad_clr[dy];
			changed_color = 1;
			rotate_ship(p);
			if (dx & 1)
				play_sound((dx & 2) ? p->hyper_out_snd :
						p->hyper_in_snd);
			p->thrust = 0;
dis_thrust:
			dx = ship_thrust - 2;		/* disrupted thrust*/
			p->obj.o_nvx += p->cos >> dx;
			p->obj.o_nvy += p->sin >> dx;
			return;
		case HYPER_PRE+11:			/* end paralysis */
			/* fall through! */
		/* the following three cases are for the anomalizer */
		case HYPER_PRE+8:
			p->is_hyper = HYPER_GLOW + 2;	/* end flash */
			/* fall through! */
		case HYPER_PRE+9: case HYPER_PRE+10:
			return; 			/* do nothing */
		/* back to hyperspace stuff */
		case HYPER_PRE:   case HYPER_PRE+1: case HYPER_PRE+2:
		case HYPER_PRE+3: case HYPER_PRE+4: case HYPER_PRE+5:
		case HYPER_PRE+6: case HYPER_PRE+7:
			dx = hyper_glow[((HYPER_DELAY/2 - 1 - dx) << 1) + 1];
			c = p->color;
			clrp = p->color_ptr;
			for (dy = 3; --dy >= 0; c++) {
				colors[c] = *clrp++ | dx;
			}
			changed_color = 1;
		CASE HYPER_JUMP:
			dx = RND(WIDTH - SHIP_IM_HEIGHT) + SHIP_IM_H_2;
			dy = RND(y_height - SHIP_IM_HEIGHT) + SHIP_IM_H_2;
			p->obj.o_nix = dx;
			p->obj.o_niy = dy;
			p->obj.o_nx = dx << CD_FRAC;
			p->obj.o_ny = dy << CD_FRAC;
			c = p->obj.o_offset;
			p->obj.o_nsprd = move_sprite(p->obj.o_sprptr,
						dx - c, dy - c);
			play_sound(p->hyper_in_snd);
		CASE HYPER_GLOW:    case HYPER_GLOW+ 1: case HYPER_GLOW+ 2:
		case HYPER_GLOW+ 3: case HYPER_GLOW+ 4: case HYPER_GLOW+ 5:
		case HYPER_GLOW+ 6: case HYPER_GLOW+ 7: case HYPER_GLOW+ 8:
		case HYPER_GLOW+ 9: case HYPER_GLOW+10: case HYPER_GLOW+11:
		case HYPER_GLOW+12: case HYPER_GLOW+13: case HYPER_GLOW+14:
			p->shield_hit = 0;
			/* fall through! */
		case HYPER_GLOW+15:
			dx = hyper_glow[dx - HYPER_GLOW];
			c = p->color;
			clrp = p->color_ptr;
			for (dy = 3; --dy >= 0; c++) {
				colors[c] = *clrp++ | dx;
			}
			changed_color = 1;
			p->ec_cnt = 4;
		}
		return;
	}
	if (p->hyper_recharge)
		p->hyper_recharge--;
	if (p->xtra_timer)
		p->xtra_timer--;
	if (p->shield_timer)
		p->shield_timer--;

	/* do shield flare when hit */

	if (p->shield_hit > 0 && !((dx = p->shield_hit--) & 1))
		COLORS[p->color + 1] = shield_flare[dx >> 1];


	/* rotate ship */

	if (p->rot_cnt && --p->rot_cnt == 0) {
		if (p->sk_guided) {
			dx = (p->sk_angle + p->rot) & MAX_ANGLES;
			p->sk_angle = dx;
			p->plsk.o_image = GUIDED_IMAGE(dx);
			spr_image7(&p->plsk.o_sprptr->posctldata[2],
					p->plsk.o_image);
			p->gd_xacc = cosine(dx) >> GUIDED_THRUST;
			p->gd_yacc = sine(dx) >> GUIDED_THRUST;
			if (p->rot)
				p->rot_cnt = ROTATE_COUNT;
		}
		else {
			rotate_ship(p);
		}
	}
	/* do joystick logic */

	c = *p->jp;
	dx = ((c & JP_LEFT) ? -1 : ((c & JP_RIGHT) ? 1 : 0));
	dy = ((c & JP_BACK) ? -1 : ((c & JP_FORW) ? 1 : 0));

	if (dx) {
		p->rot = dx;
		if (p->rot_cnt == 0)
			p->rot_cnt = ROTATE_COUNT;
	}
	else
		p->rot_cnt = 0;

	if ((c & JP_BK_3) && p->shield_timer == 0) {
		if (p->full_shields) {
			if (--(p->full_shields) == 0)
				goto shields_down;
		}
		else if (!p->shields_blown) {
			p->full_shields = FULL_LIMIT;
			spr_image11(&p->obj.o_sprptr->posctldata[2],
					full_shield);
			play_sound(p->hyper_out_snd);
		}
		p->rot = p->rot_cnt = p->thrust = 0;
		if (p->is_cloaked)
			goto decloak;
		return; 			/* full shields */
	}
	if (c & JP_BK_2) {
		if (p->xtra_timer == 0) {
			switch (p->xtra_opt) {
			case XO_ANOM:
				if (p->plsk.o_energy < 0) {
					fire_plsk(p, OTYPE_ANOM);
					return; 	/* did eng color */
				}
			CASE XO_DISRUPTOR:
				if (p->plsk.o_energy < 0) {
					fire_plsk(p, OTYPE_DISRUPTOR);
					return; 	/* did eng color */
				}
			CASE XO_CLOAK:
				if (!p->is_cloaked) {
					p->is_cloaked = 1;
					changed_color = 1;
					play_sound(p->hyper_out_snd);
					clrp = &colors[p->color];
					*clrp++ = 0;
					*clrp++ = 0;
					*clrp = 0;
					p->rot_cnt = p->rot = p->thrust = 0;
					return;
				}
				if (!dx && !p->shield_hit) {
					clrp = &colors[p->color];
					for (dx = 3; --dx >= 0; clrp++) {
						if (*clrp) {
							*clrp = 0;
							changed_color = 1;
						}
					}
					dx = 0;
				}			/* cloaked */
				if (dy < 0)
					return;
			CASE XO_PLASMA:
				if (p->plsk.o_energy < 0) {
					fire_plsk(p, OTYPE_PLASMA);
					return; 	/* did eng color */
				}
			CASE XO_QUAD_THRUST:
				if (p->thrust >= 0) {
					p->thrust = -1;
					p->ec_cnt = 0;
					play_sound(p->quad_snd);
				}
				dx = ship_thrust - 2;
				p->obj.o_nvx += p->cos >> dx;
				p->obj.o_nvy += p->sin >> dx;
				if (--(p->ec_cnt) > 0)
					return;
				p->ec_cnt = QUAD_COUNT;
				c = p->ec_idx = (p->ec_idx + 1) & MAX_Q_CLR;
				COLORS[p->color] = quad_clr[c];
				return;
			case XO_SEEKER:
				if (p->plsk.o_energy < 0) {
					fire_plsk(p, OTYPE_SEEKER);
					return; 	/* did eng color */
				}
			CASE XO_GUIDED:
				p->sk_guided = TRUE;
				if (p->plsk.o_energy < 0) {
					fire_plsk(p, OTYPE_GUIDED);
					return; 	/* did eng color */
				}
			CASE XO_CLUSTER:
				if (p->plsk.o_energy < 0) {
					fire_plsk(p, OTYPE_CLUSTER);
					return; 	/* did eng color */
				}
			}
		}
	}
	else if (p->is_cloaked) {
decloak:
		p->is_cloaked = 0;
		c = p->color;
		clrp = p->color_ptr + 1;
		colors[c++] = BRIGHT_ENG;
		for (dx = 2; --dx >= 0; c++) {
			colors[c] = *clrp++;
		}
		changed_color = 1;
		p->ec_cnt = 4;
		play_sound(p->hyper_in_snd);
		p->rot = p->rot_cnt = p->thrust = 0;
		p->xtra_timer = CLOAK_RECHARGE;
		return;
	}
	else {
		p->sk_guided = FALSE;
		p->sk_ax = p->sk_ay = 0;
	}

	if (p->full_shields) {
shields_down:
		p->full_shields = 0;
		p->shield_timer = FULL_RECHARGE;
		ship_image(p);
		play_sound(p->hyper_in_snd);
		p->ec_cnt = 4;
		COLORS[p->color] = BRIGHT_ENG;		/* engines glow */
		return;
	}

	if (dy < 0) {
		if (dx == 0 && p->hyper_recharge == 0 && !p->is_cloaked) {
			p->is_hyper = HYPER_DELAY;	/* warp */
			p->hyper_recharge = HYPER_RECHARGE;
			COLORS[p->color] = 0xf44;	/* engines glow */
			play_sound(p->hyper_out_snd);
			p->rot = p->rot_cnt = p->thrust = 0;
			return; 			/* did eng color */
		}
	}
	else {
		if (p->sk_guided) {
			if (dy) {
				p->sk_ax = p->gd_xacc;
				p->sk_ay = p->gd_yacc;
				COLORS[p->color] = BRIGHT_ENG;
				if (p->thrust <= 0)
					play_sound(p->thrust_snd);
			}
			else {
				p->sk_ax = p->sk_ay = 0;
				COLORS[p->color] = BRIGHT_ENG & 0x7ff;
				if (p->thrust)
					flush_sound(p->thrust_snd);
			}
			p->thrust = dy;
			return; 			/* did eng color */
		}
		if (dy) {
			if (p->thrust <= 0)
				play_sound(p->thrust_snd);
			p->obj.o_nvx += p->ax;
			p->obj.o_nvy += p->ay;
		}
		else {
			if (p->thrust)
				flush_sound(p->thrust_snd);
		}
		p->thrust = dy;
		if (p->is_cloaked) {
			if (dy || dx)
				goto eng_clr;	/* can't fire when cloaked */
			return; 			/* cloaked */
		}
	}

	if ((c & JP_FIRE) && !p->is_cloaked) {
		if (!p->fired && (p->fired = fire_torp(p))) {
			p->ec_cnt = 4;		/* engines cycle after next*/
			COLORS[p->color] = 0xddd;/* engines flare */
			return; 		/* did eng color */
		}
	}
	else
		p->fired = 0;

	/* do engine color animation */
eng_clr:
#if 1
	if (p->thrust) {
		c = 0x311;			/* thrust brightness boost */
		p->ec_cnt -= 3;
	}
	else
		c = 0;				/* idle brightness */
	if (--p->ec_cnt > 0)
		return;
#else
	if (p->thrust)
		c = 0x311;			/* thrust brightness boost */
	else if (--p->ec_cnt > 0)
		return;
	else
		c = 0;				/* idle brightness */
#endif

#define MAX_E_CLR	(NEL(engine_clr) - 1)

	p->ec_cnt = ENGINE_COUNT;
	c += engine_clr[p->ec_idx = (p->ec_idx + 1) & MAX_E_CLR];
	COLORS[p->color] = c;
}


/*
 * game_cycle -- run the universe through one cycle
 */

void
game_cycle(void)
{
	if (copper_data.c_clxdat)
		collide();		/* process collisions */

	if (copper_data.c_clxdat)
		move_universe();

	/*
	 * do per-game-cycle stuff
	 */
	do_joy_ports();
	do_player(&plr_1);
	do_player(&plr_2);
	if (changed_color)
		update_colors();

	grav_universe();

	copper_data.c_counter = 1;	/* ready for next screen refresh */
}


/*
 * single_step -- do a full calculate-and-move game cycle when paused
 */

void
single_step(void)
{
	register UWORD	clxdat;

	clxdat = custom.clxdat & COLLIDE_MASK;
	copper_data.c_clxdat = clxdat;
	game_cycle();
	copper_data.c_counter = -1;
	if (clxdat == 0)
		move_universe();	/* else game_cycle did it */
}

