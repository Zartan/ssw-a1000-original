/*
 * Copyright 1993, 1995 James Cleverdon.  All rights reserved.
 *
 * iff8SVX.c -- Read a CAT of IFF 8SVX FORMs and load into SSW's SOUND
 *		structures.
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

#ident "$Id: iff8SVX.c 1.3 1997/01/19 07:21:44 jamesc Exp jamesc $"

#include "ssw.h"
#include <exec/exec.h>
#include <libraries/dos.h>
#include <libraries/iffparse.h>
#include <string.h>
#include <proto/dos.h>

/* #define DEBUG_8SVX  /* **/

#ifdef DEBUG_8SVX
#define ERR0(fmt)		err(fmt); wait1()
#define ERR1(fmt,a)		err((fmt), (a)); wait1()
#define ERR2(fmt,a,b)		err((fmt), (a), (b)); wait1()
#define ERR3(fmt,a,b,c)		err((fmt), (a), (b), (c)); wait1()
#define ERR4(fmt,a,b,c,d)	err((fmt), (a), (b), (c), (d)); wait1()
#define ERR5(fmt,a,b,c,d,e)	err((fmt), (a), (b), (c), (d), (e)); wait1()
#else
#define ERR0(fmt)
#define ERR1(fmt,a)
#define ERR2(fmt,a,b)
#define ERR3(fmt,a,b,c)
#define ERR4(fmt,a,b,c,d)
#define ERR5(fmt,a,b,c,d,e)
#endif /* DEBUG_8SVX */

Static char	*snd_name[] = { 	/* names in SOUND index order */
	"fire",		/* 0 */
	"explode",	/* 1 */
	"shield",	/* 2 */
	"torp_die",	/* 3 */
	"thrust",	/* 4 */
	"hyper_out",	/* 5 */
	"hyper_in",	/* 6 */
	"bounce",	/* 7 */
	"quad",		/* 8 */
};

typedef struct snd_param_s {
	uchar	cycles;
	uchar	unit;
	uchar	volume;
} snd_parm_t;

Static snd_parm_t	snd_parm[] = {
	{ 1, 2, 64 },	/* fire1	*/
	{ 1, 4, 64 },	/* explode1	*/
	{ 1, 2, 64 },	/* shield1	*/
	{ 1, 6, 16 },	/* torp_die1	*/
	{ 0, 4, 24 },	/* thrust1	*/
	{ 1, 4, 48 },	/* hyper_out1	*/
	{ 1, 4, 48 },	/* hyper_in1	*/
	{ 1, 2, 32 },	/* bounce1	*/
	{ 0, 4, 32 },	/* quad1	*/
	{ 1, 1, 64 },	/* fire2	*/
	{ 1, 8, 64 },	/* explode2	*/
	{ 1, 1, 64 },	/* shield2	*/
	{ 1, 9, 16 },	/* torp_die2	*/
	{ 0, 8, 24 },	/* thrust2	*/
	{ 1, 8, 48 },	/* hyper_out2	*/
	{ 1, 8, 48 },	/* hyper_in2	*/
	{ 1, 1, 32 },	/* bounce2	*/
	{ 0, 8, 32 },	/* quad2	*/
	{ 1, 2, 32 },	/* ping0	*/
	{ 1, 1, 32 },	/* ping1	*/
	{ 1, 2, 32 },	/* ping3	*/
	{ 1, 1, 32 },	/* ping4	*/
};

SOUND		sounds[NUM_SOUNDS];


extern void	wait1(void);


/*
 * Some standard IFF stuff that doesn't seem to come with SAS/C.
 */

typedef LONG	ID;		/* 4 chars in ' ' through '~'		*/

typedef struct _chunk_s {
	ID	ckID;		/* four byte ID field			*/
	LONG	ckSize; 	/* chunk size (not counting a pad byte) */
} Chunk;


/**** stolen from NewIFF/iffp/8SVX.h ****/

