/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * collide.c -- various collision-related routines for SSW
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

#ident "$Id: collide.c 1.2 1997/01/19 07:04:58 jamesc Exp jamesc $"

#include "ssw.h"
#include <graphics/rastport.h>
#include <graphics/gfxmacros.h>
#include <proto/graphics.h>

#ifndef NULL
#define NULL		(0)
#endif

#define BOUNCE_TIME	3	/* don't bounce for at least N cycles */

#define BOUNCER(op)	((op)->o_ntime - (op)->o_energy > BOUNCE_TIME)

#define SHIELD_CK(x, y) (shield_xy[x] & SBT(y))

#define XY(x, y)	((((x) & 0xf) << 4) + ((y) & 0xf))
#define SBT(bit)	(1 << (bit))

typedef struct x_y_s {		/* 2 pixel thick shield XY coords */
	uchar	outer, inner;
} X_Y;

Static X_Y const	shield_coords[] = {
  { 0xa5, 0x95 }, { 0xa6, 0x96 }, { 0xa7, 0x97 }, { 0x98, 0x87 },
  { 0x99, 0x88 }, { 0x89, 0x78 }, { 0x7a, 0x79 }, { 0x6a, 0x69 },
  { 0x5a, 0x59 }, { 0x4a, 0x49 }, { 0x3a, 0x39 }, { 0x29, 0x38 },
  { 0x19, 0x28 }, { 0x18, 0x27 }, { 0x07, 0x17 }, { 0x06, 0x16 },
  { 0x05, 0x15 }, { 0x04, 0x14 }, { 0x03, 0x13 }, { 0x12, 0x23 },
  { 0x11, 0x22 }, { 0x21, 0x32 }, { 0x30, 0x31 }, { 0x40, 0x41 },
  { 0x50, 0x51 }, { 0x60, 0x61 }, { 0x70, 0x71 }, { 0x81, 0x72 },
  { 0x91, 0x82 }, { 0x92, 0x83 }, { 0xa3, 0x93 }, { 0xa4, 0xa3 },
};
/*
 * shield_xy -- bit map of shield coordinates:	shield_xy[x] & SBT(y)
 */
Static ushort const	shield_xy[] = {
/*0*/SBT(0x3) | SBT(0x4) | SBT(0x5) | SBT(0x6) | SBT(0x7),
/*1*/SBT(0x1) | SBT(0x2) | SBT(0x3) | SBT(0x4) | SBT(0x5) | SBT(0x6) |
		SBT(0x7) | SBT(0x8) | SBT(0x9),
/*2*/SBT(0x1) | SBT(0x2) | SBT(0x3) | SBT(0x7) | SBT(0x8) | SBT(0x9),
/*3*/SBT(0x0) | SBT(0x1) | SBT(0x2) | SBT(0x8) | SBT(0x9) | SBT(0xa),
/*4*/SBT(0x0) | SBT(0x1) | SBT(0x9) | SBT(0xa),
/*5*/SBT(0x0) | SBT(0x1) | SBT(0x9) | SBT(0xa),
/*6*/SBT(0x0) | SBT(0x1) | SBT(0x9) | SBT(0xa),
/*7*/SBT(0x0) | SBT(0x1) | SBT(0x2) | SBT(0x8) | SBT(0x9) | SBT(0xa),
/*8*/SBT(0x1) | SBT(0x2) | SBT(0x3) | SBT(0x7) | SBT(0x8) | SBT(0x9),
/*9*/SBT(0x1) | SBT(0x2) | SBT(0x3) | SBT(0x4) | SBT(0x5) | SBT(0x6) |
		SBT(0x7) | SBT(0x8) | SBT(0x9),
/*A*/SBT(0x3) | SBT(0x4) | SBT(0x5) | SBT(0x6) | SBT(0x7),
/*B*/0,
/*C*/0,
/*D*/0,
/*E*/0,
/*F*/0
};
Static uchar const	atan_a32[] = {
/*00*/ 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
/*10*/ 0, 4, 6, 6, 7, 7, 7, 7, 7, 7, 7, 8, 8, 8, 8, 8,
/*20*/ 0, 2, 4, 5, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7, 7, 7,
/*30*/ 0, 2, 3, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7,
/*40*/ 0, 1, 2, 3, 4, 5, 5, 5, 6, 6, 6, 6, 6, 6, 7, 7,
/*50*/ 0, 1, 2, 3, 3, 4, 4, 5, 5, 5, 6, 6, 6, 6, 6, 6,
/*60*/ 0, 1, 2, 2, 3, 4, 4, 4, 5, 5, 5, 5, 6, 6, 6, 6,
/*70*/ 0, 1, 1, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 6, 6,
/*80*/ 0, 1, 1, 2, 2, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5, 6,
/*90*/ 0, 1, 1, 2, 2, 3, 3, 3, 4, 4, 4, 5, 5, 5, 5, 5,
/*A0*/ 0, 1, 1, 1, 2, 2, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5,
/*B0*/ 0, 0, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5,
/*C0*/ 0, 0, 1, 1, 2, 2, 2, 3, 3, 3, 4, 4, 4, 4, 4, 5,
/*D0*/ 0, 0, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4,
/*E0*/ 0, 0, 1, 1, 1, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4,
/*F0*/ 0, 0, 1, 1, 1, 2, 2, 2, 2, 3, 3, 3, 3, 4, 4, 4,
};
Static ulong const	shield_mask[SHIP_IM_HEIGHT] = {
 0x00000000, /*00000000000*/
 0x00000000, /*00000000000*/
 0x0E000E00, /*00003330000*/
 0x1F001F00, /*00033333000*/
 0x3F803F80, /*00333333300*/
 0x3F803F80, /*00333333300*/
 0x3F803F80, /*00333333300*/
 0x1F001F00, /*00033333000*/
 0x0E000E00, /*00003330000*/
 0x00000000, /*00000000000*/
 0x00000000, /*00000000000*/
};

