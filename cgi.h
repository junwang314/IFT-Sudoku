#ident "$Id: cgi.h,v 1.4 2011/04/16 07:02:51 pwh Exp $"
/*
 * Sudoku Puzzle Solver, CGI interface logic.
 */

typedef struct {

	const char	*servSoftware;		/* SERVER_SOFTWARE */
	const char	*servName;		/* SERVER_NAME */
	const char	*cgiRev;		/* GATEWAY_INTERFACE */

	const char	*servProtocol;		/* SERVER_PROTOCOL */
	const char	*servPort;		/* SERVER_PORT */
	const char	*reqMethod;		/* REQUEST_METHOD */
	const char	*pathInfo;		/* PATH_INFO */
	const char	*pathXlate;		/* PATH_TRANSLATED */
	const char	*scriptName;		/* SCRIPT_NAME */
	const char	*remoteHost;		/* REMOTE_HOST */
	const char	*remoteAddr;		/* REMOTE_ADDR */
	const char	*authType;		/* AUTH_TYPE */
	const char	*remoteUser;		/* REMOTE_USER */
	const char	*remoteID;		/* REMOTE_IDENT */
	const char	*contentType;		/* CONTENT_TYPE */

	const char	*httpAccept;		/* HTTP_ACCEPT */
	const char	*httpAcceptLanguage;	/* HTTP_ACCEPT_LANGUAGE */
	const char	*httpCookie;		/* HTTP_COOKIE */
	const char	*browser;		/* HTTP_USER_AGENT */

	int		paramCount;
	char		*rawParams;
	char		**names;
	char		**values;
	int		*sorted;

} CGIparams;


char		*getCGIparam ( CGIparams *, char * );
CGIparams	*openCGIparams ( char ** );
void		closeCGIparams ( CGIparams * );