/*
 * 8SVX.H  Definitions for 8-bit sampled voice (VOX).  2/10/86
 *
 * By Jerry Morrison and Steve Hayes, Electronic Arts.
 * This software is in the public domain.
 *
 * This version for the Commodore-Amiga computer.
 *
 */

#define ID_8SVX 	MAKE_ID('8', 'S', 'V', 'X')
#define ID_VHDR 	MAKE_ID('V', 'H', 'D', 'R')
#define ID_BODY 	MAKE_ID('B', 'O', 'D', 'Y')

/* ---------- Voice8Header ------------- */

typedef LONG	Fixed;	/* A fixed-point value, 16 bits to the left of
			 * the point and 16 to the right. A Fixed is a
			 * number of 2**16ths, i.e. 65536ths.
			 */
#define Unity	0x10000L	/* Unity = Fixed 1.0 = maximum volume */

/* sCompression: Choice of compression algorithm applied to the samples. */
#define sCmpNone	0	/* not compressed */
#define sCmpFibDelta	1	/* Fibonacci-delta encoding (Appendix C) */
				/* Could be more kinds in the future. */

typedef struct _v8hdr_s {
	ULONG	oneShotHiSamples,	/* samples in high octave 1-shot */
		repeatHiSamples,	/* samples in high octave repeat */
		samplesPerHiCycle;	/* samples/cycle in high octave */
	UWORD	samplesPerSec;		/* data sampling rate		*/
	UBYTE	ctOctave,		/* # of octaves of waveforms	*/
		sCompression;		/* data compression technique used */
	Fixed	volume; 		/* playback nominal volume from 0 to
					 * Unity (full volume).  Map this
					 * value into the output hardware's
					 * dynamic range.
					 */
} Voice8Header;

/* ---------- BODY ------------*/
/* BODY chunk contains a BYTE[], array of audio data samples. */
/* (8-bit signed numbers, -128 through 127.) */

/**** end NewIFF/iffp/8SVX.h ****/

/*
 * My IFF definitions and typedefs.
 */

typedef struct iff_s {
	BPTR	file;		/* open file handle			*/
	Chunk	chunk;		/* scratch chunk used by everybody	*/
} iff_t;

typedef struct frame_s {
	struct frame_s *par;	/* parent context frame link		*/
	LONG	bytes;		/* available bytes in current context	*/
	ID	id;		/* chunk's ID (for debug)		*/
} frame_t;


#define ID_NAME 	MAKE_ID('N', 'A', 'M', 'E')

/*
 * Standard 8SVX functions.
 */

/*
 * DUnpack.c --- Fibonacci Delta decompression by Steve Hayes
 */

/*
 * Fibonacci delta encoding for sound data
 */
BYTE	codeToDelta[] = { -34,-21,-13,-8,-5,-3,-2,-1,0,1,2,3,5,8,13,21 };

/*
 * word_shift -- shift the data in a buffer over two bytes, destroying
 *		the first UWORD.
 */

Static void
word_shift(register UWORD *buf, register LONG n)
{
	while (--n > 0) {
		buf[0] = buf[1];
		++buf;
	}
}


/* Unpack Fibonacci-delta encoded data from n byte
 * source buffer into 2*(n-2) byte dest buffer.
 * Source buffer has a pad byte, an 8-bit initial
 * value, followed by n-2 bytes comprising 2*(n-2)
 * 4-bit encoded samples.
 *
 * Unpacks the data in place, so the original contents of buf are lost.
 * Assumes that i has already been calculated = 2 * (size - 2).
 */

Static void
FD_Unpack(register BYTE *buf, register LONG i)
{
	register BYTE	x;
	register BYTE	d;

	if (i <= 0) {
		return; 		/* no samples			*/
	}

	x = buf[1];			/* get initial value from buf[1] */

	word_shift((UWORD *)buf, i >> 1);

	while ((i -= 2) >= 0) {
		/* Decode a data nibble, high nibble then low nibble */
		d = buf[i >> 1];	/* get a pair of nibbles */
		x += codeToDelta[d >> 4];
		buf[i] = x;
		x += codeToDelta[d & 0xF];
		buf[i + 1] = x;
	}
}


