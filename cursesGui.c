//#ident "$Id: cursesGui.c,v 1.5 2011/05/03 09:34:06 pwh Exp $"
/*
 * Sudoku Puzzle Solver, curses GUI logic.
 */
#define DEBUG

#if	defined(DEBUG)
#include	<stdio.h>
extern FILE *logFile;
#endif

#include	<sys/stat.h>
#include	<stdlib.h>
#include	<curses.h>	/* Use the ncurses library. */
#include	"cursesGui.h"
#include	"twiddleBits.h"
#include	<string.h>
#include	"ift.h"

#define		BIT_MASK	0x1FF

const char	*gameName = PUZZLE_TITLE;
const char	*instructions1 = "Enter clues";
const char	*instructions2 = "'s' = solve";
const char	*quit = "Quit (Y/N)? ";
const char	*thinking = "Thinking...";
const char	*paused = "Paused...";
const char	*stumped = "Multiple Solutions";
const char	*noSolution = "No Solution!";
const char	*solved = "Ta Daaaa!!!";
const char	*toExit = "Press any key ...";
const char	*solveAnyway = "Solve (Y/N)?";


int confirm ( DISPLAY *display, const char *message )

{
   int  y = display->y0 - 1;
   int	x;
   int	c;
   int	stringLength = strlen ( message );

#if	defined(DEBUG)
   fprintf ( logFile, "confirm ( %p, \"%s\" ) {\n", display, message );
   fflush ( logFile );
#endif

   x = display->x0 + ( ( isBig ( display ) ? 19 : 13 ) - stringLength ) / 2;

   move ( y, x );

   while ( *message ) addch ( *message++ );

   refresh ();

   while ( ( c = getch () ) == ERR );

   move ( y, 0 );

   clrtoeol ();

#if	defined(DEBUG)
   fprintf ( logFile, "} = '%c' /* confirm () */\n", c );
   fflush ( logFile );
#endif

   return ( c );
}


void displayCaptions ( DISPLAY *display, const char *line1, const char *line2 )

{
   int		y;
   const char	*c;

#if	defined(DEBUG)
   fprintf ( logFile, "displayCaptions ( %p, \"%s\", \"%s\" ) {\n", display,
								line1, line2 );
   fflush ( logFile );
#endif

   y = display->y0 + ( isBig ( display ) ? 20 : 14 );

   if ( c = line1 ) {

	int	stringLength;

	move ( y, 0 );
	clrtoeol ();

	if ( stringLength = strlen ( c ) ) {

		int	x;

		x = display->x0 + ( ( isBig ( display ) ? 19 : 13 )
							- stringLength ) / 2;
		move ( y, x );
		while ( *c ) addch ( *c++ );
	}
   }

   ++y;

   if ( c = line2 ) {

	int	stringLength;

	move ( y, 0 );
	clrtoeol ();

	if ( stringLength = strlen ( c ) ) {

		int	x;

		x = display->x0 + ( ( isBig ( display ) ? 19 : 13 )
							- stringLength ) / 2;
		move ( y, x );
		while ( *c ) addch ( *c++ );
	}
   }

   move ( display->y0 + 1, display->x0 + 1 );

#if	defined(DEBUG)
   fprintf ( logFile, "} /* displayCaptions () */\n" );
   fflush ( logFile );
#endif
}


//int displayEntry ( DISPLAY *display, int i, int j, int entry )
int displayEntry ( DISPLAY *display, int i, int j, short *pentry )

