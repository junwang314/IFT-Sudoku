//#ident "$Id: sudokuCGI.c,v 1.7 2011/05/02 23:07:35 pwh Exp $"
/*
 * Sudoku Puzzle Solver.
 */

#include        <stdio.h>
#include	<stdlib.h>
#include	<string.h>

#include	"cgi.h"
#include	"puzzle.h"

#if	defined(DEBUG)
FILE *logFile = NULL;
#endif


int main ( int argc, char **argv, char **envp )

{
   int		status = 0;
   PUZZLE	*puzzle;
   CGIparams	*params = NULL;
   char		*clues = NULL;

#if	defined(DEBUG)
   char		*logFileName;

   logFileName = (char *) malloc ( strlen ( argv [0] ) + 5 );
   strcpy ( logFileName, argv [0] );
   strcpy ( logFileName + strlen ( logFileName ), ".log" );
   logFile = fopen ( logFileName, "w" );

#endif

   if ( argc > 1 ) clues = *++argv;
   else {

	params = openCGIparams ( envp );
	if ( params ) {

		if ( clues = getCGIparam ( params, "clues" ) )
				printf ( "Content-type: text/plain\n\n" );
	}
   }

   while ( clues ) {

	if ( puzzle = openPuzzleSolver ( clues ) ) {

		status = solvePuzzle ( puzzle, NULL );

		fprintf ( stdout, "%d\n", status );

		if ( status ) {

			int	row;

			fprintf ( stdout, "%d\n", puzzle->difficulty );

			if ( status > 1 ) {

				fprintf ( stdout, "Ambiguous\n" );

			} else if ( puzzle->technique & WHAT_IF ) {

				fprintf ( stdout, "Expert\n" );

			} else if ( puzzle->technique
				& ( NAKED_TUPLE | HIDDEN_TUPLE ) ) {

				fprintf ( stdout, "Hard\n" );

			} else if ( puzzle->technique & SET_INTERSECTION ) {

				fprintf ( stdout, "Medium\n" );

			} else fprintf ( stdout, "Easy\n" );

			for ( row = 0; row < 9; ++row ) {

				int	column;

				for ( column = 0; column < 9; ++column ) {

					fputc ( puzzle->grid [row][column]
								+ '0', stdout );
				}

				fputc ( '\n', stdout );
			}
		}

		closePuzzle ( puzzle );

	} else fprintf ( stdout, "0\n" );

	if ( params ) clues = NULL;
	else clues = *++argv;
   }

   if ( params ) closeCGIparams ( params );

#if	defined(DEBUG)
   fclose ( logFile );
   free ( logFileName );
#endif

   return ( status );
}
