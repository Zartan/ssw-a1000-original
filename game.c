/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * game.c -- game port routines for sprite test program
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

#ident "$Id: game.c 1.2 1997/01/19 07:04:58 jamesc Exp jamesc $"

#include "ssw.h"
#include <hardware/custom.h>
#include <hardware/cia.h>
#include <exec/devices.h>
#include <devices/gameport.h>
#include <devices/inputevent.h>
#include <devices/input.h>
#include <exec/execbase.h>
#include <resources/potgo.h>
#include <proto/potgo.h>
#include <proto/timer.h>

#ifndef NULL
#define NULL		(0)
#endif

/*
 * Lie, cheat, and steal POTGO bits.
 *
 * Intuition (or the input.device) has already reserved the joyport unit
 * 0's POTGO bits, so we will just allocate those for unit 1.
 *
 * Once we have deactivated the mouse using input.device's IND_SETMTYPE
 * command, we will simply take over the POTGO bits for unit 0 as well.
 *
 * When analog joystick support is added, reserve A001L to control the
 * START bit and reserve the right to START unit 1's analog inputs.
 * This may not work, depending on the mode that unit 0's bits were
 * allocated in, but try it anyway.
 */
#define POTGO_BITS	(0xF000L)	/* we want all unit 1 POTGO bits */


Static struct MsgPort		*gameport;
Static struct IOStdReq		*gamemsg;
Static struct InputEvent	gamedata;
Static int			gameopen;

joyport_t			joyport[4];
Static joyport_t		lastjoy[2];

struct Library			*PotgoBase;
Static ULONG			potgo_bits;

Static struct IOStdReq		*inp_req;

Static uchar			inp_open = 0;
Static BYTE			mouse_ctrlr_type = GPCT_MOUSE;

Static char			gameport_device[] = "gameport.device";
Static char			potgoname[] = POTGONAME;

extern volatile struct Custom far custom;
extern volatile struct CIA far	ciaa;

extern void			wait1(void);


/*
 * create_req -- create a port, and return an IORequest or other for it
 */

struct IORequest *
create_req(char *name, long size)
{
	struct MsgPort		*port;
	struct IORequest	*req;

	if ((port = CreatePort(NULL, 0)) == NULL) {
		err("Can't CreatePort for %s!", name);
		goto wait4it;
	}
	if ((req = CreateExtIO(port, size)) == NULL) {
		DeletePort(port);
		err("Can't CreateExtIO for %s!\nSize = %ld", name, size);
wait4it:
		wait1();
		die(9);
	}

	return (req);
}


/*
 * delete_req -- close a port created by create_port and free its request
 */

void
delete_req(struct IORequest *req)
{
	struct MsgPort	*port;

	if (req) {
		port = req->io_Message.mn_ReplyPort;
		DeleteExtIO(req);
		if (port)
			DeletePort(port);
	}
}


/*
 * open_dev -- open a device, returns IORequest or other struct in pptr
 */

int
open_dev(char *name, long unit, struct IORequest **pptr, long size)
{
	struct IORequest	*req;
	int			errnum;

	req = create_req(name, size);
	if (errnum = OpenDevice(name, unit, req, 0L)) {
		delete_req(req);
		err("Can't OpenDevice!\n%s, unit # %ld\nError = %ld",
			name, unit, errnum);
		wait1();
		die(9);
	}
	*pptr = req;			/* return message pointer */
	return (MDEV_OPEN);
}


/*
 * close_dev -- close a device opened by open_dev
 */

void
close_dev(int flag, struct IORequest *req)
{
	if (flag & MDEV_OPEN)
		CloseDevice(req);

	delete_req(req);
}


/*
 * counter2joy -- turns the hardware mouse counters into joystick bits
 *	 format: 8 7 6 5 4 3 2 1 0
 *		 K 2 k 3 F L U R D
 */

#define COUNTER2JOY(v) (v = (v & 0x3) | (((v >> 8) & 0x3) << 2), v ^= (v & 0xa) >> 1)

/*
 * do_joy_ports -- get the state of the joystick ports from the hardware
 */

void
do_joy_ports(void)
{
	register ushort 	val;
	register joyport_t	joy_val_0, joy_val_1;

	val = custom.joy0dat;
	joy_val_0 = COUNTER2JOY(val);
	val = custom.joy1dat;
	joy_val_1 = COUNTER2JOY(val);
	val = (~ciaa.ciapra) >> (CIAB_GAMEPORT0 - JP_FIRE_SHIFT);
	joy_val_0 |= val & JP_FIRE;
	joy_val_1 |= (val >> 1) & JP_FIRE;
	/*
	 * potinp:
	 *   15   14   13   12	 11   10    9	 8  7 6 5 4 3 2 1 0
	 *  OE14|P2#9|OE12|P2#5|OE10|P1#9|OE08|P1#5|X|X|X|X|X|X|X|S
	 *
	 * Note that the JP_BUT[23] values are defined to match the layout
	 * in potinp after being shifted by 8 - JP_BUT3_SHIFT.
	 *
	 *  K 2 k 3 F L U R D
	 */
	val = ~custom.potinp >> (8 - JP_BUT3_SHIFT);
	joy_val_0 |= val & JP_BUT23;
	joy_val_1 |= (val >> 4) & JP_BUT23;
	/*
	 * Debounce the joysticks -- only update if joy_val == lastjoy.
	 */
	if (val = joy_val_0 ^ lastjoy[0])
		joyport[0] = joyport[0] & ~val | joy_val_0 & val;
	if (val = joy_val_1 ^ lastjoy[1])
		joyport[1] = joyport[1] & ~val | joy_val_1 & val;
	lastjoy[0] = joy_val_0;
	lastjoy[1] = joy_val_1;
}

