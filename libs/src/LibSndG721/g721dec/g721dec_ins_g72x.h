#pragma once

/*
 * This source code is a product of Sun Microsystems, Inc. and is provided
 * for unrestricted use.  Users may copy or modify this source code without
 * charge.
 *
 * SUN SOURCE CODE IS PROVIDED AS IS WITH NO WARRANTIES OF ANY KIND INCLUDING
 * THE WARRANTIES OF DESIGN, MERCHANTIBILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE, OR ARISING FROM A COURSE OF DEALING, USAGE OR TRADE PRACTICE.
 *
 * Sun source code is provided with no support and without any obligation on
 * the part of Sun Microsystems, Inc. to assist in its use, correction,
 * modification or enhancement.
 *
 * SUN MICROSYSTEMS, INC. SHALL HAVE NO LIABILITY WITH RESPECT TO THE
 * INFRINGEMENT OF COPYRIGHTS, TRADE SECRETS OR ANY PATENTS BY THIS SOFTWARE
 * OR ANY PART THEREOF.
 *
 * In no event will Sun Microsystems, Inc. be liable for any lost revenue
 * or profits or other special, indirect and consequential damages, even if
 * Sun has been advised of the possibility of such damages.
 *
 * Sun Microsystems, Inc.
 * 2550 Garcia Avenue
 * Mountain View, California  94043
 */

/*
 * g72x.c
 *
 * Common routines for G.721 and G.723 conversions.
 */

//#include "g72x.h"

/*
 * quan()
 *
 * quantizes the input val against the table of size short integers.
 * It returns i if table[i - 1] <= val < table[i].
 *
 * Using linear search for simple coding.
 */
static INLINE int
_quan(
	int		val,
	const short		*_table)
{
	const short		*table=_table;
	
	u32 i=0;
	while(1){
		if (val < table[i]) return(i);
		i++;
	}
}
#define quan(a,b,c) _quan(a,b)

static INLINE int
quan721(
	int		val)
{
	u32 uval=(u32)val;
	
	if(val<-124) return(0);
	if(val<80) return(1);
	if(uval<178) return(2);
	if(uval<246) return(3);
	if(uval<300) return(4);
	if(uval<349) return(5);
	if(uval<400) return(6);
	return(7);
}

static INLINE u32 getpower2(u32 val)
{
  u32 p2i=0;
  u32 p2v=1;
  
  while(1){
    if(val<p2v) return(p2i);
    p2v<<=1;
    p2i++;
    if(p2i==15) return(p2i);
  }
}

/*
 * fmult()
 *
 * returns the integer product of the 14-bit integer "an" and
 * "floating point" representation (4-bit exponent, 6-bit mantessa) "srn".
 */
static INLINE int
fmult(
	int		an,
	int		srn)
{
	short		anmag, anexp, anmant;
	short		wanexp, wanmant;
	short		retval;

	anmag = (an > 0) ? an : ((-an) & 0x1FFF);
	anexp = getpower2(anmag) - 6;
	anmant = (anmag == 0) ? 32 :
	    (anexp >= 0) ? anmag >> anexp : anmag << -anexp;
	wanexp = anexp + ((srn >> 6) & 0xF) - 13;

	wanmant = (anmant * (srn & 077) + 0x30) >> 4;
	retval = (wanexp >= 0) ? ((wanmant << wanexp) & 0x7FFF) :
	    (wanmant >> -wanexp);

	return (((an ^ srn) < 0) ? -retval : retval);
}

/*
 * g72x_init_state()
 *
 * This routine initializes and/or resets the g72x_state structure
 * pointed to by 'pState'.
 * All the initial state values are specified in the CCITT G.721 document.
 */
void g72x_init_state(TG721State *pState)
{
	u32		cnta;

	pState->yl = 34816;
	pState->yu = 544;
	pState->dms = 0;
	pState->dml = 0;
	pState->ap = 0;
	for (cnta = 0; cnta < 2; cnta++) {
		pState->a[cnta] = 0;
		pState->pk[cnta] = 0;
		pState->sr[cnta] = 32;
	}
	for (cnta = 0; cnta < 6; cnta++) {
		pState->b[cnta] = 0;
		pState->dq[cnta] = 32;
	}
	pState->td = 0;
}

/*
 * predictor_zero()
 *
 * computes the estimated signal from 6-zero predictor.
 *
 */
static int predictor_zero(const TG721State *pState)
{
	u32		i;
	int		sezi;

	sezi = 0;
	for (i = 0; i < 6; i++)			/* ACCUM */
		sezi += fmult(pState->b[i] >> 2, pState->dq[i]);
	return (sezi);
}
/*
 * predictor_pole()
 *
 * computes the estimated signal from 2-pole predictor.
 *
 */
static INLINE int predictor_pole(const TG721State *pState)
{
	return (fmult(pState->a[1] >> 2, pState->sr[1]) +
	    fmult(pState->a[0] >> 2, pState->sr[0]));
}
/*
 * step_size()
 *
 * computes the quantization step size of the adaptive quantizer.
 *
 */