/*
 * barf_iff -- close down and exit with error code
 */

Static void
barf_iff(iff_t *iffp)
{
	if (iffp->file) {
		Close(iffp->file);
		iffp->file = 0L;
	}
	die(0);
}


/*
 * adj_frames -- Verify the given bytes against the currently active
 *		context frames.  Returns even-padded byte value.
 */

Static LONG
adj_frames(register iff_t *iffp, register frame_t *fptr, register LONG bytes)
{
	uchar	pad1 = FALSE;

	if (bytes & 1) {
		++bytes;		/* always pad odd chunks by one */
		pad1 = TRUE;
	}

	while (fptr != NULL) {
		if ((fptr->bytes -= bytes) < 0L &&
		    (fptr->bytes != -1 || pad1 == FALSE)) {
			err("adj_frames: EOF on frame!\n%4.4s (%ld bytes)",
				&fptr->id, bytes);
			wait1();
ERR1("adj_frames: fptr->bytes ended up %ld", fptr->bytes);
			barf_iff(iffp);
		}
		fptr = fptr->par;	/* up link to parent frame	*/
	}

	return (bytes);
}


/*
 * read_bytes -- Read the given number of bytes from the current chunk,
 *		verify them against and subtract them from the primary
 *		chunk's bytes, etc.
 */

Static void
read_bytes(iff_t *iffp, frame_t *fptr, APTR buf, LONG bytes)
{
	LONG	ret;

	bytes = adj_frames(iffp, fptr, bytes);

	ret = Read(iffp->file, buf, bytes);
	if (ret < 0L) {
		err("read_bytes:\nRead error = %ld!\nChunk %4.4s",
			IoErr(), &fptr->id);
		goto wait4it;
	}
	if (ret != bytes) {
		err("read_bytes: Short file!\n"
			"got %ld bytes\nwanted %ld\nChunk %4.4s",
			ret, bytes, &fptr->id);
wait4it:
		wait1();
		barf_iff(iffp);
	}
}


/*
 * skip_chunk -- skip the remainder of the chunk in iffp
 */

Static void
skip_chunk(iff_t *iffp, frame_t *fptr)
{
	LONG	bytes = iffp->chunk.ckSize;

	bytes = adj_frames(iffp, fptr, bytes);
ERR2("(skipping %4.4s chunk, %ld bytes)", &iffp->chunk.ckID, bytes);

	bytes = Seek(iffp->file, bytes, OFFSET_CURRENT);
	if (bytes < 0L) {
		err("skip_chunk:\nSeek error = %ld!\nChunk %4.4s",
			IoErr(), &fptr->id);
		wait1();
		barf_iff(iffp);
	}
}


/*
 * read_chunk -- Read a Chunk from iffp and verify it against the IFF
 *		standard.  If non-NULL, also check against the local
 *		byte counter, bytep.
 */

Static void
read_chunk(iff_t *iffp, frame_t *fptr)
{
	Chunk	*chunkp = &iffp->chunk;
	int	n;
	char	*p;

	read_bytes(iffp, fptr, chunkp, sizeof(Chunk));
	if (chunkp->ckSize < 0) {
		err("read_chunk:\nnegative ckSize! (%ld)", chunkp->ckSize);
		goto wait4it;
	}
	if (chunkp->ckSize > 0x001fffff) {
		err("read_chunk:\nexcessive ckSize! (%ld)", chunkp->ckSize);
wait4it:
		wait1();
		barf_iff(iffp);
	}
	p = (char *)(void *)&chunkp->ckID;
	for (n = sizeof(chunkp->ckID); --n >= 0; ++p) {
		if (*p < ' ' || *p > '~') {
			err("read_chunk:\nbad ID = 0x%08lx", chunkp->ckID);
			goto wait4it;
		}
	}
}


