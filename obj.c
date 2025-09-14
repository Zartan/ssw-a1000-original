/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * obj.c -- define objects
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

#ident "$Id: obj.c 1.2 1997/01/19 07:04:58 jamesc Exp jamesc $"

#include "ssw.h"

#define I_OBJ(ot, x, y, vx, vy, im, spr) { im, C_X + x, y, vx, vy, ot, spr }

#define SI_INIT(arr)		(arr), (NEL(arr) / 4), (NEL(arr) / 2)
#define SHIP_INIT()		ship_image, SHIP_IM_H_2, SHIP_IM_HEIGHT

#define I_SSYS(array)		{ array, NEL(array) }

extern ushort	ship_image[], big_sun[30], med_sun[26], small_sun[22],
		asteroid[14], ast_1[18], ast_2[18], ast_3[18],
		black_hole[2];

Static SSW_IMAGE
	si_ship1    = { SHIP_INIT(),		{ 0xf00, 0x555, 0x0A0 } },
	si_ship2    = { SHIP_INIT(),		{ 0xf00, 0x555, 0x808 } },
	si_red	    = { SI_INIT(small_sun),	{ 0xC33, 0xF33, 0xA42 } },
	si_yellow   = { SI_INIT(med_sun),	{ 0xBB3, 0xEE3, 0x883 } },
	si_blue     = { SI_INIT(big_sun),	{ 0xAAE, 0xBBF, 0x88D } },
	si_black    = { SI_INIT(black_hole),	{ 0x95A, 0x848, 0xF7F } },
	si_asteroid = { SI_INIT(asteroid),	{ 0x755, 0x343, 0x557 } },
	si_ast_1    = { SI_INIT(ast_1),		{ 0x755, 0x343, 0x557 } },
	si_ast_2    = { SI_INIT(ast_2),		{ 0x755, 0x343, 0x557 } },
	si_ast_3    = { SI_INIT(ast_3),		{ 0x755, 0x343, 0x557 } };

/*
 * Key for velocity defines:
 *  s=ship
 *  A=asteroid
 *  R=red dwarf
 *  Y=yellow star
 *  B=blue giant
 *  S=small black hole
 *  M=medium black hole
 *  L=large black hole
 *
 * R_RR_BIN means velocity of red dwarf in red-red binary
 * A_L_SGL  means velocity of asteroid in large black hole single
 */
