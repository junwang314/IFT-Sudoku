//#ident "$Id: puzzle.h,v 1.4 2011/05/02 22:59:25 pwh Exp $"
/*
 * Sudoku Puzzle Solver, puzzle solution logic.
 */
#define DEBUG

#if	defined(DEBUG)
#include	<stdio.h>
extern FILE *logFile;
#endif

/* Technique flags. */
#define		NAKED_TUPLE_LEVEL	0xF
#define		HIDDEN_TUPLE_LEVEL	0xF0
#define		CROSS_HATCH		0x100
#define		CROSS_HATCH_2		0x200
#define		MISSING_DIGIT		0x400
#define		NAKED_TUPLE		0x800
#define		HIDDEN_TUPLE		0x1000
#define		SET_INTERSECTION	0x2000
#define		WHAT_IF			0x4000
#define		SET_NAKED_TUPLE_LEVEL(a,b) ((a)=((a)&~NAKED_TUPLE_LEVEL)|(b))
#define		GET_NAKED_TUPLE_LEVEL(a) ((a)&NAKED_TUPLE_LEVEL)
#define		SET_HIDDEN_TUPLE_LEVEL(a,b) ((a)=((a)&~HIDDEN_TUPLE_LEVEL)|((b)<<4))
#define		GET_HIDDEN_TUPLE_LEVEL(a) (((a)&HIDDEN_TUPLE_LEVEL)>>4)


/*
 * Cell Coordinates.
 */
typedef struct {

	int	row;
	int	column;
} CELL;


/*
 * Solution history object.
 */
typedef struct {

	int	head;
	int	list [81];
	CELL	cellCoords;

} HISTORY;


HISTORY *openHistory ();
#define	closeHistory(deadMeat)	free((void*)deadMeat)
int	pushHistory ( HISTORY *hist, int row, int col );
CELL	*popHistory ( HISTORY *hist );


/*
 * Sudoku puzzle object.
 */
typedef struct {

	int	difficulty;
	int	technique;
	char	grid [9][9];
	short	masks [9][9];
	short	rowMasks [9];
	short	columnMasks [9];
	short	blockMasks [3][3];
	HISTORY	*history;

} PUZZLE;


#define		BIT_MASK	0x1FF

#define		SQUARE_1	0x1C0
#define		SQUARE_2	0x038
#define		SQUARE_3	0x007
#define		ROW_1		SQUARE_1
#define		ROW_2		SQUARE_2
#define		ROW_3		SQUARE_3
#define		COLUMN_1	0x124
#define		COLUMN_2	0x092
#define		COLUMN_3	0x049


/* Puzzle solution function. */
int	solvePuzzle ( PUZZLE *, HISTORY * );

PUZZLE	*openPuzzleSolver ( char * );
void	closePuzzle ( PUZZLE * );