extern ushort		*plane[], vbit[];

extern struct ViewPort	*vp;
extern struct RastPort	*rp;

extern uint		and_sprites(ulong *, ulong *, long, long);
extern void		do_player(PLAYER *);
extern void		morph_weapon(PLAYER *);
extern void		grav1(OBJ *);


/*
 * ob_find_spr -- find object by sprite number, returns NULL if not found
 */

Static OBJ *
ob_find_spr(int n)
{
	register OBJ	*op;
	register UWORD	num = n;

	for (op = ohead; op; op = op->o_next)
		if (op->o_sprptr && op->o_sprptr->num == num)
			return (op);

	return (NULL);
}


/*
 * ob_find_xy -- find object by coordinates, returns NULL if not found
 *		Note that the only thing we find by coords are torpedos
 */

OBJ *
ob_find_xy(register long x, register long y)
{
	register OBJ	*op;

	for (op = ohead; op; op = op->o_next) {
		if (op->o_moveme >= 0) {
			if (op->o_ix != x || op->o_iy != y)
				continue;
		}
		else {
			if (op->o_nix != x || op->o_niy != y)
				continue;
		}
		if (op->o_energy > 0 && op->o_type == OTYPE_TORP)
			return (op);
	}

	return (NULL);
}


/*
 * blow_shields -- remove the shields from a ship's sprite and images
 */

Static void
blow_shields(OBJ *op)
{
	register PLAYER *p;
	extern void	rem_shields(ulong *image, ulong const *mask,
					int num_imgs, int mask_len);

	p = TO_PLR(op);
	if (p->shields_blown)
		return; 				/* don't bother */
	play_sound(p->bounce_snd);
	p->shields_blown = TRUE;
	p->full_shields = 0;
	rem_shields((ulong *)(p->obj.o_image), shield_mask,
			NUM_ANGLES, NEL(shield_mask));
	ship_image(p);
}


/*
 * nova -- colliding stars!!!
 */

Static void
nova(OBJ *sun, OBJ *destructor)
{
	register int	i, c;
	register OBJ	*op, *next;

	colors[plr_1.color] = 0xf00;
	colors[plr_2.color] = 0xf00;
	changed_color = 1;
	update_colors();

	for (i = 0x10; --i >= 0; ) {			/* zot! */
		c = 0xf - 1;
		SetRGB4(vp, 0, c, c, c);
		SetRGB4(vp, 1, i, i, i);
		delay(1);
	}

	blow_shields(&plr_1.obj);
	blow_shields(&plr_2.obj);
	delay(4);

	if (op = destructor) {
		next = sun;
		if ((i = next->o_mass - op->o_mass) < 0) {
			sun = op;			/* use bigger sun */
			op = next;
			next = sun;
		}
		if (i == 0 && --(op->o_mass) < 0)	/* absorb sun */
			op->o_mass = 0;
		op->o_vx += next->o_vx >> i;		/* do momemtum */
		op->o_vy += next->o_vy >> i;
	}

	ob_unlink(sun);

	/* snuff torpedos first, in case cluster pods are out there */

	for (op = ohead; op; op = next) {
		next = op->o_next;
		if (op->o_type == OTYPE_TORP)
			ob_unlink(op);
	}

	for (op = ohead; op; op = next) {
		next = op->o_next;
		if (op->o_type <= OTYPE_ASTEROID)
			ob_unlink(op);
	}

	for (i = 16; --i >= 0; ) {
		/* do player updates here to move explosions along */
		do_player(&plr_1);
		do_player(&plr_2);
		delay(1);
	}

	for (i = 0x10; --i >= 0; ) {			/* anti-zot! */
		SetRGB4(vp, 0, i, i, i);
		delay(1);
	}
	colors[1] = 0xddd;
	LoadRGB4(vp, colors, NEL(colors));
}


/*
 * erode_shield -- erode the ship's shield in all of its images
 */

