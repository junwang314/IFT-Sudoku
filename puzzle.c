//#ident "$Id: puzzle.c,v 1.9 2011/05/03 09:35:09 pwh Exp $"
/*
 * Sudoku Puzzle Solver, puzzle solution logic.
 */

#if	defined(DEBUG)
#include	<stdio.h>
extern FILE *logFile;
#endif

#include	<stdlib.h>
#include	<sys/stat.h>
#include	"puzzle.h"
#include	"twiddleBits.h"
#include	"ift.h"


HISTORY *openHistory ( void )

{
   HISTORY	*hist;

#if	defined(DEBUG)
   fprintf ( logFile, "openHistory ( void ) {\n" );
   fflush ( logFile );
#endif

   if ( hist = ( HISTORY * ) malloc ( sizeof ( HISTORY ) ) ) {

	int	i;

	hist->head = -1;

	for ( i = 0; i < 81; ++i ) {

		hist->list[i] = -1;
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %p /* openHistory () */\n", hist );
   fflush ( logFile );
#endif

   return ( hist );
}


int pushHistory ( HISTORY *hist, int row, int column )

{
   int	status = 0;
   int	i = 9 * row + column;

#if	defined(DEBUG)
   fprintf ( logFile, "pushHistory ( %p, %d, %d ) {\n", hist, row, column );
   fflush ( logFile );
#endif

   if ( i < 81 && i >= 0 && hist->list [i] == -1 ) {

	status = 1;

	hist->list[i] = hist->head;
	hist->head = i;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* pushHistory () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


CELL *popHistory ( HISTORY *hist )

{
   CELL	*coords = NULL;

#if	defined(DEBUG)
   fprintf ( logFile, "popHistory ( %p ) {\n", hist );
   fflush ( logFile );
#endif

   if ( hist->head > -1 ) {

	int	i = hist->head;

	hist->cellCoords.row = i / 9;
	hist->cellCoords.column = i % 9;
	coords = &( hist->cellCoords );
	hist->head = hist->list [i];
	hist->list [i] = -1;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %p /* popHistory () */\n", coords );
   fflush ( logFile );
#endif

   return ( coords );
}


static void flipHistory ( HISTORY *hist )

{
   int	head = hist->head;

#if	defined(DEBUG)
   fprintf ( logFile, "flipHistory ( %p ) {\n", hist );
   fflush ( logFile );
#endif

   hist->head = -1;

   while ( head > -1 ) {

	int	i = head;

	head = hist->list [i];

	hist->list [i] = hist->head;
	hist->head = i;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* flipHistory () */\n" );
   fflush ( logFile );
#endif
}


void closePuzzle ( PUZZLE *puzzle )

{
#if	defined(DEBUG)
   fprintf ( logFile, "closePuzzle ( %p ) {\n", puzzle );
   fflush ( logFile );
#endif
   if ( puzzle ) {

	if ( puzzle->history ) closeHistory ( puzzle->history );
	free ( ( void * ) puzzle );
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* closePuzzle () */\n" );
   fflush ( logFile );
#endif
}


static void copyPuzzle ( PUZZLE *src, PUZZLE *dest )

{
   int	puzzleSize = sizeof (PUZZLE);
   char	*p = ( char * ) src;
   char *q = ( char * ) dest;

#if	defined(DEBUG)
   fprintf ( logFile, "copyPuzzle ( %p, %p ) {\n", src, dest );
   fflush ( logFile );
#endif

   while ( puzzleSize-- ) *q++ = *p++;

   dest->history = NULL;

#if	defined(DEBUG)
   fprintf ( logFile, "} /* copyPuzzle () */\n" );
   fflush ( logFile );
#endif
}


static void clearPuzzle ( PUZZLE *puzzle )

{
   int	i;
   int	j;

   puzzle->technique = 0;
   puzzle->difficulty = 0;
   puzzle->history = NULL;

   for ( i = 0; i < 9; ++i ) {

	puzzle->rowMasks [i] = BIT_MASK;
	puzzle->columnMasks [i] = BIT_MASK;
	puzzle->prowMasks [i] = &(puzzle->rowMasks [i]);
	puzzle->pcolumnMasks [i] = &(puzzle->columnMasks [i]);

	for ( j = 0; j < 9; ++j ) {

		puzzle->grid [i][j] = 0;
		puzzle->masks [i][j] = BIT_MASK;
		puzzle->pgrid [i][j] = &(puzzle->grid [i][j]);
		puzzle->pmasks [i][j] = &(puzzle->masks [i][j]);
	}
   }

   for ( i = 0; i < 3; ++i )
	for ( j = 0; j < 3; ++j )
		puzzle->blockMasks [i][j] = BIT_MASK;
		puzzle->pblockMasks [i][j] = &(puzzle->blockMasks [i][j]);
}


PUZZLE *openPuzzle ( void )

{
   PUZZLE	*puzzle = NULL;

#if	defined(DEBUG)
   fprintf (  logFile, "openPuzzle () {\n" );
   fflush ( logFile );
#endif

   /* Allocate the object and initialize it. */
   if ( puzzle = (PUZZLE *) malloc ( sizeof ( PUZZLE ) ) )
							clearPuzzle ( puzzle );

#if	defined(DEBUG)
   fprintf (  logFile, "} = %p /* openPuzzle () */\n", puzzle );
   fflush ( logFile );
#endif

   return ( puzzle );
}


//static int makeEntry ( PUZZLE *puzzle, int i, int j, int entry )
static int makeEntry ( PUZZLE *puzzle, int i, int j, short *pentry )

{
   int	status = 0;
   //int	bit = digit2bit ( entry );
   short	bit = digit2bit ( star(pentry) );
   short	*pbit = &bit;
   IFT(pbit, pentry);

#if	defined(DEBUG)
   //fprintf ( logFile, "makeEntry ( %p, %d, %d, %d ) {\n", puzzle, i, j, entry );
   fprintf ( logFile, "makeEntry ( %p, %d, %d, %p ) {\n", puzzle, i, j, pentry );
   fflush ( logFile );
#endif

   /* Is this a legal entry? */
   if ( puzzle->rowMasks [i] & puzzle->columnMasks [j]
				& puzzle->blockMasks [i/3][j/3] & bit ) {
	status = 1;
	puzzle->rowMasks [i] ^= bit;
	puzzle->columnMasks [j] ^= bit;
	puzzle->blockMasks [i/3][j/3] ^= bit;
	//puzzle->grid [i][j] = entry;
	puzzle->grid [i][j] = star(pentry);
	puzzle->masks [i][j] = 0;

	IFT(puzzle->prowMasks [i], pbit);
	IFT(puzzle->pcolumnMasks [j], pbit);
	IFT(puzzle->pblockMasks [i/3][j/3], pbit);
	IFT(puzzle->pgrid [i][j], pentry);

	if ( puzzle->history ) pushHistory ( puzzle->history, i, j );
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* makeEntry () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


//static int enterClues ( PUZZLE *puzzle, char *clues )
static int enterClues ( PUZZLE *puzzle, short **pclues )

{
   int	status = 0;
   int	row = 0;
   int	column = 0;

#if	defined(DEBUG)
   //fprintf (  logFile, "enterClues ( %p, \"%s\" ) {\n", puzzle, clues );
   fprintf (  logFile, "enterClues ( %p, %p ) {\n", puzzle, pclues );
   fflush ( logFile );
#endif

   //while ( *clues ) {
   while ( star(*pclues) ) {

	//if ( isdigit ( *clues ) ) {
	if ( isdigit ( star(*pclues) ) ) {

		//int	clue = *clues - '0';
		short	clue = star(*pclues) - '0';
		short	*pclue = &clue;
		IFT(pclue, *pclues);
		//if (TEST(*pclues)) pclue = TAINT(pclue);
#if	defined(DEBUG)
   fprintf (  logFile, "EEE: %p, %p\n", pclue, *pclues );
   fflush ( logFile );
#endif

		if ( column > 8 ) {

			column = 0;
			if ( ++row > 8 ) break;
		}

		if ( clue > 0 ) {

			if ( ! ( status
				//= makeEntry ( puzzle, row, column, clue ) ) )
				= makeEntry ( puzzle, row, column, pclue ) ) )
									break;
		}

		++column;
	}

	++pclues;
   }

   if ( status ) {

	for ( row = 0; row < 9; ++row ) {

		for ( column = 0; column < 9; ++column ) {

			if ( puzzle->grid [row][column] == 0 ) {
				puzzle->masks [row][column]
				= ( puzzle->rowMasks [row]
				& puzzle->columnMasks [column]
				& puzzle->blockMasks [row/3][column/3] );
				
				IFT(puzzle->pmasks [row][column], puzzle->prowMasks [row]);			
				IFT(puzzle->pmasks [row][column], puzzle->pcolumnMasks [column]);			
				IFT(puzzle->pmasks [row][column], puzzle->pblockMasks [row/3][column/3]);			
			}
		}
	}
   }

#if	defined(DEBUG)
   fprintf (  logFile, "} = %d /* enterClues () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


//PUZZLE *openPuzzleSolver ( char *clues )
PUZZLE *openPuzzleSolver ( short **pclues )

{
   PUZZLE	*puzzle = NULL;

#if	defined(DEBUG)
   fprintf (  logFile, "openPuzzleSolver ( %p ) {\n", pclues );
   fflush ( logFile );
#endif

   /* Allocate the object and initialize it. */
   if ( puzzle = openPuzzle () ) {

	//if ( ! enterClues ( puzzle, clues ) ) {
	if ( ! enterClues ( puzzle, pclues ) ) {

		closePuzzle ( puzzle );
		puzzle = NULL;
	}

   }

#if	defined(DEBUG)
   fprintf (  logFile, "} = %p /* openPuzzleSolver () */\n", puzzle );
   fflush ( logFile );
#endif

   return ( puzzle );
}


static int countUnsolved ( PUZZLE *puzzle )

{
   int	unsolved = 0;
   int	i;

#if	defined(DEBUG)
   fprintf ( logFile, "countUnsolved ( %p ) {\n", puzzle );
   fflush ( logFile );
#endif

   for ( i = 0; i < 9; ++i ) {

	int	j;

	for ( j = 0; j < 9; ++j ) {

		if ( puzzle->grid [i][j] == 0 ) ++unsolved;
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* countUnsolved () */\n", unsolved );
   fflush ( logFile );
#endif

   return ( unsolved );
}


//static void updateRowMasks ( PUZZLE *puzzle, int row, int entry )
static void updateRowMasks ( PUZZLE *puzzle, int row, short *pentry )

{
   int	column;
   //int	mask = ~digit2bit ( entry );
   short	mask = ~digit2bit ( star(pentry) );
   short	*pmask = &mask;
   IFT(pmask, pentry);

#if	defined(DEBUG)
   //fprintf ( logFile, "updateRowMasks ( %p, %d, %d ) {\n", puzzle, row, entry );
   fprintf ( logFile, "updateRowMasks ( %p, %d, %p ) {\n", puzzle, row, pentry );
   fflush ( logFile );
#endif

   for ( column = 0; column < 9; ++column ) {

	puzzle->masks [row][column] &= mask;
	IFT(puzzle->pmasks [row][column], pmask);
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* updateRowMasks () */\n" );
   fflush ( logFile );
#endif
}


//static void updateColumnMasks ( PUZZLE *puzzle, int column, int entry )
static void updateColumnMasks ( PUZZLE *puzzle, int column, short *pentry )

{
   int	row;
   //int	mask = ~digit2bit ( entry );
   short	mask = ~digit2bit ( star(pentry) );
   short	*pmask = &mask;
   IFT(pmask, pentry);

#if	defined(DEBUG)
   //fprintf ( logFile, "updateColumnMasks ( %p, %d, %d ) {\n", puzzle, column,
   fprintf ( logFile, "updateColumnMasks ( %p, %d, %p ) {\n", puzzle, column,
									pentry );
   fflush ( logFile );
#endif

   for ( row = 0; row < 9; ++row ) {

	puzzle->masks [row][column] &= mask;
	IFT(puzzle->pmasks [row][column], pmask);
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* updateColumnMasks () */\n" );
   fflush ( logFile );
#endif
}


//static void updateBlockMasks ( PUZZLE *puzzle, int row, int column, int entry )
static void updateBlockMasks ( PUZZLE *puzzle, int row, int column, short *pentry )

{
   int	rowMax = row + 3 - ( row % 3 );
   int	columnMax = column + 3 - ( column % 3 );
   //int	mask = ~digit2bit ( entry );
   short	mask = ~digit2bit ( star(pentry) );
   short	*pmask = &mask;
   IFT(pmask, pentry);

#if	defined(DEBUG)
   //fprintf ( logFile, "updateBlockMasks ( %p, %d, %d, %d ) {\n", puzzle, row,
   fprintf ( logFile, "updateBlockMasks ( %p, %d, %d, %p ) {\n", puzzle, row,
								column, pentry );
   fflush ( logFile );
#endif

   for ( row = rowMax - 3; row < rowMax; ++row ) {

	for ( column = columnMax - 3; column < columnMax; ++column ) {

		puzzle->masks [row][column] &= mask;
		IFT(puzzle->pmasks [row][column], pmask);
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* updateBlockMasks () */\n" );
   fflush ( logFile );
#endif
}


static int missingDigitScan ( PUZZLE *puzzle )

{
   int	progress = 0;
   int	i;

#if	defined(DEBUG)
   fprintf ( logFile, "missingDigitScan ( %p ) {\n", puzzle );
   fflush ( logFile );
#endif

   puzzle->difficulty += 1;

   for ( i = 0; i < 9 && progress > -1; ++i ) {

	int	j;

	for ( j = 0; j < 9 && progress > -1; ++j ) {

		if ( puzzle->grid [i][j] == 0 ) {

			int	solution;

			if ( solution = bit2digit ( puzzle->masks [i][j] ) ) {

				++progress;
				makeEntry ( puzzle, i, j, solution );
				updateRowMasks ( puzzle, i, solution );
				updateColumnMasks ( puzzle, j, solution );
				updateBlockMasks ( puzzle, i, j, solution );

			} else if ( ! puzzle->masks [i][j] ) progress = -1;
		}
	}
   }

   if ( progress > 0 ) puzzle->technique |= MISSING_DIGIT;

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* missingDigitScan () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


//static int crossHatch ( int *masks )
static int crossHatch ( short **pmasks )

{
   int	progress = 0;
   //int	solutions [9];
   short	solutions [9];
   short	*psolutions [9];
   int	k;
   for ( k = 0; k < 9; k++ ) {
   	psolutions [k] = &solutions[k];
   }
   int	i;

#if	defined(DEBUG)
   //fprintf ( logFile, "crossHatch ( %04X ) {\n", (unsigned int)masks );
   fprintf ( logFile, "crossHatch ( %p ) {\n", pmasks );
   fflush ( logFile );
#endif

   for ( i = 0; i < 9; ++i ) {

	int	j;

	//solutions [i] = masks [i];
	solutions [i] = star(pmasks [i]);
	IFT(psolutions [i], pmasks [i]);

	//for ( j = 0; j < 9; ++j ) if ( j != i ) solutions [i] &= ~masks [j];
	for ( j = 0; j < 9; ++j ) {
		if ( j != i ) {
			solutions [i] &= ~(star(pmasks [j]));
			IFT(psolutions [i], pmasks [j]);
		}
	}
		
   }

   for ( i = 0; i < 9; ++i ) {

	//if ( masks [i] = bit2digit ( solutions [i] ) ) ++progress;
	if ( STAR(pmasks [i]) = bit2digit ( solutions [i] ) ) ++progress;
	IFT(pmasks [i], psolutions [i]);
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* crossHatch () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}
		

//static int getRowMasks ( PUZZLE *puzzle, int row, int *masks )
static int getRowMasks ( PUZZLE *puzzle, int row, short **pmasks )

{
   int	status = 0;

#if	defined(DEBUG)
   //fprintf ( logFile, "getRowMasks ( %p, %d, %p ) {\n", puzzle, row, masks );
   fprintf ( logFile, "getRowMasks ( %p, %d, %p ) {\n", puzzle, row, pmasks );
   fflush ( logFile );
#endif

   if ( puzzle->rowMasks [row] ) {

	//int	mask = 0;
	short	mask = 0;
	int	column;

	status = 1;

	/* Make masks for the row. */
	for ( column = 0; column < 9; ++column ) {

		//masks [column] = puzzle->masks [row][column];
		STAR(pmasks [column]) = puzzle->masks [row][column];
		IFT(pmasks [column], puzzle->pmasks [row][column]);
		//mask |= masks [column];
		mask |= star(pmasks [column]);
	}

	if ( mask != puzzle->rowMasks [row] ) status = -1;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* getRowMasks () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


//static int enterRowSolutions ( PUZZLE *puzzle, int row, int *solutions )
static int enterRowSolutions ( PUZZLE *puzzle, int row, short **psolutions )

{
   int	progress = 0;
   int	column;

#if	defined(DEBUG)
   //fprintf ( logFile, "enterRowSolutions ( %p, %d, %p ) {\n", puzzle, row,
								//solutions );
   fprintf ( logFile, "enterRowSolutions ( %p, %d, %p ) {\n", puzzle, row,
								psolutions );
   fflush ( logFile );
#endif

   /* Display the solutions. */
   for ( column = 0; column < 9; ++column ) {

	//if ( solutions [column] ) {
	if ( star(psolutions [column]) ) {

		//if ( makeEntry ( puzzle, row, column, solutions [column] ) ) {
		if ( makeEntry ( puzzle, row, column, psolutions [column] ) ) {

			++progress;

			updateColumnMasks ( puzzle, column,
							psolutions [column] );
			updateBlockMasks ( puzzle, row, column,
							psolutions [column] );
		}
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* enterRowSolutions () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


//static int getColumnMasks ( PUZZLE *puzzle, int column, int *masks )
static int getColumnMasks ( PUZZLE *puzzle, int column, short **pmasks )

{
   int	status = 0;

#if	defined(DEBUG)
   //fprintf ( logFile, "getColumnMasks ( %p, %d, %p ) {\n", puzzle, column,
									//masks );
   fprintf ( logFile, "getColumnMasks ( %p, %d, %p ) {\n", puzzle, column,
									pmasks );
   fflush ( logFile );
#endif

   /* Make sure the column is not already filled. */
   if ( puzzle->columnMasks [column] ) {

	//int	mask = 0;
	short	mask = 0;
	int	row;

	status = 1;

	/* Make masks for the column. */
	for ( row = 0; row < 9; ++row ) {

		//masks [row] = puzzle->masks [row][column];
		STAR(pmasks [row]) = puzzle->masks [row][column];
		IFT(pmasks [row], puzzle->pmasks [row][column]);
		//mask |= masks [row];
		mask |= star(pmasks [row]);
	}

	if ( mask != puzzle->columnMasks [column] ) status = -1;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* getColumnMasks () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


//static int enterColumnSolutions ( PUZZLE *puzzle, int column, int *solutions )
static int enterColumnSolutions ( PUZZLE *puzzle, int column, short **psolutions )

{
   int	progress = 0;
   int	row;

#if	defined(DEBUG)
   //fprintf ( logFile, "enterColumnSolutions ( %p, %d, %p ) {\n", puzzle,
							//column, solutions );
   fprintf ( logFile, "enterColumnSolutions ( %p, %d, %p ) {\n", puzzle,
							column, psolutions );
   fflush ( logFile );
#endif

   /* Enter the results of cross hatching the column. */
   for ( row = 0; row < 9; ++row ) {

	//if ( solutions [row] ) {
	if ( star(psolutions [row]) ) {

		//if ( makeEntry ( puzzle, row, column, solutions [row] ) ) {
		if ( makeEntry ( puzzle, row, column, psolutions [row] ) ) {

			++progress;

			updateRowMasks ( puzzle, row, psolutions [row] );
			updateBlockMasks ( puzzle, row, column,
							psolutions [row] );
		}
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* enterColumnSolutions () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


//static int getBlockMasks ( PUZZLE *puzzle, int row, int column, int *masks )
static int getBlockMasks ( PUZZLE *puzzle, int row, int column, short **pmasks )

{
   int	status = 0;

#if	defined(DEBUG)
   //fprintf ( logFile, "getBlockMasks ( %p, %d, %d, %p ) {\n", puzzle, row,
								//column, masks );
   fprintf ( logFile, "getBlockMasks ( %p, %d, %d, %p ) {\n", puzzle, row,
								column, pmasks );
   fflush ( logFile );
#endif

   /* Make sure the 3x3 block is not already full. */
   if ( puzzle->blockMasks [row][column] ) {

	int	rowMax = ( row + 1 ) * 3;
	int	columnMax = ( column + 1 ) * 3;
	int	k = 0;
	//int	mask = 0;
	short	mask = 0;
	int	i;

	status = 1;

	/* Make masks for the 3x3 block. */
	for ( i = rowMax - 3; i < rowMax; ++i ) {

		int	j;

		for ( j = columnMax - 3; j < columnMax; ++j ) {

			//masks [k] = puzzle->masks [i][j];
			STAR(pmasks [k]) = puzzle->masks [i][j];
			IFT(pmasks [k], puzzle->pmasks [i][j]);
			//mask |= masks [k];
			mask |= star(pmasks [k]);

			++k;
		}
	}

	if ( mask != puzzle->blockMasks [row][column] ) status = -1;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* getBlockMasks () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


static int enterBlockSolutions ( PUZZLE *puzzle, int row, int column,
								//int *solutions )
								short **psolutions )
{
   int	progress = 0;
   int	rowMax = ( row + 1 ) * 3;
   int	columnMax = ( column + 1 ) * 3;
   int	k = 0;
   int	i;

#if	defined(DEBUG)
//   fprintf ( logFile, "enterBlockSolutions ( %p, %d, %d, %p ) {\n", puzzle,
//						row, column, solutions );
   fprintf ( logFile, "enterBlockSolutions ( %p, %d, %d, %p ) {\n", puzzle,
						row, column, psolutions );
   fflush ( logFile );
#endif

   for ( i = rowMax - 3; i < rowMax; ++i ) {

	int	j;

	for ( j = columnMax - 3; j < columnMax; ++j ) {

		//if ( solutions [k] ) {
		if ( star(psolutions [k]) ) {

			//if ( makeEntry ( puzzle, i, j, solutions [k] ) ) {
			if ( makeEntry ( puzzle, i, j, psolutions [k] ) ) {

				++progress;
				updateRowMasks ( puzzle, i, psolutions [k] );
				updateColumnMasks ( puzzle, j, psolutions [k] );
			}
		}

		++k;
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* enterBlockSolutions () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


static int crossHatchScan ( PUZZLE *puzzle )

{
   int	progress = 0;
   int	i;

#if	defined(DEBUG)
   fprintf ( logFile, "crossHatchScan ( %p ) {\n", puzzle );
   fflush ( logFile );
#endif

   puzzle->difficulty += 1;

   /* Cross hatch the 3x3 blocks. */
   for ( i = 0; i < 3 && progress > -1; i++ ) {

	int	j;

	for ( j = 0; j < 3 && progress > -1; j++ ) {

		//int	masks [9];
		short	masks [9];
		short	*pmasks [9];
		int	k;
		for ( k = 0; k < 9; k++ ) {
			pmasks [k] = &masks[k];
		}

		int	status;

		//if ( ( status = getBlockMasks ( puzzle, i, j, masks ) ) > 0 ) {
		if ( ( status = getBlockMasks ( puzzle, i, j, pmasks ) ) > 0 ) {

			//if ( crossHatch ( masks ) ) {
			if ( crossHatch ( pmasks ) ) {

				progress += enterBlockSolutions ( puzzle,
								//i, j, masks );
								i, j, pmasks );
			}

		} else if ( status < 0 ) progress = -1;
	}
   }

   if ( ! progress ) {

	puzzle->difficulty += 1;

	/* Cross hatch the rows. */
	for ( i = 0; i < 9 && progress > -1; ++i ) {

		//int	masks [9];
		short	masks [9];
		short	*pmasks [9];
		int	k;
		for ( k = 0; k < 9; k++ ) {
			pmasks [k] = &masks[k];
		}
		int	status;

		//if ( ( status = getRowMasks ( puzzle, i, masks ) ) > 0 ) {
		if ( ( status = getRowMasks ( puzzle, i, pmasks ) ) > 0 ) {

			//if ( crossHatch ( masks ) ) {
			if ( crossHatch ( pmasks ) ) {

				progress += enterRowSolutions ( puzzle, i,
									//masks );
									pmasks );
			}

		} else if ( status < 0 ) progress = -1;
	}

	/* Cross hatch the columns. */
	for ( i = 0; i < 9 && progress > -1; ++i ) {

		//int	masks [9];
		short	masks [9];
		short	*pmasks [9];
		int	k;
		for ( k = 0; k < 9; k++ ) {
			pmasks [k] = &masks[k];
		}

		int	status;

		//if ( ( status = getColumnMasks ( puzzle, i, masks ) ) > 0 ) {
		if ( ( status = getColumnMasks ( puzzle, i, pmasks ) ) > 0 ) {

			//if ( crossHatch ( masks ) ) {
			if ( crossHatch ( pmasks ) ) {

				progress += enterColumnSolutions ( puzzle, i,
									//masks );
									pmasks );
			}

		} else if ( status < 0 ) progress = -1;
	}

	if ( progress > 0 ) puzzle->technique |= CROSS_HATCH_2;

   } else if ( progress > 0 ) puzzle->technique |= CROSS_HATCH;

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* crossHatchScan () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


static int findRowTuple ( PUZZLE *puzzle, int row, int mask, int bitCount )

{
   int	progress = 0;
   int	tupleMask = 0;
   int	tupleCount = 0;
   int	tupleBit = 0x100;
   int	exactCount = 0;
   int	rowMax = row + 3 - ( row % 3 );
   int	columnMax = 0;
   int	column;

#if	defined(DEBUG)
   fprintf ( logFile, "findRowTuple ( %p, %d, %04X, %d ) {\n", puzzle, row,
							mask, bitCount );
   fflush ( logFile );
#endif

   for ( column = 0; column < 9; ++column ) {

	int	cellMask = puzzle->masks [row][column];

	if ( mask & cellMask ) {

		++tupleCount;

		tupleMask |= tupleBit;

		if ( ! ( ~mask & cellMask ) ) ++exactCount;
	}

	tupleBit >>= 1;
   }

   if ( exactCount == bitCount ) {

	tupleBit = 0x100;
	tupleMask = 0;

	for ( column = 0; column < 9; ++column ) {

		int	cellMask = puzzle->masks [row][column];

		if ( cellMask ) {

			if ( ! ( ~mask & cellMask ) ) tupleMask |= tupleBit;

			else if ( mask & cellMask ) {

				puzzle->masks [row][column] &= ~mask;
				++progress;
			}
		}

		tupleBit >>= 1;
	}

	if ( progress && bitCount > 1 ) {

		puzzle->technique |= NAKED_TUPLE;
		if ( GET_NAKED_TUPLE_LEVEL ( puzzle->technique ) < bitCount )
			SET_NAKED_TUPLE_LEVEL ( puzzle->technique, bitCount );
	}

   } else if ( tupleCount == bitCount ) {

	for ( column = 0; column < 9; ++column ) {

		int	cellMask = puzzle->masks [row][column];

		if ( ( mask & cellMask ) && ( ~mask & cellMask ) ) {

			puzzle->masks [row][column] &= mask;
			++progress;
		}
	}

	if ( progress && bitCount > 1 ) {

		puzzle->technique |= HIDDEN_TUPLE;
		if ( GET_HIDDEN_TUPLE_LEVEL ( puzzle->technique ) < bitCount )
			SET_HIDDEN_TUPLE_LEVEL ( puzzle->technique, bitCount );
	}
   }

   if ( bitCount < 4 ) {

	int	dProgress = 0;

	if ( ! ( tupleMask & ~SQUARE_1 ) ) columnMax = 3;
	else if ( ! ( tupleMask & ~SQUARE_2 ) ) columnMax = 6;
	else if ( ! ( tupleMask & ~SQUARE_3 ) ) columnMax = 9;

	if ( columnMax ) {

		int	i;

		for ( i = rowMax - 3; i < rowMax; ++i ) {

			if ( i != row ) {

				for ( column = columnMax - 3;
						column < columnMax; ++column ) {

					if ( puzzle->masks [i][column]
								& mask ) {

						puzzle->masks [i][column]
								&= ~mask;
						++dProgress;
					}
				}
			}
		}
	}

	if ( dProgress ) {

		if ( bitCount == 1 ) puzzle->technique |= SET_INTERSECTION;
		else {

			puzzle->technique |= NAKED_TUPLE;
			if ( GET_NAKED_TUPLE_LEVEL ( puzzle->technique )
								< bitCount )
				SET_NAKED_TUPLE_LEVEL ( puzzle->technique,
								bitCount );
		}

		progress += dProgress;
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* findRowTuple () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


static int rowTuples ( PUZZLE *puzzle, int row, int mask, int bitCount,
								int skip )
{
   int	progress = 0;
   int	maskBits = countBits ( mask );

#if	defined(DEBUG)
   fprintf ( logFile, "rowTuples ( %p, %d, %04X, %d, %d ) {\n", puzzle, row,
							mask, bitCount, skip );
   fflush ( logFile );
#endif

   if ( maskBits > bitCount && skip < maskBits ) {

	int	digitBit = 0x100;
	int	skipped = 0;

	--maskBits;

	while ( ! progress && digitBit ) {

		if ( ( digitBit & mask ) && skipped++ == skip ) {

			int	digitMask = mask ^ digitBit;

			if ( maskBits != bitCount )
				progress += rowTuples ( puzzle, row, digitMask,
							bitCount, skip );
			else
				progress += findRowTuple ( puzzle, row,
						digitMask, bitCount );

			++skip;
		}

		digitBit >>= 1;
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* rowTuples () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}
 

static int findColumnTuple ( PUZZLE *puzzle, int column, int mask,
								int bitCount )
{
   int	progress = 0;
   int	tupleCount = 0;
   int	tupleMask = 0;
   int	tupleBit = 0x100;
   int	exactCount = 0;
   int	rowMax = 0;
   int	columnMax = column + 3 - ( column % 3 );
   int	row;

#if	defined(DEBUG)
   fprintf ( logFile, "findColumnTuple ( %p, %d, %04X, %d ) {\n", puzzle,
						column, mask, bitCount );
   fflush ( logFile );
#endif

   for ( row = 0; row < 9; ++row ) {

	int	cellMask = puzzle->masks [row][column];

	if ( mask & cellMask ) {

		++tupleCount;

		tupleMask |= tupleBit;

		if ( ! ( ~mask & cellMask ) ) ++exactCount;
	}

	tupleBit >>= 1;
   }

   if ( exactCount == bitCount ) {

	tupleMask = 0;
	tupleBit = 0x100;

	for ( row = 0; row < 9; ++row ) {

		int	cellMask = puzzle->masks [row][column];

		if ( cellMask ) {

			if ( ! ( ~mask & cellMask ) ) tupleMask |= tupleBit;

			else if ( mask & cellMask ) {

				puzzle->masks [row][column] &= ~mask;
				++progress;
			}
		}

		tupleBit >>= 1;
	}

	if ( progress && bitCount > 1 ) {

		puzzle->technique |= NAKED_TUPLE;
		if ( GET_NAKED_TUPLE_LEVEL ( puzzle->technique ) < bitCount )
			SET_NAKED_TUPLE_LEVEL ( puzzle->technique, bitCount );
	}

   } else if ( tupleCount == bitCount ) {

	for ( row = 0; row < 9; ++row ) {

		int	cellMask = puzzle->masks [row][column];

		if ( ( mask & cellMask ) && ( ~mask & cellMask ) ) {

			puzzle->masks [row][column] &= mask;
			++progress;
		}
	}

	if ( progress && bitCount > 1 ) {

		puzzle->technique |= HIDDEN_TUPLE;
		if ( GET_HIDDEN_TUPLE_LEVEL ( puzzle->technique ) < bitCount )
			SET_HIDDEN_TUPLE_LEVEL ( puzzle->technique, bitCount );
	}
   }

   if ( bitCount < 4 ) {

	int	dProgress = 0;

	if ( ! ( tupleMask & ~SQUARE_1 ) ) rowMax = 3;
	else if ( ! ( tupleMask & ~SQUARE_2 ) ) rowMax = 6;
	else if ( ! ( tupleMask & ~SQUARE_3 ) ) rowMax = 9;

	if ( rowMax ) {

		int	j;

		for ( j = columnMax - 3; j < columnMax; ++j ) {

			if ( j != column ) {

				for ( row = rowMax - 3; row < rowMax; ++row ) {

					if ( puzzle->masks [row][j] & mask ) {

						puzzle->masks [row][j] &= ~mask;
						++dProgress;
					}
				}
			}
		}
	}

	if ( dProgress ) {

		if ( bitCount == 1 ) puzzle->technique |= SET_INTERSECTION;
		else {

			puzzle->technique |= NAKED_TUPLE;
			if ( GET_NAKED_TUPLE_LEVEL ( puzzle->technique )
								< bitCount )
				SET_NAKED_TUPLE_LEVEL ( puzzle->technique,
								bitCount );
		}

		progress += dProgress;
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* findColumnTuple () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


static int columnTuples ( PUZZLE *puzzle, int column, int mask, int bitCount,
								int skip )
{
   int	progress = 0;
   int	maskBits = countBits ( mask );

#if	defined(DEBUG)
   fprintf ( logFile, "columnTuples ( %p, %d, %04X, %d, %d ) {\n", puzzle,
						column, mask, bitCount, skip );
   fflush ( logFile );
#endif

   if ( maskBits > bitCount && skip < maskBits ) {

	int	digitBit = 0x100;
	int	skipped = 0;

	--maskBits;

	while ( ! progress && digitBit ) {

		if ( ( digitBit & mask ) && skipped++ == skip ) {

			int	digitMask = mask ^ digitBit;

			if ( maskBits != bitCount )
				progress += columnTuples ( puzzle, column,
						digitMask, bitCount, skip );
			else
				progress += findColumnTuple ( puzzle, column,
						digitMask, bitCount );

			++skip;
		}

		digitBit >>= 1;
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* columnTuples () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}
 

static findBlockTuple ( PUZZLE *puzzle, int top, int left, int mask,
								int bitCount )
{
   int	progress = 0;
   int	rowMax = ( top + 1 ) * 3;
   int	columnMax = ( left + 1 ) * 3;
   int	tupleCount = 0;
   int	tupleMask = 0;
   int	tupleBit = 0x100;
   int	exactCount = 0;
   int	i = -1;
   int	j = -1;
   int	row;

#if	defined(DEBUG)
   fprintf ( logFile, "findBlockTuple ( %p, %d, %d, %04X, %d ) {\n", puzzle,
						top, left, mask, bitCount );
   fflush ( logFile );
#endif

   for ( row = rowMax - 3; row < rowMax; ++row ) {

	int	column;

	for ( column = columnMax - 3; column < columnMax; ++column ) {

		int	cellMask = puzzle->masks [row][column];

		if ( mask & cellMask ) {

			++tupleCount;

			tupleMask |= tupleBit;

			if ( ! ( ~mask & cellMask ) ) ++exactCount;
		}

		tupleBit >>= 1;
	}
   }

   if ( exactCount == bitCount ) {

	tupleMask = 0;
	tupleBit = 0x100;

	for ( row = rowMax - 3; row < rowMax; ++row ) {

		int	column;

		for ( column = columnMax - 3; column < columnMax; ++column ) {

			int	cellMask = puzzle->masks [row][column];

			if ( cellMask ) {


				if ( ! ( ~mask & cellMask ) )
							tupleMask |= tupleBit;

				else if ( mask & cellMask ) {

					puzzle->masks [row][column] &= ~mask;
					++progress;
				}
			}

			tupleBit >>= 1;
		}
	}

	if ( progress && bitCount > 1 ) {

		puzzle->technique |= NAKED_TUPLE;
		if ( GET_NAKED_TUPLE_LEVEL ( puzzle->technique ) < bitCount )
			SET_NAKED_TUPLE_LEVEL ( puzzle->technique, bitCount );
	}

   } else if ( tupleCount == bitCount ) {

	for ( row = rowMax - 3; row < rowMax; ++row ) {

		int	column;

		for ( column = columnMax - 3; column < columnMax; ++column ) {

			int	cellMask = puzzle->masks [row][column];

			if ( ( mask & cellMask ) && ( ~mask & cellMask ) ) {

				puzzle->masks [row][column] &= mask;
				++progress;
			}
		}
	}

	if ( progress && bitCount > 1 ) {

		puzzle->technique |= HIDDEN_TUPLE;
		if ( GET_HIDDEN_TUPLE_LEVEL ( puzzle->technique ) < bitCount )
			SET_HIDDEN_TUPLE_LEVEL ( puzzle->technique, bitCount );
	}
   }

   if ( bitCount < 4 ) {

	int	dProgress = 0;

	if ( ! ( tupleMask & ~ROW_1 ) ) i = rowMax - 3;
	else if ( ! ( tupleMask & ~ROW_2 ) ) i = rowMax - 2;
	else if ( ! ( tupleMask & ~ROW_3 ) ) i = rowMax - 1;
	else if ( ! ( tupleMask & ~COLUMN_1 ) ) j = columnMax - 3;
	else if ( ! ( tupleMask & ~COLUMN_2 ) ) j = columnMax - 2;
	else if ( ! ( tupleMask & ~COLUMN_3 ) ) j = columnMax - 1;

	if ( i > -1 ) {

		int	jMax = columnMax - 3;

		for ( j = 0; j < jMax; ++j ) {

			if ( puzzle->masks [i][j] & mask ) {

				puzzle->masks [i][j] &= ~mask;
				++dProgress;
			}
		}

		for ( j = columnMax; j < 9; ++j ) {

			if ( puzzle->masks [i][j] & mask ) {

				puzzle->masks [i][j] &= ~mask;
				++dProgress;
			}
		}

	} else if ( j > -1 ) {

		int	iMax = rowMax - 3;

		for ( i = 0; i < iMax; ++i ) {

			if ( puzzle->masks [i][j] & mask ) {

				puzzle->masks [i][j] &= ~mask;
				++dProgress;
			}
		}

		for ( i = rowMax; i < 9; ++i ) {

			if ( puzzle->masks [i][j] & mask ) {

				puzzle->masks [i][j] &= ~mask;
				++dProgress;
			}
		}
	}
	if ( dProgress ) {

		if ( bitCount == 1 ) puzzle->technique |= SET_INTERSECTION;
		else {

			puzzle->technique |= NAKED_TUPLE;
			if ( GET_NAKED_TUPLE_LEVEL ( puzzle->technique )
								< bitCount )
				SET_NAKED_TUPLE_LEVEL ( puzzle->technique,
								bitCount );
		}

		progress += dProgress;
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* findBlockTuple () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


static int blockTuples ( PUZZLE *puzzle, int top, int left, int mask,
							int bitCount, int skip )
{
   int	progress = 0;
   int	maskBits = countBits ( mask );

#if	defined(DEBUG)
   fprintf ( logFile, "blockTuples ( %p, %d, %d, %04X, %d, %d ) {\n", puzzle,
					top, left, mask, bitCount, skip );
   fflush ( logFile );
#endif

   if ( maskBits > bitCount && skip < maskBits ) {

	int	digitBit = 0x100;
	int	skipped = 0;

	--maskBits;

	while ( digitBit ) {

		if ( ( digitBit & mask ) && skipped++ == skip ) {

			int	digitMask = mask ^ digitBit;

			if ( maskBits != bitCount )
				progress += blockTuples ( puzzle, top,
					left, digitMask, bitCount, skip );
			else
				progress += findBlockTuple ( puzzle,
					top, left, digitMask, bitCount );

			++skip;
		}

		digitBit >>= 1;
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* blockTuples () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}
 

static int tuplesAnalysis ( PUZZLE *puzzle )

{
   int	progress = 0;
   int	bitCount = 1;	/* Start at 1 to cover virtual cross-hatching. */
   int	i;

#if	defined(DEBUG)
   fprintf ( logFile, "tuplesAnalysis ( %p ) {\n", puzzle );
   fflush ( logFile );
#endif

   while ( ! progress && bitCount < 5 ) {

	puzzle->difficulty += 1;

	for ( i = 0; i < 9; ++i ) {

		int	mask = puzzle->rowMasks [i];

		if ( mask ) progress += rowTuples ( puzzle, i, mask,
								bitCount, 0 );
	}

	for ( i = 0; i < 9; ++i ) {

		int	mask = puzzle->columnMasks [i];

		if ( mask ) progress += columnTuples ( puzzle, i, mask,
								bitCount, 0 );
	}

	for ( i = 0; i < 3; ++i ) {

		int	j;

		for ( j = 0; j < 3; ++j ) {

			int	mask = puzzle->blockMasks [i][j];

			if ( mask ) progress += blockTuples ( puzzle, i, j,
							mask, bitCount, 0 );
		}
	}

	++bitCount;
   }

   if ( ! progress ) puzzle->difficulty += 4;

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* tuplesAnalysis () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


static int contradictions ( PUZZLE *puzzle )

{
   int	progress = 0;
   int	minBitCount = 9;
   int	row;
   int	column;
   int	i;
   int	bit = 0x100;
   int	ambiguous = 0;
   int	mask;

#if	defined(DEBUG)
   fprintf ( logFile, "contradictions ( %p ) {\n", puzzle );
   fflush ( logFile );
#endif

   puzzle->difficulty += 1;

   /* Scan for a cell with a minimal candidate set. */
   for ( i = 0; ! progress && i < 9; i++ ) {

	int	j;

	for ( j = 0; ! progress && j < 9; j++ ) {

		int	bits = 0;

		if ( puzzle->masks [i][j]
			&& ( bits = countBits ( puzzle->masks [i][j] ) )
									== 2 ) {

			row = i;
			column = j;
			progress = 1;

		} else if ( bits > 2 && bits < minBitCount ) {

			minBitCount = bits;
			row = i;
			column = j;
		}
	}
   }

   mask = puzzle->masks [row][column];

   while ( bit ) {

	if ( bit & mask ) {

		PUZZLE	puzzle2;
		int	entry = bit2digit ( bit );
		int	solutions;

		copyPuzzle ( puzzle, &puzzle2 );
		makeEntry ( &puzzle2, row, column, entry );
		updateRowMasks ( &puzzle2, row, entry );
		updateColumnMasks ( &puzzle2, column, entry );
		updateBlockMasks ( &puzzle2, row, column, entry );

		if ( ! ( solutions = solvePuzzle ( &puzzle2, NULL ) ) ) {

			puzzle->masks [row][column] ^= bit;
			puzzle->technique = puzzle2.technique;
			progress = 1;
			break;

		} else if ( ambiguous || solutions > 1 ) {

			int r;

			for ( r = 0; r < 9; ++r ) {

				int c;

				for ( c = 0; c < 9; ++c ) {

					puzzle->grid [r][c]
						= puzzle2.grid [r][c];
				}
			}

			puzzle->difficulty = puzzle2.difficulty;
			progress = -1;
			break;

		} else ++ambiguous;
	}

	bit >>= 1;
   }

   if ( progress > 0 ) puzzle->technique |= WHAT_IF;

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* contradictions () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


int solvePuzzle ( PUZZLE *puzzle, HISTORY *hist )

{
   int	solutions;
   int	progress = 1;
   int	i;

#if	defined(DEBUG)
   fprintf ( logFile, "solvePuzzle ( %p, %p ) {\n", puzzle, hist );
   fflush ( logFile );
#endif

   puzzle->history = hist;
   solutions = countUnsolved ( puzzle );

   while ( solutions && progress > 0 ) {

	if ( ( progress = crossHatchScan ( puzzle ) )
			|| ( progress = missingDigitScan ( puzzle ) ) ) {

		if ( progress > 0 ) solutions -= progress;
		else {

			solutions = -1;
			progress = 0;
		}

	} else if ( ! ( progress = tuplesAnalysis ( puzzle ) ) )
					progress = contradictions ( puzzle );
   }

   if ( puzzle->history ) flipHistory ( puzzle->history );
   if ( ( solutions += 1 ) > 2 ) solutions = 2;

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* solvePuzzle () */\n", solutions );
   fflush ( logFile );
#endif

   return ( solutions );
}
