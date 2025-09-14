/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * audio.c -- control and manage sounds
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

#ident "$Id: audio.c 1.2 1997/01/19 07:04:58 jamesc Exp jamesc $"

#include "ssw.h"
#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>
#include <devices/audio.h>
#include <graphics/gfxbase.h>
#include <clib/exec_protos.h>

#ifndef NULL
#define NULL		(0)
#endif

#define NUM_CHANNELS	4
#define NUM_AUDIO_POOL	5

#define TO_IOA(p)	((struct IOAudio *)(p))

typedef union audio_units_u {
	struct Unit	*s;
	ULONG		l;
} audio_units_t;


static long		audio_open;
static audio_units_t	audio_units;
static struct IOAudio	*audio_cmd;		/* always used IOB_QUICK */
static struct MsgPort	*audio_port, *audio_pool;
static struct IOAudio	*audio_lock[NUM_CHANNELS];
static UBYTE		audio_ch[NUM_CHANNELS];
static UBYTE		audio_unit_alloc[sizeof(long)] = { 0xf };

extern struct GfxBase	*GfxBase;

extern void __stdargs	BeginIO(struct IORequest *);
extern struct IORequest __stdargs *CreateExtIO(struct MsgPort *, long);
#if 0
extern struct MsgPort __stdargs *CreatePort(char *, long);
#endif
extern void __stdargs	DeleteExtIO(struct IORequest *);
extern void __stdargs	DeletePort(struct MsgPort *);
extern void		wait1(void);


/*
 * init_audio -- start up the audio device, alloc structs, etc
 */

ULONG
init_audio(void)
{
	register struct IOAudio *ioap;
	register SOUND		*sp;
	register int		n;
	register long		clock;
	static char		audioname[] = AUDIONAME;

	ioap = TO_IOA(create_req("audio_port", sizeof(*ioap)));
	audio_cmd = ioap;
	audio_port = ioap->ioa_Request.io_Message.mn_ReplyPort;

	/* open the audio device */

	ioap->ioa_Request.io_Command = ADCMD_ALLOCATE;
	ioap->ioa_Request.io_Flags = ADIOF_NOWAIT;
	ioap->ioa_Request.io_Message.mn_Node.ln_Pri = -50;
	ioap->ioa_Data = audio_unit_alloc;
	ioap->ioa_Length = 1;
	ioap->ioa_AllocKey = 0; 		/* new open */
	/*
	 * try to open 8 times before giving up
	 */
	for (n = 7; --n >= 0; ) {
		if (!OpenDevice(audioname, 0L, TO_IOR(ioap), 0L))
			goto opened;
		if (n > 1)
			delay(1);
	}
	if (n = OpenDevice(audioname, 0L, TO_IOR(ioap), 0L)) {
		if (n == ADIOERR_ALLOCFAILED)
			err("Can't alloc all 4 audio channels!");
		else
			err("Can't open %s!\nError = %ld", audioname, n);
		goto wait4it;
	}
opened:
	audio_open = MDEV_OPEN; 		/* open flag	*/
	audio_units.l = 0x0f;			/* open channels */

	if ((audio_pool = CreatePort(NULL, 0)) == NULL) {
		err("audio_pool create failed!");
		goto wait4it;
	}
	audio_pool->mp_Flags = (audio_pool->mp_Flags & ~PF_ACTION)|PA_IGNORE;

	/* do ADCMD_LOCK commands to learn when a channel is stolen */

	for (n = NUM_CHANNELS; --n >= 0; ) {
		ioap = TO_IOA(CreateExtIO(audio_port, sizeof(*ioap)));
		if (ioap == NULL) {
			err("Can't alloc audio_lock!");
wait4it:
			wait1();
			die(1);
		}
		audio_lock[n] = ioap;
		*ioap = *audio_cmd;		/* dup audio_cmd */
		ioap->ioa_Request.io_Command = ADCMD_LOCK;
		ioap->ioa_Request.io_Flags = 0;
		audio_ch[n] = 1L << n;
		ioap->ioa_Request.io_Unit = (struct Unit *) audio_ch[n];
		ioap->ioa_Data = &audio_ch[n];
		ioap->ioa_Length = 1;
		BeginIO(TO_IOR(ioap));
	}

	/* loop and alloc other IOAudio structs */

	for (n = NUM_AUDIO_POOL; --n >= 0; ) {
		ioap = TO_IOA(CreateExtIO(audio_port, sizeof(*ioap)));
		if (ioap == NULL) {
			err("Can't alloc IOAudio!");
			goto wait4it;
		}
		*ioap = *audio_cmd;		/* dup audio_cmd */
			/* set reply pointer to audio_pool and reply */
		ioap->ioa_Request.io_Message.mn_ReplyPort = audio_pool;
		ReplyMsg(TO_MSG(ioap));
	}

	/* reorder initialized SOUND structs here */

	/* convert SOUND struct period to current clock division value */

	clock = (GfxBase->DisplayFlags & PAL) ?
		3546895L << 8 : 3579545L << 8;

	for (sp = sounds, n = NUM_SOUNDS; --n >= 0; sp++)
		sp->period = (clock + (sp->period << 7)) / sp->period >> 8;

	return ((ULONG)1L << audio_port->mp_SigBit);	/* tell the world */
}