/*
 * get_body -- Use a 8SVX BODY chunk and Voice8Header to initialize
 *		a SOUND structure
 *
 * Expects chunk header in iffp->chunk.
 */

Static void
get_body(iff_t *iffp, frame_t *fptr, Voice8Header *v8p, uint index)
{
	SOUND	*sp = &sounds[index];
	LONG	v;

	v = v8p->oneShotHiSamples + v8p->repeatHiSamples;
	if (v8p->sCompression) {
		v = 2 * (v - 2);
		if (v <= 0) {
			err("VHDR: no sound samples\nleft after decompress!");
			wait1();
			barf_iff(iffp);
		}
	}
	sp->length = v;

	v = v8p->volume >> (16 - 6);	/* same as (vol * 64) / Unity */
	if (v <= 0)
		v = 0;
	else if (v > 64)
		v = 64;
	sp->volume = v;
	sp->period = v8p->samplesPerSec;	/* convert in init_audio */
	sp->cycles = (v8p->repeatHiSamples == 0);
ERR4("SOUND:\n length = %ld\n volume = %ld\n period = %ld\n cycles = %ld", sp->length, (ulong)sp->volume, (ulong)sp->period, (ulong)sp->cycles);

	sp->data = (BYTE *) AllocMem(sp->length, MEMF_CHIP);
	read_bytes(iffp, fptr, sp->data, sp->length);
	if (v8p->sCompression) {
		FD_Unpack(sp->data, sp->length);
	}
}


/*
 * get_vhdr -- Read a 8SVX VHDR chunk into a Voice8Header and verify it.
 *
 * Expects chunk header in iffp->chunk.
 */

Static void
get_vhdr(iff_t *iffp, frame_t *fptr, Voice8Header *v8p)
{
	register ULONG	len;

	read_bytes(iffp, fptr, v8p, sizeof(*v8p));
	if (v8p->sCompression > sCmpFibDelta) {
		err("Unknown VHDR compression!\ntype=0x%lx",
			v8p->sCompression);
		goto wait4it;
	}
	len = v8p->oneShotHiSamples;
	if (len >= 0x20000) {
		err("VHDR: oneShotHiSamples\ntoo big! %ld", len);
		goto wait4it;
	}
	if (len == 0 && v8p->repeatHiSamples == 0) {
		err("VHDR: no sound samples!");
		goto wait4it;
	}
	if (len && v8p->repeatHiSamples) {
		err("VHDR: I don't do combined\nOneShot / Repeat 8SVX");
wait4it:
		wait1();
		barf_iff(iffp);
	}
	len = v8p->repeatHiSamples;
	if (len >= 0x20000) {
		err("VHDR: repeatHiSamples too big!\n(%ld)", len);
		goto wait4it;
	}
}


/*
 * get_name -- Read the NAME chunk and check the name list
 *
 * Expects chunk header in iffp->chunk.
 *
 * Returns the sounds[] index, or -1.
 */
/*** JPC: Soon, convert this to use NAME chunks! ***/