Static void
erode_shield(OBJ *op, register uint a32)
{
	register int	n;
	register uint	xy;
	register ushort *wptr;
	register X_Y const *xyp;
	register PLAYER *p;

	p = (PLAYER *)op;
	play_sound(p->shield_snd);

	p->shield_hit = SHIELD_FLARE;			/* hit!! */
	wptr = p->obj.o_image + 1;			/* color 2 */
	a32 = (a32 - (NUM_A32 / NUM_ANGLES) * p->angle) & MAX_A32;
	xyp = &shield_coords[a32];

	for (n = NUM_ANGLES; --n >= 0; wptr += SHIP_IM_S) {
		xy = xyp->outer;
		wptr[(xy & 0xf) * 2] &= ~vbit[xy >> 4];
		xy = xyp->inner;
		wptr[(xy & 0xf) * 2] &= ~vbit[xy >> 4];
		xyp += NUM_A32 / NUM_ANGLES;
		a32 += NUM_A32 / NUM_ANGLES;
		if (a32 >= NUM_A32) {
			a32 -= NUM_A32;
			xyp -= NUM_A32;
		}
	}

	ship_image(p);
}

/*
 * atan2 -- fake an atan2, returns A32 angles
 */

long
atan2(register long x, register long y)
{
	register int	neg;

	neg = 0;
	if (x < 0) {
		x = -x;
		neg = 1;
	}
	if (y < 0) {
		y = -y;
		neg += 2;
	}
	while ((x | y) > 0xf) {
		x >>= 1;
		y >>= 1;
	}

	x = atan_a32[XY(x, y)];

	switch (neg) {
	case 0:
		;			/* do nothing */
	CASE 1:
		x = NUM_A32 / 2 - x;
	CASE 2:
		x = NUM_A32 - x;
	CASE 3:
		x += NUM_A32 / 2;
	}

	return (x);
}


/*
 * morph_star -- transform a star into a black hole after an anomalizer hit!
 */

Static void
morph_star(register OBJ *star)
{
	extern ushort	black_hole[];

	star->o_sprptr->height = 1;
	star->o_image = black_hole;
	spr_image(star->o_sprptr, star->o_image);
	star->o_sprd = star->o_nsprd =
			move_sprite(star->o_sprptr, star->o_ix, star->o_iy);
	SetRGB4(vp, star->o_color, 0xf, 0x9, 0xf);
	star->o_offset = 0;
	star->o_mass = RND(M_RED - M_LRG_HOLE + 1) + M_LRG_HOLE;
	star->o_moveme = TRUE;
	colors[star->o_color] = colors[star->o_color + 1] =
	  colors[star->o_color + 2] = 0xf9f;	/* glowing purple-white */
	changed_color = TRUE;
}


/*
 * anomalize -- hit by an anomaly blast!
 *
 *	Returns zero for an ordinary hit, else non-zero.
 */

Static int
anomalize(OBJ *op)
{
	register PLAYER *p;
	register long	c;
	register long	v;
	extern void	menu_xo(PLAYER *);

	p = TO_PLR(op);
	play_sound(p->shield_snd);
	p->is_hyper = (HYPER_PRE + 11) * 2;
	p->ec_cnt = 9;

	switch (RND(16)) {
	case 0: 				/* heal shields! */
		CopyMem(ship_image, p->obj.o_image, sizeof(ship_image));
		ship_image(p);
		p->shields_blown = FALSE;
		v = 0xf42;

	CASE 1: 	case 2: 	case 3: /* random turn */
		do {
			v = RND(MAX_ANGLES) - MAX_ANGLES / 2 + 1;
		} while (v == 0);
		p->rot = v;
		rotate_ship(p);
		p->rot_cnt = p->rot = 0;
		v = 0x8f2;

	CASE 4: 	case 5: 	case 6: /* random velocity */
		do {
			v = RND(MAX_ANGLES) - MAX_ANGLES / 2 + 1;
		} while (v == 0);

		v += atan2(p->obj.o_vx, p->obj.o_vy);
		c = cos32(v);				/* unit vectors */
		v = sin32(v);
							/* cross multiply */
		p->obj.o_vx = p->obj.o_nvx =
			mult64(p->obj.o_vx, c) - mult64(p->obj.o_vy, v);
		p->obj.o_vy = p->obj.o_nvy =
			mult64(p->obj.o_vx, v) + mult64(p->obj.o_vy, c);
		v = 0xf7f;

	CASE 7: 	case 8: 	case 9: /* random xtra weapon */
		p->xtra_opt = RND(NUM_XOPTS);
		menu_xo(p);
		v = 0xff8;

	CASE 10:	case 11:	case 12:/* ordinary hit */
		return(0);

	default:
		play_sound(p->hyper_out_snd);
		p->is_hyper = (HYPER_PRE + 5) * 2;
		v = 0xfab;
	}
	c = p->color;
	colors[c++] = v;
	colors[c++] = v;
	colors[c] = v;
	changed_color = 1;
	return(1);
}


/*
 * torp_check -- check for collisions between the given object's sprite
 *		and any photon torpedos
 */

