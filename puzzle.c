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

	for ( j = 0; j < 9; ++j ) {

		puzzle->grid [i][j] = 0;
		puzzle->masks [i][j] = BIT_MASK;
	}
   }

   for ( i = 0; i < 3; ++i )
	for ( j = 0; j < 3; ++j )
		puzzle->blockMasks [i][j] = BIT_MASK;
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


static int makeEntry ( PUZZLE *puzzle, int i, int j, int entry )

{
   int	status = 0;
   int	bit = digit2bit ( entry );

#if	defined(DEBUG)
   fprintf ( logFile, "makeEntry ( %p, %d, %d, %d ) {\n", puzzle, i, j, entry );
   fflush ( logFile );
#endif

   /* Is this a legal entry? */
   if ( puzzle->rowMasks [i] & puzzle->columnMasks [j]
				& puzzle->blockMasks [i/3][j/3] & bit ) {
	status = 1;
	puzzle->rowMasks [i] ^= bit;
	puzzle->columnMasks [j] ^= bit;
	puzzle->blockMasks [i/3][j/3] ^= bit;
	puzzle->grid [i][j] = entry;
	puzzle->masks [i][j] = 0;

	if ( puzzle->history ) pushHistory ( puzzle->history, i, j );
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* makeEntry () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


static int enterClues ( PUZZLE *puzzle, char *clues )

{
   int	status = 0;
   int	row = 0;
   int	column = 0;

#if	defined(DEBUG)
   fprintf (  logFile, "enterClues ( %p, \"%s\" ) {\n", puzzle, clues );
   fflush ( logFile );
#endif

   while ( *clues ) {

	if ( isdigit ( *clues ) ) {

		int	clue = *clues - '0';

		if ( column > 8 ) {

			column = 0;
			if ( ++row > 8 ) break;
		}

		if ( clue > 0 ) {

			if ( ! ( status
				= makeEntry ( puzzle, row, column, clue ) ) )
									break;
		}

		++column;
	}

	++clues;
   }

   if ( status ) {

	for ( row = 0; row < 9; ++row ) {

		for ( column = 0; column < 9; ++column ) {

			if ( puzzle->grid [row][column] == 0 )
				puzzle->masks [row][column]
				= ( puzzle->rowMasks [row]
				& puzzle->columnMasks [column]
				& puzzle->blockMasks [row/3][column/3] );
		}
	}
   }

#if	defined(DEBUG)
   fprintf (  logFile, "} = %d /* enterClues () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


PUZZLE *openPuzzleSolver ( char *clues )

{
   PUZZLE	*puzzle = NULL;

#if	defined(DEBUG)
   fprintf (  logFile, "openPuzzleSolver ( %s ) {\n", clues );
   fflush ( logFile );
#endif

   /* Allocate the object and initialize it. */
   if ( puzzle = openPuzzle () ) {

	if ( ! enterClues ( puzzle, clues ) ) {

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


static void updateRowMasks ( PUZZLE *puzzle, int row, int entry )

{
   int	column;
   int	mask = ~digit2bit ( entry );

#if	defined(DEBUG)
   fprintf ( logFile, "updateRowMasks ( %p, %d, %d ) {\n", puzzle, row, entry );
   fflush ( logFile );
#endif

   for ( column = 0; column < 9; ++column ) {

	puzzle->masks [row][column] &= mask;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* updateRowMasks () */\n" );
   fflush ( logFile );
#endif
}


static void updateColumnMasks ( PUZZLE *puzzle, int column, int entry )

{
   int	row;
   int	mask = ~digit2bit ( entry );

#if	defined(DEBUG)
   fprintf ( logFile, "updateColumnMasks ( %p, %d, %d ) {\n", puzzle, column,
									entry );
   fflush ( logFile );
#endif

   for ( row = 0; row < 9; ++row ) {

	puzzle->masks [row][column] &= mask;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* updateColumnMasks () */\n" );
   fflush ( logFile );
#endif
}


static void updateBlockMasks ( PUZZLE *puzzle, int row, int column, int entry )

{
   int	rowMax = row + 3 - ( row % 3 );
   int	columnMax = column + 3 - ( column % 3 );
   int	mask = ~digit2bit ( entry );

#if	defined(DEBUG)
   fprintf ( logFile, "updateBlockMasks ( %p, %d, %d, %d ) {\n", puzzle, row,
								column, entry );
   fflush ( logFile );
#endif

   for ( row = rowMax - 3; row < rowMax; ++row ) {

	for ( column = columnMax - 3; column < columnMax; ++column ) {

		puzzle->masks [row][column] &= mask;
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


static int crossHatch ( int *masks )

{
   int	progress = 0;
   int	solutions [9];
   int	i;

#if	defined(DEBUG)
   fprintf ( logFile, "crossHatch ( %04X ) {\n", (unsigned int)masks );
   fflush ( logFile );
#endif

   for ( i = 0; i < 9; ++i ) {

	int	j;

	solutions [i] = masks [i];

	for ( j = 0; j < 9; ++j ) if ( j != i ) solutions [i] &= ~masks [j];
   }

   for ( i = 0; i < 9; ++i ) {

	if ( masks [i] = bit2digit ( solutions [i] ) ) ++progress;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* crossHatch () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}
		

static int getRowMasks ( PUZZLE *puzzle, int row, int *masks )

{
   int	status = 0;

#if	defined(DEBUG)
   fprintf ( logFile, "getRowMasks ( %p, %d, %p ) {\n", puzzle, row, masks );
   fflush ( logFile );
#endif

   if ( puzzle->rowMasks [row] ) {

	int	mask = 0;
	int	column;

	status = 1;

	/* Make masks for the row. */
	for ( column = 0; column < 9; ++column ) {

		masks [column] = puzzle->masks [row][column];
		mask |= masks [column];
	}

	if ( mask != puzzle->rowMasks [row] ) status = -1;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* getRowMasks () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


static int enterRowSolutions ( PUZZLE *puzzle, int row, int *solutions )

{
   int	progress = 0;
   int	column;

#if	defined(DEBUG)
   fprintf ( logFile, "enterRowSolutions ( %p, %d, %p ) {\n", puzzle, row,
								solutions );
   fflush ( logFile );
#endif

   /* Display the solutions. */
   for ( column = 0; column < 9; ++column ) {

	if ( solutions [column] ) {

		if ( makeEntry ( puzzle, row, column, solutions [column] ) ) {

			++progress;

			updateColumnMasks ( puzzle, column,
							solutions [column] );
			updateBlockMasks ( puzzle, row, column,
							solutions [column] );
		}
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* enterRowSolutions () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


static int getColumnMasks ( PUZZLE *puzzle, int column, int *masks )

{
   int	status = 0;

#if	defined(DEBUG)
   fprintf ( logFile, "getColumnMasks ( %p, %d, %p ) {\n", puzzle, column,
									masks );
   fflush ( logFile );
#endif

   /* Make sure the column is not already filled. */
   if ( puzzle->columnMasks [column] ) {

	int	mask = 0;
	int	row;

	status = 1;

	/* Make masks for the column. */
	for ( row = 0; row < 9; ++row ) {

		masks [row] = puzzle->masks [row][column];
		mask |= masks [row];
	}

	if ( mask != puzzle->columnMasks [column] ) status = -1;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* getColumnMasks () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


static int enterColumnSolutions ( PUZZLE *puzzle, int column, int *solutions )

{
   int	progress = 0;
   int	row;

#if	defined(DEBUG)
   fprintf ( logFile, "enterColumnSolutions ( %p, %d, %p ) {\n", puzzle,
							column, solutions );
   fflush ( logFile );
#endif

   /* Enter the results of cross hatching the column. */
   for ( row = 0; row < 9; ++row ) {

	if ( solutions [row] ) {

		if ( makeEntry ( puzzle, row, column, solutions [row] ) ) {

			++progress;

			updateRowMasks ( puzzle, row, solutions [row] );
			updateBlockMasks ( puzzle, row, column,
							solutions [row] );
		}
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* enterColumnSolutions () */\n", progress );
   fflush ( logFile );
#endif

   return ( progress );
}


static int getBlockMasks ( PUZZLE *puzzle, int row, int column, int *masks )

{
   int	status = 0;

#if	defined(DEBUG)
   fprintf ( logFile, "getBlockMasks ( %p, %d, %d, %p ) {\n", puzzle, row,
								column, masks );
   fflush ( logFile );
#endif

   /* Make sure the 3x3 block is not already full. */
   if ( puzzle->blockMasks [row][column] ) {

	int	rowMax = ( row + 1 ) * 3;
	int	columnMax = ( column + 1 ) * 3;
	int	k = 0;
	int	mask = 0;
	int	i;

	status = 1;

	/* Make masks for the 3x3 block. */
	for ( i = rowMax - 3; i < rowMax; ++i ) {

		int	j;

		for ( j = columnMax - 3; j < columnMax; ++j ) {

			masks [k] = puzzle->masks [i][j];
			mask |= masks [k];

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
								int *solutions )
{
   int	progress = 0;
   int	rowMax = ( row + 1 ) * 3;
   int	columnMax = ( column + 1 ) * 3;
   int	k = 0;
   int	i;

#if	defined(DEBUG)
   fprintf ( logFile, "enterBlockSolutions ( %p, %d, %d, %p ) {\n", puzzle,
						row, column, solutions );
   fflush ( logFile );
#endif

   for ( i = rowMax - 3; i < rowMax; ++i ) {

	int	j;

	for ( j = columnMax - 3; j < columnMax; ++j ) {

		if ( solutions [k] ) {

			if ( makeEntry ( puzzle, i, j, solutions [k] ) ) {

				++progress;
				updateRowMasks ( puzzle, i, solutions [k] );
				updateColumnMasks ( puzzle, j, solutions [k] );
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

		int	masks [9];
		int	status;

		if ( ( status = getBlockMasks ( puzzle, i, j, masks ) ) > 0 ) {

			if ( crossHatch ( masks ) ) {

				progress += enterBlockSolutions ( puzzle,
								i, j, masks );
			}

		} else if ( status < 0 ) progress = -1;
	}
   }

   if ( ! progress ) {

	puzzle->difficulty += 1;

	/* Cross hatch the rows. */
	for ( i = 0; i < 9 && progress > -1; ++i ) {

		int	masks [9];
		int	status;

		if ( ( status = getRowMasks ( puzzle, i, masks ) ) > 0 ) {

			if ( crossHatch ( masks ) ) {

				progress += enterRowSolutions ( puzzle, i,
									masks );
			}

		} else if ( status < 0 ) progress = -1;
	}

	/* Cross hatch the columns. */
	for ( i = 0; i < 9 && progress > -1; ++i ) {

		int	masks [9];
		int	status;

		if ( ( status = getColumnMasks ( puzzle, i, masks ) ) > 0 ) {

			if ( crossHatch ( masks ) ) {

				progress += enterColumnSolutions ( puzzle, i,
									masks );
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