int
get_name(iff_t *iffp, frame_t *fptr)
{
	LONG		n;
	int		offset;
	SOUND		*sp;
	char		*p;
	char		**namep;
	frame_t 	frame;
	char		name[20];			/* max 19 bytes */

	frame.par = fptr;
	frame.id = iffp->chunk.ckID;
	n = iffp->chunk.ckSize;
	frame.bytes = n;

	if (n > sizeof(name) - 1)
		n = sizeof(name) - 1;

	/* read the NAME's file name  */
	read_bytes(iffp, &frame, name, n);
	name[n] = '\0';
ERR1(". . NAME %s", name);

	/* if the name was too long to fit in name, eat the rest */
	if (frame.bytes > 0L) {
		iffp->chunk.ckSize = frame.bytes;	/* skip the rest */
		skip_chunk(iffp, &frame);
	}

	/* look at the last character */

	p = &name[strlen(name) - 1];
	offset = 0;

	switch (*p) {
	case '2':
		offset = NUM_SOUNDS_PER_PLAYER;
		/* fall through! */
	case '1':
		*p = '\0';				/* snuff [12] */
	DEFAULT:
		/* assume player 1, offset = 0 */
		break;
	}

	for (namep = snd_name, n = NEL(snd_name); --n >= 0; namep++) {
		if (strcmp(name, *namep) == 0)
			break;				/* gotcha */
	}
	if (n < 0) {
ERR1("Didn't find NAME %s in list!", name);
		return (-1);				/* not on list */
	}

	n = (namep - snd_name) + offset;
	sp = &sounds[n];

	if (sp->data) {
		err("Got two 8SVXs NAMEed '%s%ld'!", name, (offset ? 2 : 1));
		barf_iff(iffp);
	}
ERR2("NAME %s is sound # %ld", name, n);

	return (n);
}


/*
 * get_8SVX -- Read a FORM 8SVX chunk and load up a SOUND structure
 *
 * Expects chunk header in iffp->chunk.
 */

Static void
get_8SVX(iff_t *iffp, frame_t *fptr, Voice8Header *v8p)
{
	frame_t 	frame;
	Voice8Header	v8h;
	uchar		got_v8h;
	uchar		got_body;
	char		index;

	frame.par = fptr;
	frame.bytes = iffp->chunk.ckSize;
	frame.id = iffp->chunk.ckID;

	/* read the FORM's type field */
	read_bytes(iffp, &frame, &iffp->chunk.ckID,
			sizeof(iffp->chunk.ckID));
ERR1(". FORM %4.4s", (char *)(&iffp->chunk.ckID));
	if (iffp->chunk.ckID != ID_8SVX) {
skip_it:
		iffp->chunk.ckSize = frame.bytes;	/* skip the rest */
		skip_chunk(iffp, &frame);
		return; 				/* not 8SVX */
	}

	/*
	 * loop and slurp up chunks
	 */

	index = -1;
	got_v8h = got_body = FALSE;

	while (frame.bytes > 0L) {
		read_chunk(iffp, &frame);
ERR2(". . %4.4s (%ld)", (char *)(&iffp->chunk.ckID), iffp->chunk.ckSize);
		switch (iffp->chunk.ckID) {
		case ID_VHDR:
			get_vhdr(iffp, &frame, &v8h);
			got_v8h = TRUE;
		CASE ID_BODY:
			if (got_v8h != FALSE) {
				v8p = &v8h;		/* use local VHDR */
			}
			else if (v8p == NULL) {
				err("8SVX: missing VHDR!");
				goto wait4it;
			}
			got_body = TRUE;
			get_body(iffp, &frame, v8p, index);
		CASE ID_NAME:
			if ((index = get_name(iffp, &frame)) < 0)
				goto skip_it;
		DEFAULT:
			skip_chunk(iffp, &frame);	/* skip unknown */
		}
	}

	if (got_body == FALSE) {
		err("8SVX: missing BODY!");
wait4it:
		wait1();
		barf_iff(iffp);
	}
}


/*
 * get_prop -- Read a PROP chunk, which may contain VHDR chunks.
 *		This had better be a 8SVX PROP.
 *
 * Expects chunk header in iffp->chunk.
 */