Static int
torp_check(register OBJ *op)
{
	register OBJ	*torp;
	register long	dx, dy, ix, iy, lim, a;
	register UWORD	*wptr;
	uint		otype;
	uint		hit = 0;
	uint		am_explosion;

	otype = op->o_type;
	am_explosion = (otype == OTYPE_EXPLOSION);
	a = op->o_offset;
	lim = 2 * a + 1;
	ix = op->o_ix - a;
	iy = op->o_iy - a;

	for (torp = ohead; torp; torp = torp->o_next) {
		if (torp->o_type != OTYPE_TORP)
			continue;			/* don't bother */

		dx = torp->o_ix - ix;
		dy = torp->o_iy - iy;
		if (dx < 0 || dy < 0 || dx >= lim || dy >= lim)
			continue;			/* no overlap */

		if (am_explosion && torp->o_ship == TO_PLR(op))
			continue;	/* debris doesn't hit explosion */

		a = vbit[dx];
		wptr = &op->o_sprptr->posctldata[(dy << 1) + 2];
		if ((wptr[0] & a) == 0 && (wptr[1] & a) == 0)
			continue;			/* missed */

		torp->o_energy = 0;			/* blam!!! */
		++hit;
		if (OT_SPRITE(otype)) {
			op->o_energy = 0;	/* "Just one phaser burst */
			continue;		/*  would detonate it..." */
		}
		if (otype != OTYPE_SHIP) {
			/*
			 * hit asteroid, randomize colors to fake craters
			 */
			if (otype == OTYPE_ASTEROID) {
				while ((dx = ARND(4)) == 0)
					;
				for ( ; dx; dx >>= 1) {
					if (dx & 1)
						*wptr++ |= a;
					else
						*wptr++ &= ~a;
				}
			}

			continue;			/* can't zap suns */
		}

		/* did we hit the shield, or the ship? */

		if (TO_PLR(op)->full_shields) {
			play_sound(TO_PLR(op)->shield_snd); /* hit!!! */
			TO_PLR(op)->shield_hit = SHIELD_FLARE;
			continue;			/* no erosion */
		}
		if (SHIELD_CK(dx, dy) == 0) {
			op->o_energy = 0;		/* hit and sunk */
			continue;
		}
		a = op->o_offset;
		erode_shield(op, atan2((dx - a) << 1, (dy - a) << 1));

	}
	return ((int)hit);
}


/*
 * spr_hit -- a sprite has hit another sprite -- assess the damage for ship
 *		returns 2 if ship destroyed, 1 if shields blown, else 0
 */

Static int
spr_hit(register OBJ *spr, register OBJ *hit)
{
	register long	hc, sc, num_rows, n;
	register ulong	*maskp, *srcp;

	srcp = (ulong *)(&spr->o_sprptr->posctldata[2]);
	maskp = (ulong *)(&hit->o_sprptr->posctldata[2]);

	hc = hit->o_iy - hit->o_offset;
	sc = spr->o_iy - spr->o_offset;
	if ((n = hc - sc) < 0) {
		maskp -= n;
		num_rows = sc;
	}
	else {
		srcp += n;
		num_rows = hc;
	}
	hc = hit->o_iy + hit->o_offset;
	sc = spr->o_iy + spr->o_offset;
	num_rows = min(sc, hc) - num_rows + 1;

	n = (hit->o_ix - hit->o_offset) - (spr->o_ix - spr->o_offset);
	n = and_sprites(srcp, maskp, num_rows, n);

	if (n & (4 | 2))
		return (2);	/* body or engine/porthole hit */
	return (n & 1);		/* shield hit, or nothing */
}


/*
 * sprite_check -- check for sprite to sprite collisions
 */

