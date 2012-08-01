#ident "$Id: cgi.c,v 1.6 2011/04/16 07:02:28 pwh Exp $"
/*
 * Sudoku Puzzle Solver, CGI interface logic.
 */

#if	defined(DEBUG)
#include	<stdio.h>
extern FILE *logFile;
#endif

#include	<stdlib.h>
#include	<string.h>
#include	<ctype.h>
#include	"cgi.h"

static const char *environ [] = {

	"AUTH_TYPE",
	"CONTENT_LENGTH",
	"CONTENT_TYPE",
	"GATEWAY_INTERFACE",
	"HTTP_ACCEPT",
	"HTTP_ACCEPT_LANGUAGE",
	"HTTP_COOKIE",
	"HTTP_USER_AGENT",
	"PATH_INFO",
	"PATH_TRANSLATED",
	"QUERY_STRING",
	"REMOTE_ADDR",
	"REMOTE_HOST",
	"REMOTE_IDENT",
	"REMOTE_USER",
	"REQUEST_METHOD",
	"SCRIPT_NAME",
	"SERVER_NAME",
	"SERVER_PROTOCOL",
	"SERVER_PORT",
	"SERVER_SOFTWARE"
};

#define	AUTH_TYPE		 0
#define	CONTENT_LENGTH		 1
#define	CONTENT_TYPE		 2
#define	GATEWAY_INTERFACE	 3
#define	HTTP_ACCEPT		 4
#define	HTTP_ACCEPT_LANGUAGE	 5
#define	HTTP_COOKIE		 6
#define	HTTP_USER_AGENT		 7
#define	PATH_INFO		 8
#define	PATH_TRANSLATED		 9
#define	QUERY_STRING		10
#define	REMOTE_ADDR		11
#define	REMOTE_HOST		12
#define	REMOTE_IDENT		13
#define	REMOTE_USER		14
#define	REQUEST_METHOD		15
#define	SCRIPT_NAME		16
#define	SERVER_NAME		17
#define	SERVER_PROTOCOL		18
#define	SERVER_PORT		19
#define	SERVER_SOFTWARE		20

#define	ENVIRON_COUNT		21