static INLINE int step_size(const TG721State *pState)
{
	int		y;
	int		dif;
	int		al;

	if (pState->ap >= 256)
		return (pState->yu);
	else {
		y = pState->yl >> 6;
		dif = pState->yu - y;
		al = pState->ap >> 2;
		if (dif > 0)
			y += (dif * al) >> 6;
		else if (dif < 0)
			y += (dif * al + 0x3F) >> 6;
		return (y);
	}
}

/*
 * quantize721() for g.721
 *
 * Given a raw sample, 'd', of the difference signal and a
 * quantization step size scale factor, 'y', this routine returns the
 * ADPCM codeword to which that sample gets quantized.  The step
 * size scale factor division operation is done in the log base 2 domain
 * as a subtraction.
 */
static INLINE int
quantize721(
	int		d,	/* Raw difference signal sample */
	int		y)	/* Step size multiplier */
{
	const int		size=7;	/* table size of short integers */
	
	short		dqm;	/* Magnitude of 'd' */
	short		exp;	/* Integer part of base 2 log of 'd' */
	short		mant;	/* Fractional part of base 2 log */
	short		dl;	/* Log of magnitude of 'd' */
	short		dln;	/* Step size scale factor normalized log */
	int		i;

	/*
	 * LOG
	 *
	 * Compute base 2 log of 'd', and store in 'dl'.
	 */
	dqm = abs(d);
	exp = getpower2(dqm >> 1);
	mant = ((dqm << 7) >> exp) & 0x7F;	/* Fractional portion. */
	dl = (exp << 7) + mant;
	
	/*
	 * SUBTB
	 *
	 * "Divide" by step size multiplier.
	 */
	dln = dl - (y >> 2);

	/*
	 * QUAN
	 *
	 * Obtain codword i for 'd'.
	 */
	i = quan721(dln);
	if (d < 0)			/* take 1's complement of i */
		return ((size << 1) + 1 - i);
	else if (i == 0)		/* take 1's complement of 0 */
		return ((size << 1) + 1); /* new in 1988 */
	else
		return (i);
}

/*
 * reconstruct()
 *
 * Returns reconstructed difference signal 'dq' obtained from
 * codeword 'i' and quantization step size scale factor 'y'.
 * Multiplication is performed in log base 2 domain as addition.
 */
static INLINE int
reconstruct(
	int		sign,	/* 0 for non-negative value */
	int		dqln,	/* G.72x codeword */
	int		y)	/* Step size multiplier */
{
	short		dql;	/* Log of 'dq' magnitude */
	short		dex;	/* Integer part of log */
	short		dqt;
	short		dq;	/* Reconstructed difference signal sample */

	dql = dqln + (y >> 2);	/* ADDA */

	if (dql < 0) {
		return ((sign) ? -0x8000 : 0);
	} else {		/* ANTILOG */
		dex = (dql >> 7) & 15;
		dqt = 128 + (dql & 127);
		dq = (dqt << 7) >> (14 - dex);
		return ((sign) ? (dq - 0x8000) : dq);
	}
}

/*
 * update()
 *
 * updates the state variables for each output code
 */