Static void
sprite_check(register OBJ *fp, register OBJ *tp)
{
	register long	dx, dy;
	register long	off, n;

	off = fp->o_offset + tp->o_offset;
	dx = tp->o_ix - fp->o_ix;
	dy = tp->o_iy - fp->o_iy;
	if (dx > off || dy > off || dx < -off || dy < -off)
		return; 			/* too far away */

	if (tp->o_type < fp->o_type) {
		OBJ	*tmp;

		tmp = tp;			/* fp should be < tp */
		tp = fp;
		fp = tmp;
	}

	if ((n = spr_hit(fp, tp)) == 0)
		return; 			/* didn't touch */

	switch (fp->o_type) {
	case OTYPE_SHIP:
		TO_PLR(fp)->thrust = 0;
		flush_sound(TO_PLR(fp)->thrust_snd);
		if (tp->o_type == OTYPE_SHIP) {
			TO_PLR(tp)->thrust = 0;
			flush_sound(TO_PLR(tp)->thrust_snd);
			n += spr_hit(tp, fp) << 2;
			off = fp->o_nvx;	/* ships bounce */
			fp->o_nvx = tp->o_nvx;
			tp->o_nvx = off;
			off = fp->o_nvy;
			fp->o_nvy = tp->o_nvy;
			tp->o_nvy = off;
			fp->o_nx = fp->o_x;	/* back up coords */
			fp->o_ny = fp->o_y;
			tp->o_nx = tp->o_x;
			tp->o_ny = tp->o_y;
			if (n & 2)
				fp->o_energy = 0;
			else if (n & 1)
				blow_shields(fp);
			if (n & 8)
				tp->o_energy = 0;
			else if (n & 4)
				blow_shields(tp);
		}
		else {			/* ship hit asteroid sprite or star */
			if (OT_SPRITE(tp->o_type)) {
				tp->o_energy = 0;	/* hit sprite weapon */

				/* special hit actions */
				switch (tp->o_type) {
				case OTYPE_ANOM:
					if (anomalize(fp))
						return; /* zot! */
				CASE OTYPE_DISRUPTOR:
					TO_PLR(fp)->is_hyper =
					  (HYPER_PRE + 22 + ARND(8)) * 2 + 1;
					TO_PLR(fp)->rot = ARND(2) ? 1 : -1;
					n = TO_PLR(fp)->color;
					colors[n] = colors[n + 1] = 0xFFF;
					changed_color = 1;
					play_sound(TO_PLR(fp)->hyper_out_snd);
					return;
				}
				off = 1;
			}
			else
				off = 0;
			if (n & 2) {
				fp->o_energy = 0;	/* unshielded ship */
				return;
			}
			blow_shields(fp);	/* shielded ship */
			if (off)
				return; 	/* don't bounce off plasma */
			/*
			 * bounce ship -- do grazing collisions
			 */
			off = atan2(dx << 3, dy << 3);  /* relative angle */
			fp->o_x += cos32(off);		/* bounce 1 unit */
			fp->o_y += sin32(off);
			dx = tp->o_vx - fp->o_vx;	/* relative vel */
			n = tp->o_vy - fp->o_vy;
						/* velocity rotation angle */
			dy = NUM_A32/2 - 2 * (atan2(dx, n) - off);
			off = dx;
			dx = cos32(dy);			/* unit vectors */
			dy = sin32(dy);
							/* cross multiply */
			fp->o_vx = -(mult64(off, dx) - mult64(n, dy));
			fp->o_vy = -(mult64(off, dy) + mult64(n, dx));
			grav1(fp);
		}
	CASE OTYPE_ASTEROID:
		if (tp->o_type != OTYPE_ASTEROID) {
			play_sound(plr_1.explode_snd);
			play_sound(plr_2.explode_snd);
			fp->o_energy = 0;	/* asteroid hit star */
			break;
		}
		/* else, asteroid bounces off asteroid */

		if (fp->o_ship == TO_PLR(tp) && BOUNCER(fp) ||
		    tp->o_ship == TO_PLR(fp) && BOUNCER(tp)) {
			break;			/* no double bounces */
		}
		off = fp->o_nvx;		/* asteroids bounce */
		fp->o_nvx = tp->o_nvx;
		tp->o_nvx = off;
		off = fp->o_nvy;
		fp->o_nvy = tp->o_nvy;
		tp->o_nvy = off;
		fp->o_nx = fp->o_x;		/* back up coords */
		fp->o_ny = fp->o_y;
		fp->o_nix = fp->o_ix;
		fp->o_niy = fp->o_iy;
		tp->o_nx = tp->o_x;
		tp->o_ny = tp->o_y;
		tp->o_nix = tp->o_ix;
		tp->o_niy = tp->o_iy;
		fp->o_moveme = tp->o_moveme = 1;
		/*
		 * back off one unit
		 */
		do {
			fp->o_nix = (fp->o_nx += fp->o_nvx) >> CD_FRAC;
			fp->o_niy = (fp->o_ny += fp->o_nvy) >> CD_FRAC;
			tp->o_nix = (tp->o_nx += tp->o_nvx) >> CD_FRAC;
			tp->o_niy = (tp->o_ny += tp->o_nvy) >> CD_FRAC;
		} while (fp->o_nix == fp->o_ix && fp->o_niy == fp->o_iy &&
			 tp->o_nix == tp->o_ix && tp->o_niy == tp->o_iy);
		/*
		 * Avoid double collisions by remembering past
		 * collider and time stamp.
		 */
		fp->o_ship = TO_PLR(tp);
		tp->o_ship = TO_PLR(fp);
		fp->o_ntime = fp->o_energy;
		tp->o_ntime = tp->o_energy;
	CASE OTYPE_ANOM:
		if (tp->o_type == OTYPE_ANOM) {
			morph_weapon(&plr_1);		/* transmorg */
			morph_weapon(&plr_2);		/* transmorg */
			break;
		}
		fp->o_energy = 0;			/* anomalizer hit */
		if (tp->o_type >= OTYPE_RED_STAR && RND(47) == 0) {
			morph_star(tp);
			break;
		}
		if (OT_SPRITE(tp->o_type)) {
			if (ARND(2)) {
				tp->o_energy = 0;	/* 50:50 blow up */
				break;
			}
			morph_weapon(fp->o_ship);	/* transmorg */
		}
	CASE OTYPE_CLUSTER:
	case OTYPE_GUIDED:
	case OTYPE_PLASMA:
	case OTYPE_SEEKER:
	case OTYPE_DISRUPTOR:
		fp->o_energy = 0;			/* plasma bolt hit */
		if (OT_SPRITE(tp->o_type))
			tp->o_energy = 0;
	CASE OTYPE_EXPLOSION:
		switch (tp->o_type) {
		case OTYPE_SEEKER:
		case OTYPE_PLASMA:
		case OTYPE_CLUSTER:
		case OTYPE_GUIDED:
		case OTYPE_ANOM:
			tp->o_energy = 0;	/* detonate bolt or seeker */
			return;
		case OTYPE_ASTEROID:		/* expl hit star - stop it */
		case OTYPE_RED_STAR:
		case OTYPE_YELLOW_STAR:
		case OTYPE_BLUE_STAR:
		case OTYPE_SM_HOLE:
		case OTYPE_MED_HOLE:
		case OTYPE_LRG_HOLE:
			fp->o_nvx = (fp->o_vx += (tp->o_vx - fp->o_vx) >> 1);
			fp->o_nvy = (fp->o_vy += (tp->o_vy - fp->o_vy) >> 1);
		}
	DEFAULT:
		nova(fp, tp);
	}
}