/*
 * flush_sound -- flush the units specified by the sound structure, or all
 *		if sp is NULL
 */

void
flush_sound(SOUND *sp)
{
	register struct IOAudio *ioap;

	if ((ioap = audio_cmd) == NULL)
		return;

	ioap->ioa_Request.io_Command = CMD_FLUSH;
	ioap->ioa_Request.io_Flags = IOF_QUICK;
	ioap->ioa_Request.io_Unit =
	  (sp ? (struct Unit *)sp->unit : audio_units.s);
	BeginIO(TO_IOR(ioap));
}


/*
 * end_audio -- cleanly close the audio device
 */

void
end_audio(void)
{
	register struct IOAudio *ioap;
	register int		n;

	if ((ioap = audio_cmd) == NULL)
		return; 			/* nothing else alloced */

	if (audio_open) {
		/* flush all channels, then free them */
		flush_sound(NULL);
		ioap->ioa_Request.io_Unit = audio_units.s;
		CloseDevice(TO_IOR(ioap));	/* does ADCMD_FREE */
	}
	/* this should have been done by the ADCMD_FREE -- do it anyway */
	Disable();
	for (n = NUM_CHANNELS; --n >= 0; ) {
		if ((ioap = audio_lock[n]) == NULL)
			continue;
		if (!CheckIO(TO_IOR(ioap)))
			AbortIO(TO_IOR(ioap));
	}
	Enable();
delay(2); /* FIX THIS: really need to wait for everything to be returned? */
	if (audio_pool) {
		while (ioap = TO_IOA(GetMsg(audio_pool)))
			DeleteExtIO(TO_IOR(ioap));
		DeletePort(audio_pool);
	}
	if (audio_port) {
		while (ioap = TO_IOA(GetMsg(audio_port)))
			DeleteExtIO(TO_IOR(ioap));
		DeletePort(audio_port);
	}
	DeleteExtIO(TO_IOR(audio_cmd));
}


/*
 * free_sounds -- free the sound samples
 */