#define R_RR_BIN	937872
#define s_RR_BIN	1243610
#define R_RY_BIN	1398101
#define Y_RY_BIN	(R_RY_BIN / 2)
#define s_RY_BIN	1523105
#define R_RB_BIN	2344687
#define B_RB_BIN	(R_RB_BIN / 4)
#define s_RB_BIN	1966320
#define R_RS_BIN	R_RB_BIN
#define S_RS_BIN	B_RB_BIN
#define s_RS_BIN	s_RB_BIN
#define R_RM_BIN	3954428
#define M_RM_BIN	(R_RM_BIN / 8)
#define s_RM_BIN	2638095
#define R_RL_BIN	5305421
#define L_RL_BIN	(R_RL_BIN / 16)
#define s_RL_BIN	3625715
#define Y_YY_BIN	1326351
#define s_YY_BIN	1758730
#define Y_YB_BIN	1977214
#define B_YB_BIN	(Y_YB_BIN / 2)
#define s_YB_BIN	2153996
#define Y_YS_BIN	Y_YB_BIN
#define S_YS_BIN	B_YB_BIN
#define s_YS_BIN	s_YB_BIN
#define Y_YM_BIN	2965821
#define M_YM_BIN	(Y_YM_BIN / 4)
#define s_YM_BIN	2780796
#define Y_YL_BIN	5592405
#define L_YL_BIN	(Y_YL_BIN / 8)
#define s_YL_BIN	3730830
#define B_BB_BIN	1875750
#define s_BB_BIN	2487220
#define B_BS_BIN	B_BB_BIN
#define S_BS_BIN	B_BB_BIN
#define s_BS_BIN	s_BB_BIN
#define B_BM_BIN	3228776
#define M_BM_BIN	(B_BM_BIN / 2)
#define s_BM_BIN	3046210
#define B_BL_BIN	4689374
#define L_BL_BIN	(B_BL_BIN / 4)
#define s_BL_BIN	3932640
#define S_SS_BIN	B_BB_BIN
#define s_SS_BIN	s_BB_BIN
#define S_SM_BIN	B_BM_BIN
#define M_SM_BIN	M_BM_BIN
#define s_SM_BIN	s_BM_BIN
#define S_SL_BIN	B_BL_BIN
#define L_SL_BIN	L_BL_BIN
#define s_SL_BIN	s_BL_BIN
#define M_MM_BIN	2652711
#define s_MM_BIN	3381036
#define M_ML_BIN	4566180
#define L_ML_BIN	(M_ML_BIN / 2)
#define s_ML_BIN	4307991
#define L_LL_BIN	3751499
#define s_LL_BIN	4974440
#define S_EG_BIN	700000
#define s_EG_BIN	1827500
#define A_R_SGL 	1186328
#define s_R_SGL 	879365
#define A_Y_SGL 	1677721
#define s_Y_SGL 	s_RR_BIN
#define A_B_SGL 	2372657
#define s_B_SGL 	s_YY_BIN
#define A_S_SGL 	A_B_SGL
#define s_S_SGL 	s_YY_BIN
#define A_M_SGL 	3355443
#define s_M_SGL 	s_BB_BIN
#define A_L_SGL 	4745313
#define s_L_SGL 	s_MM_BIN
#define R_RYY_TRI	1914429
#define Y_RYY_TRI_X	(R_RYY_TRI / 4)
#define Y_RYY_TRI_Y	B_BB_BIN
#define s_RYY_TRI	s_RB_BIN
#define Y_YBB_TRI	2707411
#define B_YBB_TRI_X	(Y_YBB_TRI / 4)
#define B_YBB_TRI_Y	M_MM_BIN
#define s_YBB_TRI	2780796
#define Y_YSS_TRI	Y_YBB_TRI
#define S_YSS_TRI_X	B_YBB_TRI_X
#define S_YSS_TRI_Y	B_YBB_TRI_Y
#define s_YSS_TRI	s_YBB_TRI
#define B_BMM_TRI	3828858
#define M_BMM_TRI_X	(B_BMM_TRI / 4)
#define M_BMM_TRI_Y	3751499
#define s_BMM_TRI	s_BL_BIN
#define M_MLL_TRI	5414823
#define L_MLL_TRI_X	(M_MLL_TRI / 4)
#define L_MLL_TRI_Y	5305421
#define s_MLL_TRI	5561593
#define R_WR_BIN	501315
#define s_WR_BIN	(R_WR_BIN + 1830544)
#define Y_WY_BIN	708967
#define s_WY_BIN	(Y_WY_BIN + 2588781)
#define B_WB_BIN	1002630
#define s_WB_BIN	(B_WB_BIN + 3661089)
#define S_WS_BIN	B_WB_BIN
#define s_WS_BIN	s_WB_BIN
#define M_WM_BIN	1417933
#define s_WM_BIN	(M_WM_BIN + 5177561)
#define L_WL_BIN	2005261
#define s_WL_BIN	(L_WL_BIN + 7322177)
#define R_R_QUAD_X	Y_WY_BIN
#define R_R_QUAD_Y	R_RR_BIN
#define s_R_QUAD	s_B_SGL
#define Y_Y_QUAD_X	B_WB_BIN
#define Y_Y_QUAD_Y	Y_YY_BIN
#define s_Y_QUAD	s_M_SGL
#define B_B_QUAD_X	M_WM_BIN
#define B_B_QUAD_Y	B_BB_BIN
#define s_B_QUAD	s_L_SGL
#define S_S_QUAD_X	M_WM_BIN
#define S_S_QUAD_Y	S_SS_BIN
#define s_S_QUAD	s_L_SGL