{
   int	status = 1;
   int	y;
   int	x;

#if	defined(DEBUG)
   fprintf ( logFile, "displayEntry ( %p, %d, %d, %d ) {\n", display, i, j,
									//entry );
									star(pentry) );
   fflush ( logFile );
#endif

   display->board [i][j] = star(pentry);

   if ( isBig ( display ) ) {

	x = ( j << 1 ) + 1;
	y = ( i << 1 ) + 1;

   } else {

	x = ( ( j << 2 ) + 3 ) / 3;
	y = ( ( i << 2 ) + 3 ) / 3;
   }

   if ( display->delay ) {

	int	c;

	timeout ( display->delay );

	if ( ( c = getch () ) != ERR ) {

		timeout ( -1 );

		if ( c == 'q' || c == 'Q' ) {

			c = confirm ( display, quit );

			if ( c == 'y' || c == 'Y' ) {

				display->flags = 0;
				display->delay = 0;
				status = 0;
			}

		} else {

			displayCaptions ( display, paused, "" );

			if ( ( c = getch () ) == 'q' || c == 'Q' ) {

				c = confirm ( display, quit );

				if ( c == 'y' || c == 'Y' ) {

					display->flags = 0;
					display->delay = 0;
					status = 0;

				} else c = ' ';

			} else c = ' ';

			if ( c == ' ' ) displayCaptions ( display, thinking,
									"" );
		}

	} else timeout ( -1 );
   }

   move ( display->y0 + y, display->x0 + x );
   //addch ( ( entry + '0' ) | A_UNDERLINE );
   if ( TEST(pentry) ) {
	addch ( ( star(pentry) + '0' ) | A_UNDERLINE | A_STANDOUT );
   }
   else {
	addch ( ( star(pentry) + '0' ) | A_UNDERLINE );
   }
#if	defined(DEBUG)
   fprintf ( logFile, "DDD: %p, %lx, %d\n", pentry, (unsigned long)pentry, star(pentry));
   fflush ( logFile );
#endif
   move ( display->y0 + y, display->x0 + x );

   if ( display->delay ) refresh ();

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* displayEntry () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


static void deleteClue ( DISPLAY *display, int i, int j )

{
   int	entry = display->board [i][j];

#if	defined(DEBUG)
   fprintf ( logFile, "deleteClue ( %p, %d, %d ) {\n", display, i, j );
   fflush ( logFile );
#endif

   if ( entry != 0 ) {

	int	bit = digit2bit ( entry );

	display->rowMasks [i] |= bit;
	display->columnMasks [j] |= bit;
	display->blockMasks [i/3][j/3] |= bit;
	display->board [i][j] = 0;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* deleteClue () */\n" );
   fflush ( logFile );
#endif
}


static int enterClue ( DISPLAY *display, int i, int j, int entry )

{
   int	status = 1;

#if	defined(DEBUG)
   fprintf ( logFile, "enterClue ( %p, %d, %d, %d ) {\n", display, i, j,
									entry );
   fflush ( logFile );
#endif

   /* Is this a necessary entry? */
   if ( entry != display->board [i][j] ) {

	int	bit = digit2bit ( entry );

	status = 0;

	/* Is this a legal entry? */
	if ( display->rowMasks [i] & display->columnMasks [j]
				& display->blockMasks [i/3][j/3] & bit ) {

		deleteClue ( display, i, j );

		status = 1;
		display->rowMasks [i] ^= bit;
		display->columnMasks [j] ^= bit;
		display->blockMasks [i/3][j/3] ^= bit;
		//displayEntry ( display, i, j, entry );
		displayEntry ( display, i, j, &entry );
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* enterClue () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


static int getClues ( DISPLAY *display )

{
   int	status = -1;
   int	y = 1;
   int	x = 1;
   int  y_count = 0;
   int  x_count = 0;

#if	defined(DEBUG)
   fprintf ( logFile, "getClues ( %p ) {\n", display );
   fflush ( logFile );
#endif

   while ( status < 0 ) {

	int	c;

	switch ( c = getch () ) {

	  case 's':	/* Solve. */
	  case 'S':	/* Solve. */

		c = 0;
		status = 1;
		break;

	  case 'q':	/* Quit. */
	  case 'Q':	/* Quit. */
	  case ESC:	/* Quit. */

		c = confirm ( display, quit );

		if ( c == 'y' || c == 'Y' ) {

			c = 0;
			status = 0;
		}

		break;

	  case KEY_NPAGE:

		if ( isBig ( display ) ) {

			if ( ( y += 6 ) > 17 ) y -= 18;

		} else {

			if ( ( y += 4 ) > 11 ) y -= 12;
		}

		break;

	  case KEY_PPAGE:

		if ( isBig ( display ) ) {

			if ( ( y -= 6 ) < 1 ) y += 18;

		} else {

			if ( ( y -= 4 ) < 1 ) y += 12;
		}

		break;

	  case KEY_HOME:

		x = 1;
		break;

	  case KEY_END:

		x = isBig ( display ) ? 17 : 11;
		break;

	  case KEY_DC:	/* DEL */

		break;

	  default:

		if ( isdigit ( c ) && c != '0' ) {

			int	i;
			int	j;

			if ( isBig ( display ) ) {

				i = ( y - 1 ) >> 1;
				j = ( x - 1 ) >> 1;

			} else {

				i = ( y * 3 ) >> 2;
				j = ( x * 3 ) >> 2;
			}

			if ( enterClue ( display, i, j, c - '0' ) ) {

				addch ( c | A_BOLD );

			} else {

				beep ();
				break;
			}

		} else if ( isspace ( c ) || c == '-' ) {

			if ( c == HT ) {

				int	remainder;

				if ( isBig ( display ) )
						remainder = 5 - ( x % 6 );
				else remainder = 3 - ( x % 4 );

				x_count = 2;

				x += remainder;

			} else if ( c == CR ) {

				x = isBig ( display ) ? 17 : 11;
				x_count = 2;

			} else {

				int	i;
				int	j;

				if ( isBig ( display ) ) {

					i = ( y - 1 ) >> 1;
					j = ( x - 1 ) >> 1;

				} else {

					i = ( y * 3 ) >> 2;
					j = ( x * 3 ) >> 2;
				}

				deleteClue ( display, i, j );
				addch ( '-' );
			}

		} else {

			c = 0;
			beep ();
			break;
		}

	  case KEY_RIGHT:

		if ( isBig ( display ) ) {

			if ( x == 17 ) x = 1;

			else {

				x += 2;
				break;
			}

		} else {

			if ( x == 11 ) {

				x = 1;
				x_count = 0;

			} else {

				x_count += 4;
				while ( x_count > 2 ) {

					x_count -= 3;
					x++;
				}

				break;
			}
		}

	  case KEY_DOWN:

		if ( isBig ( display ) ) {

			if ( y == 17 ) y = 1;
			else y += 2;

		} else {

			if ( y == 11 ) {

				y = 1;
				y_count = 0;

			} else {

				y_count += 4;
				while ( y_count > 2 ) {

					y_count -= 3;
					y++;
				}
			}
		}

		break;

	  case KEY_BACKSPACE:
	  case KEY_LEFT:

		if ( isBig ( display ) ) {

			if ( x == 1 ) x = 17;

			else {

				x -= 2;
				break;
			}

		} else {

			if ( x == 1 ) {

				x = 11;
				x_count = 2;

			} else {

				x_count -= 4;
				while ( x_count < 0 ) {

					x_count += 3;
					--x;
				}

				break;
			}
		}

	  case KEY_UP:

		if ( isBig ( display ) ) {

			if ( y == 1 ) y = 17;
			else y -= 2;

		} else {

			if ( y == 1 ) {

				y_count = 2;
				y = 11;

			} else {

				y_count -= 4;
				while ( y_count < 0 ) {

					y_count += 3;
					--y;
				}
			}
		}

		break;
	}

	move ( display->y0 + y, display->x0 + x );

	if ( c == KEY_BACKSPACE || c == KEY_DC ) {

		int	i;
		int	j;

		if ( isBig ( display ) ) {

			i = ( y - 1 ) >> 1;
			j = ( x - 1 ) >> 1;

		} else {

			i = ( y * 3 ) >> 2;
			j = ( x * 3 ) >> 2;
		}

		deleteClue ( display, i, j );
		addch ( '-' );
		move ( display->y0 + y, display->x0 + x );
	}

	refresh ();
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* getClues () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


static void drawTitle ( DISPLAY *display )

{
#if	defined(DEBUG)
   fprintf ( logFile, "drawTitle ( %p ) {\n", display );
   fflush ( logFile );
#endif

   int	titleLen = strlen ( gameName );
   int	x;

   x = display->x0 + ( ( isBig ( display ) ? 19 : 13 ) - titleLen ) / 2;

   move ( display->y0 - 2, x );

   x = 0;

   while ( x < titleLen ) addch ( gameName [ x++] | A_BOLD | A_UNDERLINE );

#if	defined(DEBUG)
   fprintf ( logFile, "} /* drawTitle () */\n" );
   fflush ( logFile );
#endif
}


static DISPLAY *drawBoard ( void )

{
   DISPLAY	*display = NULL;
   int		y;
   int		x;

#if	defined(DEBUG)
   fprintf ( logFile, "drawBoard ( void ) {\n" );
   fflush ( logFile );
#endif

   getmaxyx ( stdscr, y, x );

   if ( y < 18 || x < 18 ) {

	if ( ! isendwin () ) endwin ();

	fprintf ( stderr,
	"Your screen is not big enough to display the Sudoku puzzle.\n" );

   } else if ( display = malloc ( sizeof ( DISPLAY ) ) ) {

	int	yBorder;
	int	bigBox;
	int	littleBox;

	display->flags = ( y < 24 || x < 20 ) ? 0 : BIG;

	if ( isBig ( display ) ) {

		display->y0 = ( ( y - 20 ) >> 1 );
		display->x0 = ( ( x - 20 ) >> 1 );

		yBorder = littleBox = 5;
		bigBox = 19;

	} else {

		display->y0 = ( ( y - 14 ) >> 1 );
		display->x0 = ( ( x - 14 ) >> 1 );

		yBorder = littleBox = 3;
		bigBox = 13;
	}

	clear ();
	drawTitle ( display );

	for ( y = 0; y < bigBox; ++y ) {

		int	xBorder = littleBox;

		move ( display->y0 + y, display->x0 );

		if ( yBorder == littleBox ) {

			for ( x = 0; x < bigBox; ++x ) {

				if ( xBorder == littleBox ) { 

					int	c = ACS_PLUS;

					if ( y == 0 ) {

						c = ACS_TTEE;

						if ( x == 0 ) c = ACS_ULCORNER;
						else if ( x == ( bigBox - 1 ) )
							c = ACS_URCORNER;

					} else if ( y == ( bigBox - 1 ) ) {

						c = ACS_BTEE;

						if ( x == 0 ) c = ACS_LLCORNER;
						else if ( x == ( bigBox - 1 ) )
							c = ACS_LRCORNER;

					} else {


						if ( x == 0 ) c = ACS_LTEE;
						else if ( x == ( bigBox - 1 ) )
								c = ACS_RTEE;
					}

					addch ( c | A_BOLD );
					xBorder = 0;

				} else {

					addch ( ACS_HLINE | A_BOLD );
					++xBorder;
				}
			}

			yBorder = 0;

		} else {

			for ( x = 0; x < bigBox; ++x ) {

				if ( xBorder == littleBox ) {

					addch ( ACS_VLINE | A_BOLD );
					xBorder = 0;

				} else {

					int	c = '-';

					if ( isBig ( display ) && ( ! ( x & 1 )
							|| ! ( y & 1 ) ) )
									c = ' ';
					addch ( c );
					++xBorder;
				}
			}

			++yBorder;
		}
	}

	displayCaptions ( display, instructions1, instructions2 );
	move ( display->y0 + 1, display->x0 + 1 );
	refresh ();
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %p /* drawBoard () */\n", display );
   fflush ( logFile );
#endif

   return ( display );
}


void closeDisplay ( DISPLAY *display )

{
#if	defined(DEBUG)
   fprintf ( logFile, "closeDisplay ( %p ) {\n", display );
   fflush ( logFile );
#endif

   if ( display ) {

	if ( ! isendwin () ) endwin ();
	free ( display );
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* closeDisplay () */\n" );
   fflush ( logFile );
#endif
}


DISPLAY *openDisplay ( int delay )

{
   DISPLAY	*display = NULL;
   struct stat	st_buff;

#if	defined(DEBUG)
   fprintf ( logFile, "openDisplay ( %d ) {\n", delay );
   fflush ( logFile );
#endif

   /* Check if standard out is a terminal. */
   if ( fstat ( 1, &st_buff ) < 0 ) {

	perror ( "Standard out not available" );

   /* Test if standard out is a terminal. */
   } else if ( S_ISCHR ( st_buff.st_mode ) ) {

	if ( initscr () ) {

		cbreak ();
		timeout ( -1 );
		noecho ();
		nonl ();
		intrflush ( stdscr, FALSE );
		keypad ( stdscr, TRUE );

		/* Draw the board and allocate the object. */
		if ( display = drawBoard () ) {

			int	i;
			int	j;

			display->delay = 0;

			for ( i = 0; i < 9; ++i ) {

				display->rowMasks [i] = BIT_MASK;
				display->columnMasks [i] = BIT_MASK;

				for ( j = 0; j < 9; ++j ) {

					display->board [i][j] = 0;
				}
			}

			for ( i = 0; i < 3; ++i )
				for ( j = 0; j < 3; ++j )
					display->blockMasks [i][j] = BIT_MASK;

			if ( ! getClues ( display ) ) {

				closeDisplay ( display );
				display = NULL;

			} else display->delay = delay;
		}

	} else fprintf ( stderr, "Cannot take control of terminal.\n" );

   } else fprintf ( stderr, "Standard out not a terminal" );

#if	defined(DEBUG)
   fprintf ( logFile, "} = %p /* openDisplay () */\n", display );
   fflush ( logFile );
#endif

   return ( display );
}