/*
 * collide -- process sprite / playfield collisions
 */

void
collide(void)
{
	register ushort clxdat;
	register int	i, n, j;
	register OBJ	*op, *np;
	static uchar	from[] = { 4, 2, 2, 0, 0, 0 };
	static uchar	to[]   = { 6, 6, 4, 6, 4, 2 };

	clxdat = copper_data.c_clxdat;
#if 0
*(ushort *)plane[0] = clxdat;
*(ushort *)plane[1] = 0xaaaa;
#endif

	/*
	 * check odd bit plane to sprite pair collisions
	 */
	for (i = 0, n = 4; (clxdat >>= 1, --n >= 0); i += 2) {
		if ((clxdat & 1) == 0)
			continue;

		if (op = ob_find_spr(i + 1))
			torp_check(op);
		if (op = ob_find_spr(i))
			torp_check(op);
	}

	/*
	 * check sprite to sprite collisions
	 */
	clxdat >>= 4;
	for (n = 6; --n >= 0; clxdat >>= 1) {
		if ((clxdat & 1) == 0)
			continue;

		j = to[n];
		i = from[n];
		if (op = ob_find_spr(i + 1)) {
			if (np = ob_find_spr(j + 1))
				sprite_check(op, np);
			if (op->o_energy > 0 && (np = ob_find_spr(j)))
				sprite_check(op, np);
		}
		if (op = ob_find_spr(i)) {
			if (np = ob_find_spr(j + 1))
				sprite_check(op, np);
			if (op->o_energy > 0 && (np = ob_find_spr(j)))
				sprite_check(op, np);
		}
	}

	/*
	 * remove any destroyed objects, torpedos first
	 */
	for (op = ohead; op; op = np) {
		np = op->o_next;
		if (op->o_energy <= 0 && op->o_type == OTYPE_TORP)
			ob_unlink(op);
	}

	for (op = ohead; op; op = np) {
		np = op->o_next;
		if (op->o_energy <= 0)
			ob_unlink(op);
	}
}


#define xlim	neg
#define ylim	sm
#define vlimit	sh
#define s_plane ((ushort **)sun)
#define offset	n
#define n_spr_2 n


#if 0

/*
 * grav_universe -- gravitate the universe through one cycle
 */

