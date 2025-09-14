/*
 * Copyright (c) 1983, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * 2020-04-10:  Hacked by James Cleverdon to use only a fixed DEG_2 state
 *              and remove unused code.
 */

#ident "@(#)random.c	8.1 (Berkeley) 6/4/93"

	/* define my own types -- JPC */
#define uchar		unsigned char
#define uint		unsigned int
#define ulong		unsigned long
	/* also, be able to turn static on and off
#define Static		/* static */	/* CPR doesn't like statics much */

/*
 * random.c:
 *
 * An improved random number generation package.  In addition to the
 * standard rand()/srand() like interface, this package also has a
 * special state info interface.  The initstate() routine is called with
 * a seed, an array of bytes, and a count of how many bytes are being
 * passed in; this array is then initialized to contain information for
 * random number generation with that much state information.  Good
 * sizes for the amount of state information are 32, 64, 128, and 256
 * bytes.  The state can be switched by calling the setstate() routine
 * with the same array as was initiallized with initstate().  By default,
 * the package runs with 128 bytes of state information and generates
 * far better random numbers than a linear congruential generator.
 *
 * Internally, the state information is treated as an array of longs;
 * the zeroeth element of the array is the type of R.N.G. being used
 * (small integer); the remainder of the array is the state information
 * for the R.N.G.  Thus, 32 bytes of state information will give 7 longs
 * worth of state information, which will allow a degree seven
 * polynomial.  (Note: the zeroeth word of state information also has
 * some other information stored in it -- see setstate() for details).
 *
 * The random number generation technique is a linear feedback shift
 * register approach, employing trinomials (since there are fewer terms
 * to sum up that way).  In this approach, the least significant bit of
 * all the numbers in the state table will act as a linear feedback
 * shift register, and will have period 2^deg - 1 (where deg is the
 * degree of the polynomial being used, assuming that the polynomial is
 * irreducible and primitive).  The higher order bits will have longer
 * periods, since their values are also influenced by pseudo-random
 * carries out of the lower bits.  The total period of the generator is
 * approximately deg*(2**deg - 1); thus doubling the amount of state
 * information has a vast influence on the period of the generator.
 * Note: the deg*(2**deg - 1) is an approximation only good for large
 * deg, when the period of the shift register is the dominant factor.
 * With deg equal to seven, the period is actually much longer than the
 * 7*(2**7 - 1) predicted by this formula.
 */

/*
 * For each of the currently supported random number generators, we have
 * a break value on the amount of state information (you need at least
 * this many bytes of state info to support this random number
 * generator), a degree for the polynomial (actually a trinomial) that
 * the R.N.G. is based on, and the separation between the two lower
 * order coefficients of the trinomial.
 */
#define	TYPE_1		1		/* x**7 + x**3 + 1 */
#define	BREAK_1		32
#define	DEG_1		7
#define	SEP_1		3

#define	TYPE_2		2		/* x**15 + x + 1 */
#define	BREAK_2		64
#define	DEG_2		15
#define	SEP_2		1

#define	TYPE_3		3		/* x**31 + x**3 + 1 */
#define	BREAK_3		128
#define	DEG_3		31
#define	SEP_3		3

#define	TYPE_4		4		/* x**63 + x + 1 */
#define	BREAK_4		256
#define	DEG_4		63
#define	SEP_4		1

#define MY_DEG		DEG_2
#define MY_SEP		SEP_2


ulong	randtbl[MY_DEG] = {
	0x9a319039, 0x32d9c024, 0x9b663182, 0x5da1f342,
	0xde3b81e0, 0xdf0a6fb5, 0xf103bc02, 0x48f340fb,
	0x7449e56b, 0xbeb1dbb0, 0xab5c5918, 0x946554fd,
	0x8c2e680f, 0xeb3d799f, 0xb11ee0b7,
};

/*
 * fptr and rptr are two pointers into the state info, a front and a
 * rear pointer.  These two pointers are always rand_sep places aparts,
 * as they cycle cyclically through the state information.  (Yes, this
 * does mean we could get away with just one pointer, but the code for
 * random() is more efficient this way).
 */
ulong	*fptr = &randtbl[MY_SEP];
ulong	*rptr = &randtbl[0];

/*
 * We remember the last location, since this is more efficient than indexing
 * every time to find the address of the last element to see if the front and
 * rear pointers have wrapped.
 */
#define	end_ptr	(&randtbl[MY_DEG])
ulong	*srand_ptr = &randtbl[0];


/*
 * rand64:
 *
 * We do our fancy trinomial stuff, which is the same in all the other cases
 * due to all the global variables that have been set up.  The basic operation
 * is to add the number at the rear pointer into the one at the front pointer.
 * Then both pointers are advanced to the next location cyclically in the
 * table.  The value returned is the sum generated, reduced to 31 bits by
 * throwing away the "least random" low bit.
 *
 * Note: the code takes advantage of the fact that both the front and
 * rear pointers can't wrap on the same call by not testing the rear
 * pointer if the front one has wrapped.
 *
 * Returns a 31-bit random number.
 */
#if 1
#if 1
long
rand64(void)
{
	register ulong	i;
	register ulong	val;
	register long	ret;

	val = (*fptr++ += *rptr++);
	if (fptr >= end_ptr) {
		fptr = randtbl;
	} else if (rptr >= end_ptr)
		rptr = randtbl;
	val >>= 1;			/* chucking least random bit */
	val &= 0xFFFF;
	i = (*fptr++ += *rptr++);
	if (fptr >= end_ptr) {
		fptr = randtbl;
	} else if (rptr >= end_ptr)
		rptr = randtbl;
	i >>= 1;			/* chucking least random bit */
	i <<= 16;
	ret = (long)(i | val);
	if (ret < 0) {
		ret = -ret;
	}
	return (ret);
}
#else
long
rand64(void)
{
	register ulong	i;

	i = (*fptr++ += *rptr++);
	if (fptr >= end_ptr) {
		fptr = randtbl;
	} else if (rptr >= end_ptr)
		rptr = randtbl;
	i >>= 1;			/* chucking least random bit */
	return ((long)i);
}
#endif


/*
 * srand64:
 *
 * Initialize the random number generator based on the given seed values.
 * Lastly, it cycles the state information a given number of times.
 */
void
srand64(ulong x, ulong y)
{
	register int	i;

	*srand_ptr++ += x;
	if (srand_ptr >= end_ptr) {
		srand_ptr = randtbl;
	}
	*srand_ptr++ += y;
	if (srand_ptr >= end_ptr) {
		srand_ptr = randtbl;
	}

	for (i = MY_DEG; --i >= 0; ) {
		(void)rand64();
	}
}
#endif	/* 0 */