Static int
get_prop(iff_t *iffp, frame_t *fptr, Voice8Header *v8p)
{
	frame_t 	frame;
	uchar		got_v8h;

	frame.par = fptr;
	frame.bytes = iffp->chunk.ckSize;
	frame.id = iffp->chunk.ckID;

	/* read the PROP's type field */
	read_bytes(iffp, &frame, &iffp->chunk.ckID,
			sizeof(iffp->chunk.ckID));
ERR1(". PROP %4.4s", (char *)(&iffp->chunk.ckID));
	if (iffp->chunk.ckID != ID_8SVX) {
		iffp->chunk.ckSize = frame.bytes;	/* skip the rest */
		skip_chunk(iffp, &frame);
		return (FALSE);				/* not 8SVX */
	}

	/*
	 * loop and slurp up chunks
	 */

	got_v8h = FALSE;

	while (frame.bytes > 0L) {
		read_chunk(iffp, &frame);
		switch (iffp->chunk.ckID) {
		case ID_VHDR:
			get_vhdr(iffp, &frame, v8p);
			got_v8h = TRUE;
		DEFAULT:
ERR1(". . %4.4s", (char *)(&iffp->chunk.ckID));
			skip_chunk(iffp, &frame);	/* nothing else */
		}
	}
	return (got_v8h);
}


/*
 * get_list -- Read a LIST chunk, which is just made of VHDR and FORM chunks.
 *		This had better be a 8SVX LIST.
 *		(Ignore nested CAT or LIST chunks.)
 *
 * Expects chunk header in iffp->chunk.
 */

Static void
get_list(iff_t *iffp, frame_t *fptr)
{
	frame_t 	frame;
	Voice8Header	v8h;
	uchar		got_v8h;
	uchar		prop_end;

	frame.par = fptr;
	frame.bytes = iffp->chunk.ckSize;
	frame.id = iffp->chunk.ckID;

	/* read the LIST's type field */
	read_bytes(iffp, &frame, &iffp->chunk.ckID,
			sizeof(iffp->chunk.ckID));
ERR1("LIST %4.4s", (char *)(&iffp->chunk.ckID));
	if (iffp->chunk.ckID != ID_8SVX && iffp->chunk.ckID != ID_NULL) {
		iffp->chunk.ckSize = frame.bytes;	/* skip the rest */
		skip_chunk(iffp, &frame);
		return; 				/* not 8SVX */
	}

	/*
	 * loop and slurp up chunks
	 */

	got_v8h = prop_end = FALSE;

	while (frame.bytes > 0L) {
		read_chunk(iffp, &frame);
		switch (iffp->chunk.ckID) {
		case ID_FORM:
			prop_end = TRUE;
			get_8SVX(iffp, &frame, (got_v8h ? &v8h : NULL));
		CASE ID_PROP:
			if (prop_end) {
				err("get_list:\nGot PROP after non-PROP!");
				goto wait4it;
			}
			got_v8h |= get_prop(iffp, &frame, &v8h);
		CASE ID_VHDR:
			err("get_list:\nVHDR chunk outside of PROP!");
wait4it:
			wait1();
			barf_iff(iffp);
		DEFAULT:
			prop_end = TRUE;		/* non-PROP */
ERR1(". %4.4s", (char *)(&iffp->chunk.ckID));
			skip_chunk(iffp, &frame);	/* no nested CATs */
		}
	}
}


/*
 * get_cat -- Read a CAT chunk, which is made of FORM or LIST chunks.
 *		(Ignore nested CAT chunks.)
 *
 * Expects chunk header in iffp->chunk.
 */

Static void
get_cat(iff_t *iffp, frame_t *fptr)
{
	frame_t frame;

	frame.par = fptr;
	frame.bytes = iffp->chunk.ckSize;
	frame.id = iffp->chunk.ckID;

	/* read (and discard) the CAT's ID hint field */
	read_bytes(iffp, &frame, &iffp->chunk.ckID,
			sizeof(iffp->chunk.ckID));
ERR1("CAT %4.4s", (char *)(&iffp->chunk.ckID));

	/*
	 * loop and slurp up chunks
	 */
	while (frame.bytes > 0) {
		read_chunk(iffp, &frame);
		switch (iffp->chunk.ckID) {
		case ID_FORM:
			get_8SVX(iffp, &frame, NULL);
		CASE ID_CAT:
			skip_chunk(iffp, &frame);	/* no nested CATs */
		CASE ID_LIST:
			get_list(iffp, &frame);
		DEFAULT:
			err("get_cat:\nbad chunk ID = %4.4s",
				&iffp->chunk.ckID);
			wait1();
			barf_iff(iffp);
		}
	}
}