/*
 * set_controller_type -- tell the gameport device what is hooked to it
 */

Static int
set_controller_type(long type)
{
	BYTE	data;

	gamemsg->io_Command = GPD_SETCTYPE;
	gamemsg->io_Length  = 1;
	gamemsg->io_Flags   = IOF_QUICK;
	gamemsg->io_Data    = (APTR)&data;
	data = type;
	return ((int)DoIO(TO_IOR(gamemsg)));
}

/*
 * flush_joystick_buffer -- flush any pre-queued events
 */

Static void
flush_joystick_buffer(void)
{
	gamemsg->io_Command = CMD_CLEAR;
	gamemsg->io_Flags   = IOF_QUICK;
	(void) DoIO(TO_IOR(gamemsg));
}


/*
 * mouse_ctl -- turn on or off mouse control
 */

void
mouse_ctl(int mouse_on)
{
	int	error;
	BYTE	ctype;

	if (!(inp_open & MDEV_OPEN))
		return;
	if (mouse_on) {
		if (!(inp_open & MDEV_SET))
			return; 			/* already done */
		inp_open &= ~MDEV_SET;
		ctype = mouse_ctrlr_type;
	}
	else {
		if (inp_open & MDEV_SET)
			return; 			/* already done */
		inp_open |= MDEV_SET;
		ctype = GPCT_ALLOCATED;
	}
	inp_req->io_Length = 1;
	inp_req->io_Command = IND_SETMTYPE;
	inp_req->io_Data = (APTR)&ctype;
	if (error = DoIO(TO_IOR(inp_req))) {
		err("IND_SETMTYPE failed!\nError=%ld", error);
		wait1();
		die(1);
	}
}


/*
 * init_game_port -- open/init game port # 1
 *
 * (Also temporarily open # 0 to find mouse controller type.)
 */

void
init_game_port(void)
{
	register int	ret;
	BYTE		data;

	gameopen = open_dev(gameport_device, 0L,
			(struct IORequest **)&gamemsg, sizeof(*gamemsg));

	/* check the game port controller type */
	gamemsg->io_Command = GPD_ASKCTYPE;
	gamemsg->io_Length  = 1;
	gamemsg->io_Flags   = IOF_QUICK;
	gamemsg->io_Data    = (APTR)&data;
	data = 0;
	Forbid();
	if (ret = DoIO(TO_IOR(gamemsg))) {
		Permit();
		err("GPD_ASKCTYPE error = %ld", ret);
		goto wait4it;
	}
	Permit();

	mouse_ctrlr_type = data;			/* got type	*/
	close_dev(gameopen, TO_IOR(gamemsg));		/* close port 0 */

	gameopen = open_dev(gameport_device, 1L,
			(struct IORequest **)&gamemsg, sizeof(*gamemsg));

	/* check the game port controller type */
	gamemsg->io_Command = GPD_ASKCTYPE;
	gamemsg->io_Length  = 1;
	gamemsg->io_Flags   = IOF_QUICK;
	gamemsg->io_Data    = (APTR)&data;
	data = 0;
	Forbid();
	if (ret = DoIO(TO_IOR(gamemsg))) {
		Permit();
		err("GPD_ASKCTYPE error = %ld", ret);
		goto wait4it;
	}
	if (data != GPCT_NOCONTROLLER) {
		Permit();
		err("Gameport unit 1 already in use! type=%ld", (int)data);
wait4it:
		wait1();
		die(9);
	}

	if (set_controller_type(GPCT_ALLOCATED)) {
		err("set_controller_type: error=%ld", ret);
		goto wait4it;
	}
	gameopen |= MDEV_SET;
	Permit();

	/* don't set joyport trigger conditions and never get events??? */

	flush_joystick_buffer();

	/* open input.device so that we can change mouse controller type */

	inp_open = open_dev("input.device", 0L,
			(struct IORequest **)&inp_req, sizeof(*inp_req));

	/* open potgo.resource and reserve all pertinent bits */

	PotgoBase = (struct Library *) OpenResource(potgoname);
	if (!PotgoBase) {
		err("Can't OpenResource %s!", potgoname);
		goto wait4it;
	}

	potgo_bits = AllocPotBits(POTGO_BITS);
	if (potgo_bits != POTGO_BITS) {
		FreePotBits(potgo_bits);
		err("Can't alloc all POTGO bits!\nOnly got 0x%lx!",
			potgo_bits);
		goto wait4it;
	}

	/* init custom hardware */

	/*
	 * Use WritePotgo first so that its saved byte is initialized.
	 */
	WritePotgo(0xFFFFFFFFL, potgo_bits);

	custom.potgo = 0xff00;		/* set all bits to output, data=1 */
}


/*
 * end_game_port -- close/free game port resources
 */

void
end_game_port(void)
{
	long	ret;

	if (inp_open)
		mouse_ctl(1);
	close_dev(inp_open, TO_IOR(inp_req));

	if (gameopen & MDEV_SET) {
		if (!CheckIO(TO_IOR(gamemsg))) {
			AbortIO(TO_IOR(gamemsg));
			WaitIO(TO_IOR(gamemsg));
		}
		(void) GetMsg(gamemsg->io_Message.mn_ReplyPort);
		flush_joystick_buffer();
		if (ret = set_controller_type(GPCT_NOCONTROLLER)) {
			err("set_controller_type: error=%ld", ret);
			wait1();
		}
	}
	close_dev(gameopen, TO_IOR(gamemsg));

	if (potgo_bits)
		FreePotBits(potgo_bits);
}