static INLINE void update(
	int		y,		/* quantizer step size */
	int		wi,		/* scale factor multiplier */
	int		fi,		/* for long/short term energies */
	int		dq,		/* quantized prediction difference */
	int		sr,		/* reconstructed signal */
	int		dqsez,		/* difference from 2-pole predictor */
	TG721State *pState)	/* coder state pointer */
{
	int		cnt;
	short		mag, exp;	/* Adaptive predictor, FLOAT A */
	short		a2p;		/* LIMC */
	short		a1ul;		/* UPA1 */
	short		pks1;	/* UPA2 */
	short		fa1;
	char		tr;		/* tone/transition detector */
	short		ylint, thr2, dqthr;
	short  		ylfrac, thr1;
	short		pk0;

	pk0 = (dqsez < 0) ? 1 : 0;	/* needed in updating predictor poles */

	mag = dq & 0x7FFF;		/* prediction difference magnitude */
	/* TRANS */
	ylint = pState->yl >> 15;	/* exponent part of yl */
	ylfrac = (pState->yl >> 10) & 0x1F;	/* fractional part of yl */
	thr1 = (32 + ylfrac) << ylint;		/* threshold */
	thr2 = (ylint > 9) ? 31 << 10 : thr1;	/* limit thr2 to 31 << 10 */
	dqthr = (thr2 + (thr2 >> 1)) >> 1;	/* dqthr = 0.75 * thr2 */
	if (pState->td == 0)		/* signal supposed voice */
		tr = 0;
	else if (mag <= dqthr)		/* supposed data, but small mag */
		tr = 0;			/* treated as voice */
	else				/* signal is data (modem) */
		tr = 1;

	/*
	 * Quantizer scale factor adaptation.
	 */

	/* FUNCTW & FILTD & DELAY */
	/* update non-steady state step size multiplier */
	pState->yu = y + ((wi - y) >> 5);

	/* LIMB */
	if (pState->yu < 544)	/* 544 <= yu <= 5120 */
		pState->yu = 544;
	else if (pState->yu > 5120)
		pState->yu = 5120;

	/* FILTE & DELAY */
	/* update steady state step size multiplier */
	pState->yl += pState->yu + ((-pState->yl) >> 6);

	/*
	 * Adaptive predictor coefficients.
	 */
	if (tr == 1) {			/* reset a's and b's for modem signal */
		a2p = 0;
		pState->a[0] = 0;
		pState->a[1] = 0;
		pState->b[0] = 0;
		pState->b[1] = 0;
		pState->b[2] = 0;
		pState->b[3] = 0;
		pState->b[4] = 0;
		pState->b[5] = 0;
	} else {			/* update a's and b's */
		pks1 = pk0 ^ pState->pk[0];		/* UPA2 */

		/* update predictor pole a[1] */
		a2p = pState->a[1] - (pState->a[1] >> 7);
		if (dqsez != 0) {
			fa1 = (pks1) ? pState->a[0] : -pState->a[0];
			if (fa1 < -8191)	/* a2p = function of fa1 */
				a2p -= 0x100;
			else if (fa1 > 8191)
				a2p += 0xFF;
			else
				a2p += fa1 >> 5;

			if (pk0 ^ pState->pk[1])
				/* LIMC */
				if (a2p <= -12160)
					a2p = -12288;
				else if (a2p >= 12416)
					a2p = 12288;
				else
					a2p -= 0x80;
			else if (a2p <= -12416)
				a2p = -12288;
			else if (a2p >= 12160)
				a2p = 12288;
			else
				a2p += 0x80;
		}

		/* TRIGB & DELAY */
		pState->a[1] = a2p;

		/* UPA1 */
		/* update predictor pole a[0] */
		pState->a[0] -= pState->a[0] >> 8;
		if (dqsez != 0)
			if (pks1 == 0)
				pState->a[0] += 192;
			else
				pState->a[0] -= 192;

		/* LIMD */
		a1ul = 15360 - a2p;
		if (pState->a[0] < -a1ul)
			pState->a[0] = -a1ul;
		else if (pState->a[0] > a1ul)
			pState->a[0] = a1ul;

		/* UPB : update predictor zeros b[6] */
		for (cnt = 0; cnt < 6; cnt++) {
			pState->b[cnt] -= pState->b[cnt] >> 8;
			if (dq & 0x7FFF) {			/* XOR */
				if ((dq ^ pState->dq[cnt]) >= 0)
					pState->b[cnt] += 128;
				else
					pState->b[cnt] -= 128;
			}
		}
	}

	for (cnt = 5; cnt > 0; cnt--)
		pState->dq[cnt] = pState->dq[cnt-1];
	/* FLOAT A : convert dq[0] to 4-bit exp, 6-bit mantissa f.p. */
	if (mag == 0) {
		pState->dq[0] = (dq >= 0) ? 0x20 : 0xFC20;
	} else {
		exp = getpower2(mag);
		pState->dq[0] = (dq >= 0) ?
		    (exp << 6) + ((mag << 6) >> exp) :
		    (exp << 6) + ((mag << 6) >> exp) - 0x400;
	}

	pState->sr[1] = pState->sr[0];
	/* FLOAT B : convert sr to 4-bit exp., 6-bit mantissa f.p. */
	if (sr == 0) {
		pState->sr[0] = 0x20;
	} else if (sr > 0) {
		exp = getpower2(sr);
		pState->sr[0] = (exp << 6) + ((sr << 6) >> exp);
	} else if (sr > -32768) {
		mag = -sr;
		exp = getpower2(mag);
		pState->sr[0] =  (exp << 6) + ((mag << 6) >> exp) - 0x400;
	} else
		pState->sr[0] = 0xFC20;

	/* DELAY A */
	pState->pk[1] = pState->pk[0];
	pState->pk[0] = pk0;

	/* TONE */
	if (tr == 1)		/* this sample has been treated as data */
		pState->td = 0;	/* next one will be treated as voice */
	else if (a2p < -11776)	/* small sample-to-sample correlation */
		pState->td = 1;	/* signal may be data */
	else				/* signal is voice */
		pState->td = 0;

	/*
	 * Adaptation speed control.
	 */
	pState->dms += (fi - pState->dms) >> 5;		/* FILTA */
	pState->dml += (((fi << 2) - pState->dml) >> 7);	/* FILTB */

	if (tr == 1)
		pState->ap = 256;
	else if (y < 1536)					/* SUBTC */
		pState->ap += (0x200 - pState->ap) >> 4;
	else if (pState->td == 1)
		pState->ap += (0x200 - pState->ap) >> 4;
	else if (abs((pState->dms << 2) - pState->dml) >=
	    (pState->dml >> 3))
		pState->ap += (0x200 - pState->ap) >> 4;
	else
		pState->ap += (-pState->ap) >> 4;
}