Static void	interpolate_sounds(void);

/*
 * parse_file -- Read the first header and decide if it is an IFF file, etc.
 */

void
parse_file(BPTR file)
{
	iff_t	my_iff;
	frame_t frame;

	my_iff.file = file;
	frame.par = NULL;		/* I'm the top  */
	frame.bytes = sizeof(Chunk);	/* haven't got primary chunk yet */

	read_chunk(&my_iff, &frame);
	frame.bytes = my_iff.chunk.ckSize;	/* got chunk size */
	frame.id = my_iff.chunk.ckID;

	switch (my_iff.chunk.ckID) {
	case ID_FORM:
		err("SSW.snd must be a CAT or LIST\n"
			"of at least 8 NAMEed FORM 8SVXs");
wait4it:
		wait1();
		barf_iff(&my_iff);
	CASE ID_CAT:
		get_cat(&my_iff, &frame);
	CASE ID_LIST:
		get_list(&my_iff, &frame);
	DEFAULT:
		err("parse_file:\n\nbad IFF file ID = %4.4s",
			&my_iff.chunk.ckID);
		goto wait4it;
	}

	if (my_iff.file) {
		Close(my_iff.file);
		my_iff.file = NULL;
	}

	interpolate_sounds();
}

#define NUM_PING	(2 * NUM_SOUNDS_PER_PLAYER)
#define PER_ADJ(sp, n)  ((sp)->period = ((ulong)(sp)->period << 16) / (n))

/*
 * interpolate_sounds -- We've got some sounds, so duplicate where necessary.
 */

