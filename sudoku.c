#ident "$Id: sudoku.c,v 1.26 2011/05/03 09:37:51 pwh Exp $"
/*
 * Sudoku Puzzle Solver.
 */

#include        <stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"cursesGui.h"
#include	"puzzle.h"

#if	defined(DEBUG)
FILE *logFile = NULL;
#endif


static char * tupleNames [] = { "pair", "triplet", "quadruplet", "quintuplet",
				"sextuplet", "septuplet", "octuplet" };

static char tupleNameBuffer [100];

static void displaySolution ( DISPLAY *display, PUZZLE *puzzle )

{
   int	row;
   int	delay = display->delay;

   display->delay = 0;

   for ( row = 0; row < 9; ++row ) {

	int	column;

	for ( column = 0; column < 9; ++column ) {

		if ( display->board [row][column] == 0 )
					displayEntry ( display, row, column,
						puzzle->grid [row][column] );
	}
   }

   display->delay = delay;
}

static char *printTuple ( int tuple )

{
   char	*tupleName = tupleNameBuffer;

   if ( tuple > 1 && tuple < 9 ) tupleName = tupleNames [tuple-2];
   else sprintf ( tupleNameBuffer, "%d", tuple );

   return ( tupleName );
}

void printHelp ( char *programName ) 

{
   fprintf ( stderr, "\nUse:  %s [-afsnhi]\n\nWhere:\n", programName );

   fprintf ( stderr, "\t-a Displays only the analysis and difficuly score.\n" );
   fprintf ( stderr, "\t-f Fast solution.\n" );
   fprintf ( stderr, "\t-s Slow solution.\n" );
   fprintf ( stderr, "\t-n No delay solution.\n" );
   fprintf ( stderr, "\t-h Help screen.\n" );
   fprintf ( stderr, "\t-i Instruction screen.\n" );
}

void printInstructions ( void )

{
   fprintf ( stderr, "\nEnter clues (digits 1-9).  Use arrow keys,\n" );
   fprintf ( stderr, "space, backspace, tab and enter keys to\n" );
   fprintf ( stderr, "position the cursor.  Press 's' to end\n" );
   fprintf ( stderr, "entering clues and start the solution\nlogic.\n" );
   fprintf ( stderr, "\nDuring the solution stage, press any key\n" );
   fprintf ( stderr, "to pause and then press any key to resume.\n" );
   fprintf ( stderr, "Press 'q' at any time to quit.\n" );
}


#define	DEFAULT_DELAY	1000
#define	FAST_DELAY	500
#define	SLOW_DELAY	2000

int main ( int argc, char **argv )

