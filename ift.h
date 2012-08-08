/*
 * Information Flow Tracking routines, based on "unused bits".
 */

#define TAG 0x00000001
#define TAGMASK ~TAG
#define TAINT(ptr) ((short *)((unsigned long)ptr|TAG))
#define TEST(ptr) ((unsigned long)ptr&TAG)
#define STAR(ptr) (*(short *)((unsigned long)ptr&TAGMASK))
#define IFT(ptrd, ptrs) if(TEST(ptrs)) {ptrd = TAINT(ptrd);}

static inline void taint(short *ptr)
{
	ptr = (short *)((unsigned long)ptr|TAG);
	return;
}

static inline short star(short *ptr)
{
	return (*(short *)((unsigned long)ptr&TAGMASK));
}

static inline void ift(short *ptrd, short *ptrs)
{
	if (TEST(ptrs)) {ptrd = TAINT(ptrd);}
	return;
}