Static IOBJ
		nothing[] = {
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,         0, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,         0, &si_ship2, 6),
},
		only_asteroid[] = {
 I_OBJ(OTYPE_ASTEROID,	  0,  0,          0,         0, &si_ast_1, 2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,         0, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,         0, &si_ship2, 6),
},
		r_single[] = {
 I_OBJ(OTYPE_RED_STAR,	  0,  0,          0,         0, &si_red,   2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_R_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_R_SGL, &si_ship2, 6),
},
		r_asteroid[] = {
 I_OBJ(OTYPE_RED_STAR,	  0,  0,          0,         0, &si_red,   2),
 I_OBJ(OTYPE_ASTEROID,	  0, 50,    A_R_SGL,         0, &si_asteroid, 1),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_R_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_R_SGL, &si_ship2, 6),
},
		y_single[] = {
 I_OBJ(OTYPE_YELLOW_STAR, 0,  0,          0,         0, &si_yellow,2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_Y_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_Y_SGL, &si_ship2, 6),
},
		y_asteroid[] = {
 I_OBJ(OTYPE_YELLOW_STAR, 0,  0,          0,         0, &si_yellow,2),
 I_OBJ(OTYPE_ASTEROID,	  0, 50,    A_Y_SGL,         0, &si_asteroid, 1),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_Y_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_Y_SGL, &si_ship2, 6),
},
		b_single[] = {
 I_OBJ(OTYPE_BLUE_STAR,	  0,  0,          0,         0, &si_blue,  2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_B_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_B_SGL, &si_ship2, 6),
},
		b_asteroid[] = {
 I_OBJ(OTYPE_BLUE_STAR,	  0,  0,          0,         0, &si_blue,  2),
 I_OBJ(OTYPE_ASTEROID,	  0, 50,    A_B_SGL,         0, &si_asteroid, 1),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_B_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_B_SGL, &si_ship2, 6),
},
		s_single[] = {
 I_OBJ(OTYPE_SM_HOLE,	  0,  0,          0,         0, &si_black, 2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_S_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_S_SGL, &si_ship2, 6),
},
		s_asteroid[] = {
 I_OBJ(OTYPE_SM_HOLE,	  0,  0,          0,         0, &si_black, 1),
 I_OBJ(OTYPE_ASTEROID,	  0, 50,    A_S_SGL,         0, &si_asteroid, 2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_S_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_S_SGL, &si_ship2, 6),
},
		m_single[] = {
 I_OBJ(OTYPE_MED_HOLE,	  0,  0,          0,         0, &si_black, 2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_M_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_M_SGL, &si_ship2, 6),
},
		m_asteroid[] = {
 I_OBJ(OTYPE_MED_HOLE,	  0,  0,          0,         0, &si_black, 1),
 I_OBJ(OTYPE_ASTEROID,	  0, 50,    A_M_SGL,         0, &si_asteroid, 2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_M_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_M_SGL, &si_ship2, 6),
},
		l_single[] = {
 I_OBJ(OTYPE_LRG_HOLE,	  0,  0,          0,         0, &si_black, 2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_L_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_L_SGL, &si_ship2, 6),
},
		l_asteroid[] = {
 I_OBJ(OTYPE_LRG_HOLE,	  0,  0,          0,         0, &si_black, 1),
 I_OBJ(OTYPE_ASTEROID,	  0, 50,    A_L_SGL,         0, &si_asteroid, 2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_L_SGL, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_L_SGL, &si_ship2, 6),
},
		rr_binary[] = {
 I_OBJ(OTYPE_RED_STAR,	 20,  0,          0,  R_RR_BIN, &si_red,   2),
 I_OBJ(OTYPE_RED_STAR,	-20,  0,          0, -R_RR_BIN, &si_red,   3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_RR_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_RR_BIN, &si_ship2, 6),
},
		ry_binary[] = {
 I_OBJ(OTYPE_YELLOW_STAR,-13,  0,          0, -Y_RY_BIN, &si_yellow,1),
 I_OBJ(OTYPE_RED_STAR,	 27,  0,          0,  R_RY_BIN, &si_red,   2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_RY_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_RY_BIN, &si_ship2, 6),
},
		rb_binary[] = {
 I_OBJ(OTYPE_BLUE_STAR,	 -8,  0,          0, -B_RB_BIN, &si_blue,  1),
 I_OBJ(OTYPE_RED_STAR,	 32,  0,          0,  R_RB_BIN, &si_red,   2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_RB_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_RB_BIN, &si_ship2, 6),
},
		rs_binary[] = {
 I_OBJ(OTYPE_SM_HOLE,	 -8,  0,          0, -S_RS_BIN, &si_black, 1),
 I_OBJ(OTYPE_RED_STAR,	 32,  0,          0,  R_RS_BIN, &si_red,   2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_RS_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_RS_BIN, &si_ship2, 6),
},
		rm_binary[] = {
 I_OBJ(OTYPE_MED_HOLE,	 -4,  0,          0, -M_RM_BIN, &si_black, 1),
 I_OBJ(OTYPE_RED_STAR,	 36,  0,          0,  R_RM_BIN, &si_red,   2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_RM_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_RM_BIN, &si_ship2, 6),
},
		rl_binary[] = {
 I_OBJ(OTYPE_LRG_HOLE,	 -2,  0,          0, -L_RL_BIN, &si_black, 1),
 I_OBJ(OTYPE_RED_STAR,	 38,  0,          0,  R_RL_BIN, &si_red,   2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_RL_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_RL_BIN, &si_ship2, 6),
},
		yy_binary[] = {
 I_OBJ(OTYPE_YELLOW_STAR, 20,  0,          0,  Y_YY_BIN, &si_yellow,2),
 I_OBJ(OTYPE_YELLOW_STAR,-20,  0,          0, -Y_YY_BIN, &si_yellow,3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_YY_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_YY_BIN, &si_ship2, 6),
},
		yb_binary[] = {
 I_OBJ(OTYPE_BLUE_STAR,	-13,  0,          0, -B_YB_BIN, &si_blue,  1),
 I_OBJ(OTYPE_YELLOW_STAR,27,  0,          0,  Y_YB_BIN, &si_yellow,2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_YB_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_YB_BIN, &si_ship2, 6),
},
		ys_binary[] = {
 I_OBJ(OTYPE_SM_HOLE,	-13,  0,          0, -S_YS_BIN, &si_black, 1),
 I_OBJ(OTYPE_YELLOW_STAR,27,  0,          0,  Y_YS_BIN, &si_yellow,2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_YS_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_YS_BIN, &si_ship2, 6),
},
		ym_binary[] = {
 I_OBJ(OTYPE_MED_HOLE,	 -8,  0,          0, -M_YM_BIN, &si_black, 1),
 I_OBJ(OTYPE_YELLOW_STAR,32,  0,          0,  Y_YM_BIN, &si_yellow,2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_YM_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_YM_BIN, &si_ship2, 6),
},
		yl_binary[] = {
 I_OBJ(OTYPE_LRG_HOLE,	 -4,  0,          0, -L_YL_BIN, &si_black, 1),
 I_OBJ(OTYPE_YELLOW_STAR,36,  0,          0,  Y_YL_BIN, &si_yellow,2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_YL_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_YL_BIN, &si_ship2, 6),
},
		bb_binary[] = {
 I_OBJ(OTYPE_BLUE_STAR,	 20,  0,          0,  B_BB_BIN, &si_blue,  2),
 I_OBJ(OTYPE_BLUE_STAR,	-20,  0,          0, -B_BB_BIN, &si_blue,  3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_BB_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_BB_BIN, &si_ship2, 6),
},
		bs_binary[] = {
 I_OBJ(OTYPE_SM_HOLE,	 20,  0,          0,  S_BS_BIN, &si_black, 1),
 I_OBJ(OTYPE_BLUE_STAR,	-20,  0,          0, -B_BS_BIN, &si_blue,  2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_BS_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_BS_BIN, &si_ship2, 6),
},
		bm_binary[] = {
 I_OBJ(OTYPE_MED_HOLE,	-13,  0,          0, -M_BM_BIN, &si_black, 1),
 I_OBJ(OTYPE_BLUE_STAR,	 27,  0,          0,  B_BM_BIN, &si_blue,  2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_BM_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_BM_BIN, &si_ship2, 6),
},
		bl_binary[] = {
 I_OBJ(OTYPE_LRG_HOLE,	 -8,  0,          0, -L_BL_BIN, &si_black, 1),
 I_OBJ(OTYPE_BLUE_STAR,	 32,  0,          0,  B_BL_BIN, &si_blue,  2),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_BL_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_BL_BIN, &si_ship2, 6),
},
		ss_binary[] = {
 I_OBJ(OTYPE_SM_HOLE,	 20,  0,          0,  S_SS_BIN, &si_black, 2),
 I_OBJ(OTYPE_SM_HOLE,	-20,  0,          0, -S_SS_BIN, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_SS_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_SS_BIN, &si_ship2, 6),
},
		sm_binary[] = {
 I_OBJ(OTYPE_MED_HOLE,	-13,  0,          0, -M_SM_BIN, &si_black, 2),
 I_OBJ(OTYPE_SM_HOLE,	 27,  0,          0,  S_SM_BIN, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_SM_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_SM_BIN, &si_ship2, 6),
},
		sl_binary[] = {
 I_OBJ(OTYPE_LRG_HOLE,	 -8,  0,          0, -L_SL_BIN, &si_black, 2),
 I_OBJ(OTYPE_SM_HOLE,	 32,  0,          0,  S_SL_BIN, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_SL_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_SL_BIN, &si_ship2, 6),
},
		mm_binary[] = {
 I_OBJ(OTYPE_MED_HOLE,	 20,  0,          0,  M_MM_BIN, &si_black, 2),
 I_OBJ(OTYPE_MED_HOLE,	-20,  0,          0, -M_MM_BIN, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_MM_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_MM_BIN, &si_ship2, 6),
},
		ml_binary[] = {
 I_OBJ(OTYPE_LRG_HOLE,	-13,  0,          0, -L_ML_BIN, &si_black, 2),
 I_OBJ(OTYPE_MED_HOLE,	 27,  0,          0,  M_ML_BIN, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_ML_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_ML_BIN, &si_ship2, 6),
},
		ll_binary[] = {
 I_OBJ(OTYPE_LRG_HOLE,	 20,  0,          0,  L_LL_BIN, &si_black, 2),
 I_OBJ(OTYPE_LRG_HOLE,	-20,  0,          0, -L_LL_BIN, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_LL_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_LL_BIN, &si_ship2, 6),
},
		ryy_trinary[] = {
 I_OBJ(OTYPE_RED_STAR,	  0, 48,R_RYY_TRI,           0, &si_red,   1),
 I_OBJ(OTYPE_YELLOW_STAR,10,-12,-Y_RYY_TRI_X, Y_RYY_TRI_Y,&si_yellow,2),
 I_OBJ(OTYPE_YELLOW_STAR,-10,-12,-Y_RYY_TRI_X,-Y_RYY_TRI_Y,&si_yellow,3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0, s_RYY_TRI, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,-s_RYY_TRI, &si_ship2, 6),
},
		ybb_trinary[] = {
 I_OBJ(OTYPE_YELLOW_STAR, 0, 48,Y_YBB_TRI,           0, &si_yellow,1),
 I_OBJ(OTYPE_BLUE_STAR,	 10,-12,-B_YBB_TRI_X, B_YBB_TRI_Y,&si_blue,2),
 I_OBJ(OTYPE_BLUE_STAR,	-10,-12,-B_YBB_TRI_X,-B_YBB_TRI_Y,&si_blue,3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0, s_YBB_TRI, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,-s_YBB_TRI, &si_ship2, 6),
},
		yss_trinary[] = {
 I_OBJ(OTYPE_YELLOW_STAR, 0, 48,Y_YSS_TRI,           0, &si_yellow, 1),
 I_OBJ(OTYPE_SM_HOLE,	 10,-12,-S_YSS_TRI_X, S_YSS_TRI_Y,&si_black,2),
 I_OBJ(OTYPE_SM_HOLE,	-10,-12,-S_YSS_TRI_X,-S_YSS_TRI_Y,&si_black,3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0, s_YSS_TRI, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,-s_YSS_TRI, &si_ship2, 6),
},
		bmm_trinary[] = {
 I_OBJ(OTYPE_BLUE_STAR,	  0, 48,B_BMM_TRI,           0, &si_blue,  1),
 I_OBJ(OTYPE_MED_HOLE,	 10,-12,-M_BMM_TRI_X, M_BMM_TRI_Y, &si_black,2),
 I_OBJ(OTYPE_MED_HOLE,	-10,-12,-M_BMM_TRI_X,-M_BMM_TRI_Y, &si_black,3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0, s_BMM_TRI, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,-s_BMM_TRI, &si_ship2, 6),
},
		smm_trinary[] = {
 I_OBJ(OTYPE_SM_HOLE,	  0, 48,B_BMM_TRI,           0, &si_black, 1),
 I_OBJ(OTYPE_MED_HOLE,	 10,-12,-M_BMM_TRI_X, M_BMM_TRI_Y, &si_black,2),
 I_OBJ(OTYPE_MED_HOLE,	-10,-12,-M_BMM_TRI_X,-M_BMM_TRI_Y, &si_black,3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0, s_BMM_TRI, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,-s_BMM_TRI, &si_ship2, 6),
},
		mll_trinary[] = {
 I_OBJ(OTYPE_MED_HOLE,	  0, 48,M_MLL_TRI,           0, &si_black, 1),
 I_OBJ(OTYPE_LRG_HOLE,	 10,-12,-L_MLL_TRI_X, L_MLL_TRI_Y, &si_black,2),
 I_OBJ(OTYPE_LRG_HOLE,	-10,-12,-L_MLL_TRI_X,-L_MLL_TRI_Y, &si_black,3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0, s_MLL_TRI, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,-s_MLL_TRI, &si_ship2, 6),
},
		black_elipse[] = {
 I_OBJ(OTYPE_SM_HOLE,	  0, 60,   S_EG_BIN,         0, &si_black, 2),
 I_OBJ(OTYPE_SM_HOLE,	  0,-60,  -S_EG_BIN,         0, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_EG_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_EG_BIN, &si_ship2, 6),
},
		wide_red[] = {
 I_OBJ(OTYPE_RED_STAR,	 70,  0,          0,  R_WR_BIN, &si_red,   2),
 I_OBJ(OTYPE_RED_STAR,	-70,  0,          0, -R_WR_BIN, &si_red,   3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_WR_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_WR_BIN, &si_ship2, 6),
},
		wide_yellow[] = {
 I_OBJ(OTYPE_YELLOW_STAR,70,  0,          0,  Y_WY_BIN, &si_yellow,2),
 I_OBJ(OTYPE_YELLOW_STAR,-70,  0,          0, -Y_WY_BIN, &si_yellow,3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_WY_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_WY_BIN, &si_ship2, 6),
},
		wide_blue[] = {
 I_OBJ(OTYPE_BLUE_STAR,	 70,  0,          0,  B_WB_BIN, &si_blue,  2),
 I_OBJ(OTYPE_BLUE_STAR,	-70,  0,          0, -B_WB_BIN, &si_blue,  3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_WB_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_WB_BIN, &si_ship2, 6),
},
		wide_small[] = {
 I_OBJ(OTYPE_SM_HOLE,	 70,  0,          0,  S_WS_BIN, &si_black, 2),
 I_OBJ(OTYPE_SM_HOLE,	-70,  0,          0, -S_WS_BIN, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_WS_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_WS_BIN, &si_ship2, 6),
},
		wide_med[] = {
 I_OBJ(OTYPE_MED_HOLE,	 70,  0,          0,  M_WM_BIN, &si_black, 2),
 I_OBJ(OTYPE_MED_HOLE,	-70,  0,          0, -M_WM_BIN, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_WM_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_WM_BIN, &si_ship2, 6),
},
		wide_large[] = {
 I_OBJ(OTYPE_LRG_HOLE,	 70,  0,          0,  L_WL_BIN, &si_black, 2),
 I_OBJ(OTYPE_LRG_HOLE,	-70,  0,          0, -L_WL_BIN, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,  s_WL_BIN, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0, -s_WL_BIN, &si_ship2, 6),
},
		ast_swarm[] = {
 I_OBJ(OTYPE_ASTEROID,	-20,-20,          0,         0, &si_asteroid, 0),
 I_OBJ(OTYPE_ASTEROID,	-20, 20,          0,         0, &si_ast_1, 1),
 I_OBJ(OTYPE_ASTEROID,	 20,-20,          0,         0, &si_ast_2, 2),
 I_OBJ(OTYPE_ASTEROID,	 20, 20,          0,         0, &si_ast_3, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,         0, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,         0, &si_ship2, 6),
},
		r_quad[] = {
 I_OBJ(OTYPE_RED_STAR,	 20, 70, R_R_QUAD_X, R_R_QUAD_Y, &si_red,   0),
 I_OBJ(OTYPE_RED_STAR,	-20, 70, R_R_QUAD_X,-R_R_QUAD_Y, &si_red,   1),
 I_OBJ(OTYPE_RED_STAR,	 20,-70,-R_R_QUAD_X, R_R_QUAD_Y, &si_red,   2),
 I_OBJ(OTYPE_RED_STAR,	-20,-70,-R_R_QUAD_X,-R_R_QUAD_Y, &si_red,   3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_R_QUAD, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_R_QUAD, &si_ship2, 6),
},
		y_quad[] = {
 I_OBJ(OTYPE_YELLOW_STAR, 20, 70, Y_Y_QUAD_X, Y_Y_QUAD_Y, &si_yellow, 0),
 I_OBJ(OTYPE_YELLOW_STAR,-20, 70, Y_Y_QUAD_X,-Y_Y_QUAD_Y, &si_yellow, 1),
 I_OBJ(OTYPE_YELLOW_STAR, 20,-70,-Y_Y_QUAD_X, Y_Y_QUAD_Y, &si_yellow, 2),
 I_OBJ(OTYPE_YELLOW_STAR,-20,-70,-Y_Y_QUAD_X,-Y_Y_QUAD_Y, &si_yellow, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_Y_QUAD, &si_ship1,  4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_Y_QUAD, &si_ship2,  6),
},
		b_quad[] = {
 I_OBJ(OTYPE_BLUE_STAR,	 20, 70, B_B_QUAD_X, B_B_QUAD_Y, &si_blue,  0),
 I_OBJ(OTYPE_BLUE_STAR,	-20, 70, B_B_QUAD_X,-B_B_QUAD_Y, &si_blue,  1),
 I_OBJ(OTYPE_BLUE_STAR,	 20,-70,-B_B_QUAD_X, B_B_QUAD_Y, &si_blue,  2),
 I_OBJ(OTYPE_BLUE_STAR,	-20,-70,-B_B_QUAD_X,-B_B_QUAD_Y, &si_blue,  3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_B_QUAD, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_B_QUAD, &si_ship2, 6),
},
		s_quad[] = {
 I_OBJ(OTYPE_SM_HOLE,	 20, 70, S_S_QUAD_X, S_S_QUAD_Y, &si_black, 0),
 I_OBJ(OTYPE_SM_HOLE,	-20, 70, S_S_QUAD_X,-S_S_QUAD_Y, &si_black, 1),
 I_OBJ(OTYPE_SM_HOLE,	 20,-70,-S_S_QUAD_X, S_S_QUAD_Y, &si_black, 2),
 I_OBJ(OTYPE_SM_HOLE,	-20,-70,-S_S_QUAD_X,-S_S_QUAD_Y, &si_black, 3),
 I_OBJ(OTYPE_SHIP,	 91,  0,          0,   s_S_QUAD, &si_ship1, 4),
 I_OBJ(OTYPE_SHIP,	-91,  0,          0,  -s_S_QUAD, &si_ship2, 6),
};