Static void
interpolate_sounds(void)
{
	register SOUND	*sp;
	register int	i;

ERR0("interpolate_sounds!");
	sp = sounds;
	for (i = 0; i < NEL(sounds); i++, sp++) {
		register SOUND	*xp;

		if (sp->data != NULL)
			continue;				/* ok */

ERR1("Trying sound # %ld", i);
		if (i >= NUM_PING)
			xp = sp;
		else if (i >= NUM_SOUNDS_PER_PLAYER)
			xp = &sp[-NUM_SOUNDS_PER_PLAYER];
		else
			xp = &sp[NUM_SOUNDS_PER_PLAYER];

		*sp = *xp;		/* be optimistic -- copy first */

		switch (i) {
		case 0: 					/* fire1 */
		case 1: 					/* explode1*/
		case 4: 					/* thrust1 */
		case 5: 					/* hyp_out1*/
		case 6: 					/* hyp_in1 */
		case 7: 					/* bounce1 */
		case 9: 					/* fire2 */
		case 10:					/* explode2*/
		case 13:					/* thrust2 */
		case 14:					/* hyp_out2*/
		case 15:					/* hyp_in2 */
		case 16:					/* bounce2 */
			if (xp->data == NULL) {
				goto missing_8svx;
			}
		CASE 2: 					/* shield1 */
			if (xp->data == NULL) {
				xp = &sounds[NUM_SOUNDS_PER_PLAYER+0];
				if (xp->data == NULL)
					xp = &sounds[0];
				*sp = *xp;
				PER_ADJ(sp, 37663U);
			}
		CASE 3: 					/* torpdie1*/
			if (xp->data == NULL) {
				xp = &sounds[NUM_SOUNDS_PER_PLAYER+1];
				if (xp->data == NULL)
					xp = &sounds[1];
				*sp = *xp;
				PER_ADJ(sp, 27219U);
			}
		CASE 8: 					/* quad1   */
			if (xp->data == NULL) {
				xp = &sounds[NUM_SOUNDS_PER_PLAYER+4];
				if (xp->data == NULL)
					xp = &sounds[4];
				*sp = *xp;
				PER_ADJ(sp, 38993U);
			}
		CASE 11:					/* shield2 */
			if (xp->data == NULL) {
				goto missing_8svx;
			}
			/*
			 * If shield1 was derived from fire2, and fire1 is
			 * different from fire2, then use fire1 instead.
			 */
			xp = &sounds[NUM_SOUNDS_PER_PLAYER+0];
			if (sp->data == xp->data &&
			    xp->data != sounds[0].data) {
				*sp = sounds[0];
				PER_ADJ(sp, 37663U);
			}
		CASE 12:					/* torpdie2*/
			if (xp->data == NULL) {
				goto missing_8svx;
			}
			/*
			 * If torpdie1 was derived from explode2, and
			 * explode1 is different from explode2, then use
			 * explode1 instead.
			 */
			xp = &sounds[NUM_SOUNDS_PER_PLAYER+1];
			if (sp->data == xp->data &&
			    xp->data != sounds[1].data) {
				*sp = sounds[1];
				PER_ADJ(sp, 27219U);
			}
		CASE 17:					/* quad2   */
			/*
			 * If the cooresponding player 1 sounds are still
			 * NULL, we won't do any better here...
			 */
			if (xp->data == NULL) {
				goto missing_8svx;
			}
			/*
			 * If quad1 was derived from thrust2, and thrust1 is
			 * different from thrust2, then use thrust1 instead.
			 */
			xp = &sounds[NUM_SOUNDS_PER_PLAYER+4];
			if (sp->data == xp->data &&
			    xp->data != sounds[4].data) {
				*sp = sounds[4];
				PER_ADJ(sp, 38993U);
			}
		CASE 18:					/* ping0 */
			xp = &sounds[7];
			*sp = *xp;
			PER_ADJ(sp, 39219U);
		CASE 19:					/* ping1 */
			xp = &sounds[NUM_SOUNDS_PER_PLAYER+7];
			*sp = *xp;
			PER_ADJ(sp, 39219U);
		CASE 20:					/* ping2 */
			xp = &sounds[7];
			*sp = *xp;
			PER_ADJ(sp, 49437U);
		CASE 21:					/* ping3 */
			xp = &sounds[NUM_SOUNDS_PER_PLAYER+7];
			*sp = *xp;
			PER_ADJ(sp, 49437U);
		DEFAULT:
			err("interpolate_sounds:\nhosed! (%ld)", i);
			wait1();
			die(0);
		}
	}
	/*
	 * Make sure that a ship's torpdie sound doesn't have the same
	 * data pointer as its fire or explode sound, otherwise
	 * no_flush_sound() gets confused.
	 */
	sp = &sounds[3];
	if (sp->data == sounds[0].data || sp->data == sounds[1].data) {
		sp->data += 2;
		sp->length -= 2;
	}
	sp = &sounds[12];
	if (sp->data == sounds[9].data || sp->data == sounds[10].data) {
		sp->data += 2;
		sp->length -= 2;
	}
ERR0("interpolate done!");

    { register snd_parm_t	*prm;

	/*
	 * Set volumes and unit numbers.
	 */
	prm = snd_parm;
	sp = sounds;
	for (i = NEL(sounds); --i >=0; ++sp, ++prm) {
		if (sp->data == NULL)
			goto missing_8svx;
		sp->unit = prm->unit;
		sp->cycles = prm->cycles;
		sp->volume = prm->volume;
	}
    }
ERR0("param set done!");

	return;

missing_8svx:
	i = (sp - sounds);
	if (i >= NUM_PING)
		i = 7;
	else
		i %= NUM_SOUNDS_PER_PLAYER;
	err("Need a 8SVX for %s1\nor %s2", snd_name[i], snd_name[i]);
	wait1();
	die(0);
}