void
grav_universe(void)
{
	register OBJ	*sun, *mov;
	register long	ix, iy, n, sh, sm, neg;

	/* First, loop through all objects, removing those that are out of
	 * energy, and if they moved, copying the new object state to the
	 * current object state.
	 *
	 * Update this loop if OBJ_STATE changes in ssw.h!
	 */
	for (sun = ohead; sun; sun = mov) {
		mov = sun->o_next;
		if (--sun->o_energy < 0) {
			ob_unlink(sun);		/* gone */
			continue;
		}
		if (! sun->o_moveme)
			continue;		/* updated by grav loop */
		sun->o_x = sun->o_nx;
		sun->o_y = sun->o_ny;
		sun->o_vx = sun->o_nvx;
		sun->o_vy = sun->o_nvy;
		sun->o_ix = sun->o_nix;
		sun->o_iy = sun->o_niy;
		sun->o_sprd = sun->o_nsprd;
		sun->o_tmask = sun->o_ntmask;
	}

	/*
	 * Next, calculate the accelerations on all remaining objects
	 */
	sun = ohead;
	for ( ; sun && (sm = sun->o_mass) >= 0; sun = sun->o_next) {
		for (mov = sun->o_next; mov; mov = mov->o_next) {
			neg = 0;
			if ((ix = sun->o_x - mov->o_x) < 0) {
				ix = -ix;
				neg = 1;
			}
			if ((iy = sun->o_y - mov->o_y) < 0) {
				iy = -iy;
				neg += 2;
			}
			ix >>= CD_FRAC - GRAV_RES;
			iy >>= CD_FRAC - GRAV_RES;
			sh = 0;
			while (ix >= GRAV_SIZE || iy >= GRAV_SIZE) {
				ix >>= 1;
				iy >>= 1;
				sh += 2;
			}
			n = gravtbl[ix][iy] >> sh;	/* n is x accel */
			iy = gravtbl[iy][ix] >> sh;	/* iy now y acc */
			if (neg & 1)
				n = -n;
			if (neg & 2)
				iy = -iy;
			mov->o_nvx += n >> sm;
			mov->o_nvy += iy >> sm;
			if ((sh = mov->o_mass) >= 0) {
				sun->o_nvx -= n >> sh;
				sun->o_nvy -= iy >> sh;
			}
		}
	}

	/*
	 * Finally, decide if each object will move; if so, clip its
	 * velocity and position, and update its pointers.
	 */

	xlim = WIDTH - 1;
	ylim = y_height - 1;
	vlimit = VEL_LIMIT;
	sun = (OBJ *)plane;

	if (bound_type == 0) {			/* bounce */
		for (mov = ohead; mov; mov = mov->o_next) {
			if ((ix = mov->o_nvx) > vlimit)
				mov->o_nvx = ix = vlimit;
			else if (ix < -vlimit)
				mov->o_nvx = ix = -vlimit;
			if ((iy = mov->o_nvy) > vlimit)
				mov->o_nvy = iy = vlimit;
			else if (iy < -vlimit)
				mov->o_nvy = iy = -vlimit;
			mov->o_nx += ix;		/* move object */
			mov->o_ny += iy;
			ix = mov->o_nx >> CD_FRAC;
			iy = mov->o_ny >> CD_FRAC;
			if (ix == mov->o_ix && iy == mov->o_iy) {
				mov->o_moveme = 0; /* didn't move on screen*/
				mov->o_x = mov->o_nx;
				mov->o_y = mov->o_ny;
				mov->o_vx = mov->o_nvx;
				mov->o_vy = mov->o_nvy;
				continue;
			}
			mov->o_moveme = 1;		/* move 'em */
			offset = mov->o_offset;
			if (ix < offset) {
				ix = offset;
				mov->o_nx = ix << CD_FRAC;
				mov->o_nvx = -mov->o_nvx;
			}
			else if (ix > xlim - offset) {
				ix = xlim - offset;
				mov->o_nx = ix << CD_FRAC;
				mov->o_nvx = -mov->o_nvx;
			}
			if (iy < offset) {
				iy = offset;
				mov->o_ny = iy << CD_FRAC;
				mov->o_nvy = -mov->o_nvy;
			}
			else if (iy > ylim - offset) {
				iy = ylim - offset;
				mov->o_ny = iy << CD_FRAC;
				mov->o_nvy = -mov->o_nvy;
			}
			mov->o_nix = ix;
			mov->o_niy = iy;
			if (mov->o_sprptr) {
				mov->o_nsprd = move_sprite(mov->o_sprptr,
						 ix - offset, iy - offset);
			}
			else {
				mov->o_ntptr = &s_plane[iy][ix >> 4];
				mov->o_ntmask = vbit[ix & 0xf];
			}
		}
	}
	else {					/* wrap */
		n_spr_2 = -SPRITE_WIDTH_2;
		for (mov = ohead; mov; mov = mov->o_next) {
			if ((ix = mov->o_nvx) > vlimit)
				mov->o_nvx = ix = vlimit;
			else if (ix < -vlimit)
				mov->o_nvx = ix = -vlimit;
			if ((iy = mov->o_nvy) > vlimit)
				mov->o_nvy = iy = vlimit;
			else if (iy < -vlimit)
				mov->o_nvy = iy = -vlimit;
			mov->o_nx += ix;		/* move object */
			mov->o_ny += iy;
			ix = mov->o_nx >> CD_FRAC;
			iy = mov->o_ny >> CD_FRAC;
			if (ix == mov->o_ix && iy == mov->o_iy) {
				mov->o_moveme = 0; /* didn't move on screen*/
				mov->o_x = mov->o_nx;
				mov->o_y = mov->o_ny;
				mov->o_vx = mov->o_nvx;
				mov->o_vy = mov->o_nvy;
				continue;
			}
			mov->o_moveme = 1;		/* move 'em */
			if (ix < n_spr_2) {
				ix = WIDTH + SPRITE_WIDTH_2 - 1;
				mov->o_nx = ix << CD_FRAC;
			}
			else if (ix > WIDTH + SPRITE_WIDTH_2 - 1) {
				ix = n_spr_2;
				mov->o_nx = ix << CD_FRAC;
			}
			if (iy < n_spr_2) {
				iy = y_spr_max;
				mov->o_ny = iy << CD_FRAC;
			}
			else if (iy > y_spr_max) {
				iy = n_spr_2;
				mov->o_ny = iy << CD_FRAC;
			}
			mov->o_nix = ix;
			mov->o_niy = iy;
			if (mov->o_sprptr) {
				mov->o_nsprd = move_sprite(mov->o_sprptr,
				  ix - mov->o_offset, iy - mov->o_offset);
			}
			else {
				if (ix >= 0 && iy >= 0 &&
				    ix <= xlim && iy <= ylim) {
					mov->o_ntptr = &s_plane[iy][ix >> 4];
					mov->o_ntmask = vbit[ix & 0xf];
				}
				else
					mov->o_ntptr = NULL;
			}
		}
	}
}
#endif


/*
 * grav1 -- regravitate a collided ship -- don't try this on a star
 *
 *	Note that it works entirely from the current data, and fills in new
 */

