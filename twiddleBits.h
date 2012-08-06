//#ident "$Id: twiddleBits.h,v 1.2 2011/04/16 19:25:42 pwh Exp $"
/*
 * Bit twiddling routines.
 */

extern char b2dLookup [];
extern int d2bLookup [];

#define bitIndex16(x) ((int)b2dLookup[((((x)&0xFFFF)*0xBCD)>>12)&0xF])
#define bit2digit(x) ((x)&&(x)==((x)&-(x))?((unsigned)(x)>0xFFFF?\
bitIndex16((x)>>16)+16:bitIndex16(x)):0)
#define digit2bit(x) ((x)>0&&(x)<33?d2bLookup[(x)-1]:0)

int	countBits ( unsigned int );