{
   int		status = 1;
   DISPLAY	*display;
   char		*programName;
   char		*revision;
   int		delay = DEFAULT_DELAY;
   int		analysis = 0;
   int		warning = 0;
   int		help = 0;

#if	defined(DEBUG)
   char		*logFileName;

   logFileName = (char *) malloc ( strlen ( argv [0] ) + 5 );
   strcpy ( logFileName, argv [0] );
   strcpy ( logFileName + strlen ( logFileName ), ".log" );
   logFile = fopen ( logFileName, "w" );
#endif

   programName = *argv;

   revision = strdup ( "$Revision: 1.26 $" );
   revision += 1;
   revision [ strlen ( revision ) - 1 ] = '\0';

   while ( *++argv ) {

	if ( **argv == '-' ) {

		int	index = 1;

		while ( argv [0][index] ) {

			switch ( argv [0][index] ) {

			  case 'a':

				if ( delay != DEFAULT_DELAY ) {

					if ( ! ( warning & 2 ) ) {

						fprintf ( stderr,
"Warning - speed setting has no effect when analysis option selected.\n" );

						warning |= 2;
					}
				}

				analysis = 1;
				break;

			  case 'f':

				if ( analysis ) {

					if ( ! ( warning & 2 ) ) {

						fprintf ( stderr, 
"Warning - speed setting has no effect when analysis option selected.\n" );

						warning |= 2;
					}

				} else if ( delay == DEFAULT_DELAY ) delay
								= FAST_DELAY;
				else if ( ! ( warning & 1 ) ) {

					warning |= 1;
					fprintf ( stderr,
	"Warning - speed already set, discarding last speed setting.\n" );
				}

				break;

			  case 's':

				if ( analysis ) {

					if ( ! ( warning & 2 ) ) {

						fprintf ( stderr, 
"Warning - speed setting has no effect when analysis option selected.\n" );

						warning |= 2;
					}

				} else if ( delay == DEFAULT_DELAY ) delay
								= SLOW_DELAY;
				else if ( ! ( warning & 1 ) ) {

					warning |= 1;
					fprintf ( stderr,
	"Warning - speed already set, discarding last speed setting.\n" );
				}

				break;

			  case 'n':

				if ( analysis ) {

					if ( ! ( warning & 2 ) ) {

						fprintf ( stderr, 
"Warning - speed setting has no effect when analysis option selected.\n" );

						warning |= 2;
					}

				} else if ( delay == DEFAULT_DELAY ) delay = 0;
				else if ( ! ( warning & 1 ) ) {

					warning |= 1;
					fprintf ( stderr,
	"Warning - speed already set, discarding last speed setting.\n" );
				}

				break;

			  case 'h':

				help |= 1;
				break;

			  case 'i':

				help |= 2;
				break;

			  default:

				warning += 4;
				fprintf ( stderr,
				"Unknown option letter (-%c) ignored.\n",
							argv [0][index] );
				break;
			}

			++index;
		}

	} else fprintf ( stderr,
			"Illegal command line parameter (%s) ignored.\n",
									*argv );
   }

   if ( warning ) sleep ( ( ( warning + 3 ) >> 1 ) + 1 );

   if ( help ) {

	fprintf ( stderr, "%s\n%s\n", PUZZLE_TITLE, revision );
	if ( help & 1 ) printHelp ( programName );
	if ( help & 2 ) printInstructions ();
	status = 0;

   } else if ( display = openDisplay ( delay ) ) {

	PUZZLE	*puzzle;
	int	technique = 0;
	int	difficulty = 0;
	char	clues[82];
	int	row;
	int	i = 0;

	for ( row = 0; row < 9; ++row ) {

		int	column;

		for ( column = 0; column < 9; ++column ) {

			clues [i++] = '0' + display->board [row] [column];
		}
	}

	clues [i] = '\0';

	if ( puzzle = openPuzzleSolver ( clues ) ) {

		HISTORY	*hist = NULL;

		if ( analysis ) {

			display->flags = 0;
			display->delay = 0;

		} else hist = openHistory ();

		status = solvePuzzle ( puzzle, hist );

		if ( status == 1 ) {

			difficulty = puzzle->difficulty;
			technique = puzzle->technique;
		}

		if ( hist ) {

			CELL	*coords;
			int	go = 1;

			if ( status > 1 )

			if ( display->delay )
				displayCaptions ( display, thinking, "" );

			while ( go && ( coords = popHistory ( hist ) ) ) {

				int	row = coords->row;
				int	column = coords->column;
				int	entry = puzzle->grid [row][column];

				go = displayEntry ( display, row, column,
									entry );
			}

			if ( go ) {

				if ( status == 1 )
					displayCaptions ( display, solved, "" );

				else if ( status > 1 ) {

					int	c;

					displayCaptions ( display, stumped,
									"" );
					c = confirm ( display, solveAnyway );

					if ( c == 'Y' || c == 'y' ) {

						displaySolution ( display,
								puzzle );
					} else go = 0;

					displayCaptions ( display, stumped,
									"" );

				} else displayCaptions ( display, noSolution,
									"" );
				if ( go ) confirm ( display, toExit );
			}
		}

		closePuzzle ( puzzle );
	}

	closeDisplay ( display );

	if ( status == 1 ) {

		fprintf ( stdout, "Solution techniques used:\n" );
		if ( technique & CROSS_HATCH ) fprintf ( stdout,
					"\tBlock cross hatch scan\n" );
		if ( technique & CROSS_HATCH_2 ) fprintf ( stdout,
				"\tRow/column cross hatch scan\n" );
		if ( technique & MISSING_DIGIT ) fprintf ( stdout,
					"\tMissing digit scan\n" );
		if ( technique & SET_INTERSECTION ) fprintf ( stdout,
				"\tRegion intersection analysis\n" );
		if ( technique & NAKED_TUPLE ) fprintf ( stdout,
				"\tNaked %s analysis\n",
			printTuple ( GET_NAKED_TUPLE_LEVEL ( technique ) ) );
		if ( technique & HIDDEN_TUPLE ) fprintf ( stdout,
				"\tHidden %s analysis\n",
			printTuple ( GET_HIDDEN_TUPLE_LEVEL ( technique ) ) );
		if ( technique & WHAT_IF ) fprintf ( stdout,
			"\tWhat if analysis\n" );
		fprintf ( stdout, "Difficulty score = %d\n", difficulty );

	} else if ( analysis ) {

		if ( ! status ) fprintf ( stdout, "No solution possible.\n" );
		else  fprintf ( stdout, "Multiple solutions possible.\n" );
	}
   }

   free ( revision - 1 );

#if	defined(DEBUG)
   fclose ( logFile );
   free ( logFileName );
#endif

   return ( status ^ 1 );
}