void
grav1(register OBJ *mov)
{
	register OBJ	*sun;
	register long	ix, iy, n, sh, sm, neg;

	mov->o_nx = mov->o_x;
	mov->o_ny = mov->o_y;
	mov->o_nvx = mov->o_vx;
	mov->o_nvy = mov->o_vy;
	/*
	 * calculate the accelerations on all remaining objects
	 */
	sun = ohead;
	for ( ; sun && (sm = sun->o_mass) >= 0; sun = sun->o_next) {
		if (sun == mov)
			continue;			/* don't do self */
		neg = 0;
		if ((ix = sun->o_x - mov->o_x) < 0) {
			ix = -ix;
			neg = 1;
		}
		if ((iy = sun->o_y - mov->o_y) < 0) {
			iy = -iy;
			neg += 2;
		}
		ix >>= CD_FRAC - GRAV_RES;
		iy >>= CD_FRAC - GRAV_RES;
		sh = 0;
		while (ix >= GRAV_SIZE || iy >= GRAV_SIZE) {
			ix >>= 1;
			iy >>= 1;
			sh += 2;
		}
		n = gravtbl[ix][iy] >> sh;		/* n is x accel */
		iy = gravtbl[iy][ix] >> sh;		/* iy now y acc */
		if (neg & 1)
			n = -n;
		if (neg & 2)
			iy = -iy;
		mov->o_nvx += n >> sm;
		mov->o_nvy += iy >> sm;
	}

	/*
	 * Finally, decide if the object will move; if so, clip its
	 * velocity and position, and update its pointers.
	 */

#define xlim	neg
#define ylim	sm
#define vlimit	sh
#define offset	n
#define n_spr_2 n

	xlim = WIDTH - 1;
	ylim = y_height - 1;
	vlimit = VEL_LIMIT;

	if ((ix = mov->o_nvx) > vlimit)
		ix = vlimit;
	else if (ix < -vlimit)
		ix = -vlimit;
	if ((iy = mov->o_nvy) > vlimit)
		iy = vlimit;
	else if (iy < -vlimit)
		iy = -vlimit;
	ix = (mov->o_nx += ix) >> CD_FRAC;	/* move object */
	iy = (mov->o_ny += iy) >> CD_FRAC;
	if (ix == mov->o_ix && iy == mov->o_iy) {
		mov->o_moveme = 0;		/* didn't move on screen*/
		mov->o_x = mov->o_nx;
		mov->o_y = mov->o_ny;
		mov->o_vx = mov->o_nvx;
		mov->o_vy = mov->o_nvy;
		return;
	}
	mov->o_moveme = 1;			/* move 'em */

	if (bound_type == 0) {			/* bounce */
		offset = mov->o_offset;
		if (ix < offset) {
			ix = offset;
			mov->o_nx = ix << CD_FRAC;
			mov->o_nvx = -mov->o_nvx;
		}
		else if (ix > xlim - offset) {
			ix = xlim - offset;
			mov->o_nx = ix << CD_FRAC;
			mov->o_nvx = -mov->o_nvx;
		}
		if (iy < offset) {
			iy = offset;
			mov->o_ny = iy << CD_FRAC;
			mov->o_nvy = -mov->o_nvy;
		}
		else if (iy > ylim - offset) {
			iy = ylim - offset;
			mov->o_ny = iy << CD_FRAC;
			mov->o_nvy = -mov->o_nvy;
		}
		mov->o_nix = ix;
		mov->o_niy = iy;
		if (mov->o_sprptr) {
			mov->o_nsprd = move_sprite(mov->o_sprptr,
					ix -= offset, iy -= offset);
		}
		else {
			mov->o_ntptr = &plane[iy][ix >> 4];
			mov->o_ntmask = vbit[ix & 0xf];
		}
	}
	else {					/* wrap */
		n_spr_2 = -SPRITE_WIDTH_2;
		if (ix < n_spr_2) {
			ix = WIDTH + SPRITE_WIDTH_2 - 1;
			mov->o_nx = ix << CD_FRAC;
		}
		else if (ix > WIDTH + SPRITE_WIDTH_2 - 1) {
			ix = n_spr_2;
			mov->o_nx = ix << CD_FRAC;
		}
		if (iy < n_spr_2) {
			iy = y_spr_max;
			mov->o_ny = iy << CD_FRAC;
		}
		else if (iy > y_spr_max) {
			iy = n_spr_2;
			mov->o_ny = iy << CD_FRAC;
		}
		mov->o_nix = ix;
		mov->o_niy = iy;
		if (mov->o_sprptr) {
			offset = mov->o_offset;
			mov->o_nsprd = move_sprite(mov->o_sprptr,
					ix -= offset, iy -= offset);
		}
		else {
			if (ix >= 0 && iy >= 0 &&
			    ix <= xlim && iy <= ylim) {
				mov->o_ntptr = &s_plane[iy][ix >> 4];
				mov->o_ntmask = vbit[ix & 0xf];
			}
			else
				mov->o_ntptr = NULL;
		}
	}
}