static int envcmp ( const unsigned char *p, const unsigned char *q,
					const unsigned char **environValue )
{
   int	diff;

#if	defined(DEBUG)
   fprintf ( logFile, "envcmp ( \"%s\", \"%s\" ) {\n", p, q );
   fflush ( logFile );
#endif

   *environValue = NULL;

   while ( ! ( diff = *p - *q ) && *p != '=' && *q ) {

	++p;
	++q;
   }

   if ( diff && *p == '=' && ! *q ) {

	diff = 0;
	*environValue = ++p;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* envcmp () */\n", diff );
   fflush ( logFile );
#endif

   return ( diff );
}


static int findEnv ( const char *name, const char **environValues )

{
   int			top = ENVIRON_COUNT;
   int			bottom = 0;
   int			i = ( top + bottom ) >> 1;
   int			found = -1;
   const unsigned char	*environValue = NULL;

#if	defined(DEBUG)
   fprintf ( logFile, "findEnv ( \"%s\" ) {\n", name );
   fflush ( logFile );
#endif

   while ( top > bottom ) {

	int	diff = envcmp ( name, environ [i], &environValue );

	if ( diff > 0 ) {

		bottom = i + 1;

	} else if ( diff < 0 ) {

		top = i;

	} else {

		found = i;
		break;
	}

	i = ( top + bottom ) >> 1;
   }

   if ( found > -1 && *environValue ) environValues [found] = environValue;

#if	defined(DEBUG)
   fprintf ( logFile, "} = %s /* findEnv () */\n", found < 0 ? "not found"
							: environ [found] );
   fflush ( logFile );
#endif

   return ( found );
}


static int readEnv ( char **envp, const char **environValues )

{
   int i;
   int status = 0;

#if	defined(DEBUG)
   fprintf ( logFile, "readEnv ( %p ) {\n", envp );
   fflush ( logFile );
#endif

   /* Initialize the environValues array. */
   for ( i = 0; i < ENVIRON_COUNT; ++i ) environValues [i] = NULL;

   while ( *envp ) {

	if ( findEnv ( *envp, environValues ) > -1 ) ++status;

	++envp;
   }

   if ( environValues [GATEWAY_INTERFACE] == NULL ) status = 0;
   else {

	const char *p = environValues [GATEWAY_INTERFACE];

	while ( isspace ( *p ) ) ++p;
	
	if ( toupper ( p[0] ) != 'C' || toupper ( p[1] ) != 'G'
				|| toupper ( p[2] ) != 'I' ) status = 0;
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %d /* readEnv () */\n", status );
   fflush ( logFile );
#endif

   return ( status );
}


static char *decodeParams ( char *code )

{
   int			strLen = 0;
   unsigned char	*p = code;
   unsigned char 	*decodedString = NULL;

#if	defined(DEBUG)
   fprintf ( logFile, "decodeParams ( \"%s\" ) {\n", code );
   fflush ( logFile );
#endif

   while ( *p ) {

	if ( ! ( isspace ( *p ) ) ) {

		++strLen;

		if ( *p == '%' ) {

			if ( isxdigit ( p [1] ) ) {

				++p;

				if ( isxdigit ( p [1] ) ) ++p;
			}
		}
	}

	++p;
   }

   if ( strLen > 0 && ( decodedString = malloc ( strLen + 1 ) ) ) {

	char	*q = decodedString;

	p = code;

	while ( *p ) {

		if ( ! isspace ( *p ) ) {

			if ( *p == '%' ) {

				int	ch;
				int	digit;

				if ( isxdigit ( p [1] ) ) {

					++p;

					if ( isdigit ( *p ) ) digit = *p - '0';
					else digit = toupper ( *p ) - 'A' + 10;

					ch = digit;

					if ( isxdigit ( p [1] ) ) {

						++p;

						if ( isdigit ( *p ) ) digit
								= *p - '0';
						else digit = toupper ( *p )
								- 'A' + 10;

						ch <<= 4;
						ch += digit;
					}

					*q++ = ch;

				} else *q++ = *p;

			} else if ( *p == '+' ) {

				*q++ = ' ';

			} else *q++ = *p;

		}

		++p;
	}

	*q = '\0';
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = \"%s\" /* decodeParams () */\n", decodedString );
   fflush ( logFile );
#endif

   return ( decodedString );
}


static void insertHeap ( CGIparams *params, int i )

{
   int	newName = params->sorted [i];

#if	defined(DEBUG)
   fprintf ( logFile, "insertHeap ( %p, %d ) {\n", params, i );
   fflush ( logFile );
#endif

   while ( i != 0 ) {

	int	parent = ( ( i + 1 ) >> 1 ) - 1;

	if ( strcmp ( params->names [newName],
			params->names [params->sorted [parent]] ) > 0 ) {

		params->sorted [i] = params->sorted [parent];
		i = parent;

	} else break;
   }

   params->sorted [i] = newName;

#if	defined(DEBUG)
   fprintf ( logFile, "} /* insertHeap () */\n" );
   fflush ( logFile );
#endif
}


static void emptyHeap ( CGIparams *params, int last )

{
   int	root = params->sorted [0];
   int	parent = 0;
   int	leftChild = 1;
   int	rightChild = 2;

#if	defined(DEBUG)
   fprintf ( logFile, "emptyHeap ( %p, %d ) {\n", params, last );
   fflush ( logFile );
#endif

   while ( leftChild <= last ) {

	if ( rightChild > last
		|| strcmp ( params->names [params->sorted [leftChild]],
			params->names [params->sorted [rightChild]] ) > 0 ) {

		params->sorted [parent] = params->sorted [leftChild];
		parent = leftChild;

	} else {

		params->sorted [parent] = params->sorted [rightChild];
		parent = rightChild;
	}

	rightChild = ( ( parent + 1 ) << 1 ) - 1;
	leftChild = rightChild - 1;
   }

   if ( parent != last ) {

	params->sorted [parent] = params->sorted [last];
	insertHeap ( params, parent );
   }

   params->sorted [last] = root;

#if	defined(DEBUG)
   fprintf ( logFile, "} /* emptyHeap () */\n" );
   fflush ( logFile );
#endif
}


static void sortParams ( CGIparams *params )

{
   int  i;

#if	defined(DEBUG)
   fprintf ( logFile, "sortParams ( %p ) {\n", params );
   fflush ( logFile );
#endif

   params->sorted [0] = 0;

   for ( i = 1; i < params->paramCount; ++i ) {

	params->sorted [i] = i;
	insertHeap ( params, i );
   }

   for ( i = params->paramCount - 1; i > 0; --i ) emptyHeap ( params, i );

#if	defined(DEBUG)
   fprintf ( logFile, "} /* sortParams () */\n" );
   fflush ( logFile );
#endif
}


static CGIparams *createCGIparams ( char *rawParams )

{
   CGIparams	*params = NULL;
   int		paramCount = 0;
   char		*p = rawParams;

#if	defined(DEBUG)
   fprintf ( logFile, "createCGIparams ( (%p) \"%s\" ) {\n", rawParams,
						rawParams ? rawParams : "" );
   fflush ( logFile );
#endif

   if ( p ) {

	/* Count the parameters */
	while ( *p ) {

		int	length = 0;
		char	*q = p;

		while ( *p != '&' && *p ) {

			++length;
			++p;
		}

		if ( length > 0 ) {

			length = 0;

			while ( *q != '=' && *q != '&' && *q ) {

				length++;
				++q;
			}

			if ( length > 0 && *q == '=' && *++q && *q != '&' )
								paramCount++;
		}

		if ( *p == '&' ) ++p;
	}
   }

   if ( ( params = ( CGIparams * ) malloc ( sizeof ( CGIparams )
			+ 2 * ( paramCount + 1 ) * sizeof ( char * )
				+ paramCount * sizeof ( int ) ) ) ) {

	params->paramCount = paramCount;

	params->names = ( char ** ) ( ( ( void * ) params )
					+ sizeof ( CGIparams ) );

	params->values = params->names + ( paramCount + 1 );

	if ( paramCount ) params->sorted = ( int * )
			( params->values + ( paramCount + 1 ) );
	else params->sorted = NULL;

	params->names [paramCount] = NULL;
	params->values [paramCount] = NULL;

	params->rawParams = rawParams;

	if ( paramCount ) {

		p = rawParams;
		paramCount = 0;

		while ( *p ) {

			int	length = 0;
			char	*name = p;
			char	*q = p;
			char	nameEnd;
			char	valueEnd;

			while ( *p != '&' && *p ) {

				++length;
				++p;
			}

			if ( length > 0 ) {

				valueEnd = *p;
				*p = '\0';

				length = 0;

				while (  *q != '=' && *q != '&' && *q ) {

					length++;
					++q;
				}

				if ( length > 0 ) {

					char	*value = ( q + 1 );

					nameEnd = *q;
					*q = '\0';

					if ( nameEnd == '=' && *value ) {

						params->names [paramCount]
							= strdup ( name );
						params->values [paramCount]
						= decodeParams ( value );
						++paramCount;
					}

					*q = nameEnd;
				}

				*p = valueEnd;
			}

			if ( *p == '&' ) ++p;
		}

		sortParams ( params );
	}
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} = %p /* createCGIparams () */\n", params );
   fflush ( logFile );
#endif

   return ( params );
}


static char *readStdin ( int length )

{
   char 	*buffer = NULL;
   char		*p;
   size_t	buff_size = 255;
   size_t	buff_used = 0;

#if	defined(DEBUG)
   fprintf ( logFile, "readStdin ( %d ) {\n", length );
   fflush ( logFile );
#endif

   if ( length ) buff_size = length;

   while ( p = realloc ( buffer, buff_size + 1 ) ) {

	size_t	bytesRead = 0;

	buffer = p;
	p += buff_used;

	while ( buff_used < buff_size ) {

		if ( ( bytesRead = read ( 0, p, buff_size - buff_used ) )
									> 0 ) {

			buff_used += bytesRead;
			p += bytesRead;

		} else {

			if ( bytesRead < 0 ) p = NULL;
			break;
		}
	}

	if ( bytesRead > 0 && length == 0 ) {

		buff_size += ( buff_size + 1 );

	} else break;
   }

   if ( ! p || ( length > 0 && buff_used != length ) ) {

	if ( buffer ) {

		free ( buffer );
		buffer = NULL;
	}

   } else if ( buffer ) buffer [buff_used] = '\0';

#if	defined(DEBUG)
   fprintf ( logFile, "} = %p /* readStdin () */\n", buffer );
   fflush ( logFile );
#endif

   return ( buffer );
}


void closeCGIparams ( CGIparams *deadMeat )

{
   int	i;

#if	defined(DEBUG)
   fprintf ( logFile, "closeCGIparams ( %p ) {\n", deadMeat );
   fflush ( logFile );
#endif

   if ( deadMeat ) {

	for ( i = 0; i < deadMeat->paramCount; ++i ) {

		free ( deadMeat->names [i] );
		free ( deadMeat->values [i] );
	}

	if ( deadMeat->rawParams ) free ( deadMeat->rawParams );

	free ( deadMeat );
   }

#if	defined(DEBUG)
   fprintf ( logFile, "} /* closeCGIparams () */\n" );
   fflush ( logFile );
#endif
}


CGIparams *openCGIparams ( char **envp )

{
   CGIparams	*params = NULL;
   char		*rawParams = NULL;
   const char	*environValues [ENVIRON_COUNT];
   const char	*p;

#if	defined(DEBUG)
   fprintf (  logFile, "openCGIparams ( %p ) {\n", envp );
   fflush ( logFile );
#endif

   if ( readEnv ( envp, environValues ) ) {

	p = environValues [QUERY_STRING];

	/* Get the query string, if there is one. */
	if ( ! p ) {

		const char	*contentLength = environValues [CONTENT_LENGTH];

		if ( contentLength ) {

			int length = strtol ( contentLength, NULL, 0 );

			rawParams = readStdin ( length );
		}

	} else rawParams = strdup ( p );

	if ( params = createCGIparams ( rawParams ) ) {

		params->servSoftware = environValues [SERVER_SOFTWARE];
		params->servName = environValues [SERVER_NAME];
		params->cgiRev = environValues [GATEWAY_INTERFACE];

		params->servProtocol = environValues [SERVER_PROTOCOL];
		params->servPort = environValues [SERVER_PORT];
		params->reqMethod = environValues [REQUEST_METHOD];
		params->pathInfo = environValues [PATH_INFO];
		params->pathXlate = environValues [PATH_TRANSLATED];
		params->scriptName = environValues [SCRIPT_NAME];
		params->remoteHost = environValues [REMOTE_HOST];
		params->remoteAddr = environValues [REMOTE_ADDR];
		params->authType = environValues [AUTH_TYPE];
		params->remoteUser = environValues [REMOTE_USER];
		params->remoteID = environValues [REMOTE_IDENT];
		params->contentType = environValues [CONTENT_TYPE];

		params->httpAccept = environValues [HTTP_ACCEPT];
		params->httpAcceptLanguage
				= environValues [HTTP_ACCEPT_LANGUAGE];
		params->httpCookie = environValues [HTTP_COOKIE];
		params->browser = environValues [HTTP_USER_AGENT];
	}
   }

#if	defined(DEBUG)
   fprintf (  logFile, "} = %p /* openCGIparams () */\n", params );
   fflush ( logFile );
#endif

   return ( params );
}


char *getCGIparam ( CGIparams *params, char *name )

{
   int	top = params->paramCount;
   int  bottom = 0;
   int	i = ( top + bottom ) >> 1;
   int	found = -1;
   char	*value = NULL;

#if	defined(DEBUG)
   fprintf (  logFile, "getCGIparam ( %p, \"%s\" ) {\n", params, name );
   fflush ( logFile );
#endif

   while ( top > bottom ) {

	int	diff = strcmp ( name, params->names [ params->sorted [i]] );

	if ( diff > 0 ) {

		bottom = i + 1;

	} else if ( diff < 0 ) {

		top = i;

	} else {

		found = i;
		break;
	}

	i = ( top + bottom ) >> 1;
   }

   if ( found > -1 ) value = params->values [params->sorted [found]];

#if	defined(DEBUG)
   fprintf (  logFile, "} = \"%s\" /* getCGIparam () */\n", value );
   fflush ( logFile );
#endif

   return ( value );
}