S_SYSTEM	pri0_sys[] = {
	I_SSYS(nothing),
},
		pri0a_sys[] = {
	I_SSYS(only_asteroid),
},
		pri1_sys[] = {
	I_SSYS(r_single),
	I_SSYS(y_single),
	I_SSYS(b_single),
	I_SSYS(s_single),
	I_SSYS(m_single),
	I_SSYS(l_single),
},
		pri1a_sys[] = {
	I_SSYS(r_asteroid),
	I_SSYS(y_asteroid),
	I_SSYS(b_asteroid),
	I_SSYS(s_asteroid),
	I_SSYS(m_asteroid),
	I_SSYS(l_asteroid),
},
		pri2_sys[] = {
	I_SSYS(rr_binary),
	I_SSYS(ry_binary),
	I_SSYS(rb_binary),
	I_SSYS(rs_binary),
	I_SSYS(rm_binary),
	I_SSYS(rl_binary),
	I_SSYS(yy_binary),
	I_SSYS(yb_binary),
	I_SSYS(ys_binary),
	I_SSYS(ym_binary),
	I_SSYS(yl_binary),
	I_SSYS(bb_binary),
	I_SSYS(bs_binary),
	I_SSYS(bm_binary),
	I_SSYS(bl_binary),
	I_SSYS(ss_binary),
	I_SSYS(sm_binary),
	I_SSYS(sl_binary),
	I_SSYS(mm_binary),
	I_SSYS(ml_binary),
	I_SSYS(ll_binary),
},
		pri3_sys[] = {
	I_SSYS(ryy_trinary),
	I_SSYS(ybb_trinary),
	I_SSYS(yss_trinary),
	I_SSYS(bmm_trinary),
	I_SSYS(smm_trinary),
	I_SSYS(mll_trinary),
},
		pri4_sys[] = {
	I_SSYS(r_quad),
	I_SSYS(y_quad),
	I_SSYS(b_quad),
	I_SSYS(s_quad),
},
		misc_sys[] = {
	I_SSYS(black_elipse),
	I_SSYS(wide_red),
	I_SSYS(wide_yellow),
	I_SSYS(wide_blue),
	I_SSYS(wide_small),
	I_SSYS(wide_med),
	I_SSYS(wide_large),
	I_SSYS(ast_swarm),
};

#define I_SSL(array, item_num, ast)     { array, NEL(array), item_num, ast }

SS_LIST 	ss_list[] = {
	I_SSL(pri0_sys,  0, 0),
	I_SSL(pri0a_sys, 0, 1),
	I_SSL(pri1_sys,  1, 0),
	I_SSL(pri1a_sys, 1, 1),
	I_SSL(pri2_sys,  2, 0),
	I_SSL(pri3_sys,  3, 0),
	I_SSL(pri4_sys,  4, 0),
	I_SSL(misc_sys,  5, 0),
};
uint		num_ss_list = NEL(ss_list);
uint		max_systems = NEL(pri0_sys) + NEL(pri0a_sys) +
			NEL(pri1_sys) + NEL(pri1a_sys) + NEL(pri2_sys) +
			NEL(pri3_sys) + NEL(pri4_sys) + NEL(misc_sys);

OBJ	obj[NUM_OBJ];

