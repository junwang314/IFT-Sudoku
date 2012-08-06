//#ident "$Id: cursesGui.h,v 1.3 2011/04/16 18:45:47 pwh Exp $"
/*
 * Sudoku Puzzle Solver, curses GUI logic.
 */

#include	<curses.h>

#define		PUZZLE_TITLE	"Sudoku Solver"

/* Useful character definitions. */
#define		ESC		27	/* Escape character. */
#define		HT		0x09	/* Horizontal tab. */
#define		CR		0x0D	/* Carriage return. */
#define		DEL		0x7F	/* Delete. */


/* Flag bits. */
#define		BIG		0x1

/* Test flag bits. */
#define		isBig(a)	((a)->flags&BIG)


typedef struct {

	int	y0;	/* Upper left hand corner of game board. */
	int	x0;	/* Upper left hand corner of game board. */
	int	flags;	/* Big display. */
	int	delay;
	short	rowMasks [9];
	short	columnMasks [9];
	short	blockMasks [3][3];
	char	board [9][9];

} DISPLAY;


DISPLAY	*openDisplay ( int );


extern const char	*gameName;
extern const char	*instructions1;
extern const char	*instructions2;
extern const char	*quit;
extern const char	*thinking;
extern const char	*paused;
extern const char	*stumped;
extern const char	*noSolution;
extern const char	*solved;
extern const char	*toExit;
extern const char	*solveAnyway;


int	confirm ( DISPLAY *, const char * );
void	displayCaptions ( DISPLAY *, const char *, const char * );
int	displayEntry ( DISPLAY *, int, int, int );