void
free_sounds(void)
{
	register int	n;
	register int	i;
	register SOUND	*sp;
	register SOUND	*xp;
	register uchar	*bp;

	for (sp = sounds, n = NUM_SOUNDS; --n >= 0; sp++) {
		if ((bp = sp->data) == NULL)
			continue;

		sp->data = NULL;
		FreeMem(bp, sp->length);		/* free it */

		/*
		 * Now check for multiply used sound samples.
		 */
#if 0
		for (xp = sp, i = n; --i >= 0; ) {
			++xp;
#else
		for (xp = sounds, i = NUM_SOUNDS; --i >= 0; xp++) {
#endif /* 0 */
			if (xp->data == bp || xp->data == bp + 2)
				xp->data = NULL;
		}
	}
}


/*
 * process_audio -- get messages from audio_port
 */

void
process_audio(void)
{
	register struct IOAudio *ioap;
	register long	n;

	while (ioap = TO_IOA(GetMsg(audio_port))) {
		if (ioap == audio_cmd) {
			/* should never get here -- do nothing */
			err("process_audio: audio_cmd!?!");
		}
		else if (ioap->ioa_Request.io_Command == ADCMD_LOCK) {
			/* we've lost a channel -- free it, then try to
			 * reallocate it */
			n = (ulong)ioap->ioa_Request.io_Unit;
			audio_units.l &= ~n;
			ioap->ioa_Request.io_Command = ADCMD_FREE;
			ioap->ioa_Request.io_Flags = IOF_QUICK;
			BeginIO(TO_IOR(ioap));

			ioap->ioa_Request.io_Command = ADCMD_ALLOCATE;
			ioap->ioa_Request.io_Flags = 0;
			/* io_Data, etc are pre-set */
			BeginIO(TO_IOR(ioap));
#if 0
errx("Lost a audio channel!  Got:", audio_units.l);
#endif
		}
		else {
			/* we've got a channel back -- log, then lock it */
			audio_units.l |= (ulong) ioap->ioa_Request.io_Unit;
			ioap->ioa_Request.io_Command = ADCMD_LOCK;
			ioap->ioa_Request.io_Flags = 0;
			BeginIO(TO_IOR(ioap));
#if 0
errx("Got a audio channel:", audio_units.l);
#endif
		}
	}
}


/*
 * play_sound -- play the specified sound structure, flushing the channel
 */

void
play_sound(register SOUND *sp)
{
	register struct IOAudio *ioap;
	register ulong	unit;

	unit = sp->unit;
	ioap = audio_cmd;

	/*
	 * flush channel first
	 */
	ioap->ioa_Request.io_Command = CMD_FLUSH;
	ioap->ioa_Request.io_Flags = IOF_QUICK;
	ioap->ioa_Request.io_Unit = (struct Unit *)unit;
	BeginIO(TO_IOR(ioap));

	if ((ioap = TO_IOA(GetMsg(audio_pool))) == NULL) {
err("play_sound: audio_pool empty!");
		return; 			/* should never fail */
	}

	ioap->ioa_Request.io_Command = CMD_WRITE;
	ioap->ioa_Request.io_Flags = ADIOF_PERVOL;
	ioap->ioa_Request.io_Unit = (struct Unit *)unit;
	ioap->ioa_Data	 = sp->data;
	ioap->ioa_Length = sp->length;
	ioap->ioa_Period = sp->period;
	ioap->ioa_Volume = sp->volume;
	ioap->ioa_Cycles = sp->cycles;
	BeginIO(TO_IOR(ioap));
}


/*
 * no_flush_sound -- play the specified sound structure, without flushing
 *		unless all allowed channels carry duplicate sounds
 */

void
no_flush_sound(register SOUND *sp)
{
	register struct IOAudio *ioap;
	register ulong	unit;
	ulong	mych = 0;

	unit = sp->unit;
	ioap = audio_cmd;

	for (;;) {
		ioap->ioa_Request.io_Command = CMD_READ;
		ioap->ioa_Data = NULL;
		ioap->ioa_Request.io_Flags = IOF_QUICK;
		ioap->ioa_Request.io_Unit = (struct Unit *)unit;
		BeginIO(TO_IOR(ioap));

		if (ioap->ioa_Data == NULL) {
			unit = (ulong)ioap->ioa_Request.io_Unit;
			break;		/* found a free channel */
		}
		if (TO_IOA(ioap->ioa_Data)->ioa_Data == sp->data)
			mych = (ulong)ioap->ioa_Request.io_Unit;

		unit &= ~(ulong)ioap->ioa_Request.io_Unit;
		if (unit == 0) {
			if (!mych)
				return; 	/* all channels busy */
			unit = mych;
			/*
			 * duplicate sound -- flush channel first
			 */
			ioap->ioa_Request.io_Command = CMD_FLUSH;
			ioap->ioa_Request.io_Flags = IOF_QUICK;
			ioap->ioa_Request.io_Unit = (struct Unit *)unit;
			BeginIO(TO_IOR(ioap));
			break;
		}
	}

	if ((ioap = TO_IOA(GetMsg(audio_pool))) == NULL) {
err("no_flush: audio_pool empty!");
		return; 			/* should never fail */
	}

	ioap->ioa_Request.io_Command = CMD_WRITE;
	ioap->ioa_Request.io_Flags = ADIOF_PERVOL;
	ioap->ioa_Request.io_Unit = (struct Unit *)unit;
	ioap->ioa_Data	 = sp->data;
	ioap->ioa_Length = sp->length;
	ioap->ioa_Period = sp->period;
	ioap->ioa_Volume = sp->volume;
	ioap->ioa_Cycles = sp->cycles;
	BeginIO(TO_IOR(ioap));
}

