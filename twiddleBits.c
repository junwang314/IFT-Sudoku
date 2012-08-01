#ident "$Id: twiddleBits.c,v 1.1 2011/04/16 07:04:50 pwh Exp $"
/*
 * Bit twiddling routines.
 */

#include	"twiddleBits.h"


char b2dLookup [] = {	( char )  1,	( char )  2,	( char )  3,
			( char ) 11,	( char ) 15,	( char )  4,
			( char ) 12,	( char )  6,	( char ) 16,
			( char ) 10,	( char ) 14,	( char )  5,
			( char )  9,	( char ) 13,	( char )  8,
			( char )  7 };


int d2bLookup [] = {	0x0001, 0x0002, 0x0004, 0x0008,
			0x0010, 0x0020, 0x0040, 0x0080,
			0x0100, 0x0200, 0x0400, 0x0800,
			0x1000, 0x2000, 0x4000, 0x8000,
	0x00010000, 0x00020000, 0x00040000, 0x00080000,
	0x00100000, 0x00200000, 0x00400000, 0x00800000,
	0x01000000, 0x02000000, 0x04000000, 0x08000000,
	0x10000000, 0x20000000, 0x40000000, 0x80000000 };


int countBits ( unsigned int bits )

{
   bits -= ( bits >> 1 ) & 0x55555555;
   bits = ( bits & 0x33333333 ) + ( ( bits >> 2 ) & 0x33333333 );
   bits = ( bits + ( bits >> 4 ) ) & 0xF0F0F0F;
   bits += ( bits >> 8 );

   return ( ( bits + ( bits >> 16 ) ) & 0xFF );
}
