#ifndef	lint
static	char	*Id = "$Id: test.c,v 5.0 1992/01/06 11:37:12 ste_cm Rel $";
#endif

/*
 * Title:	maskit.c
 * Author:	T.E.Dickey
 * Created:	06 Jan 1992
 *
 * Function:	masks all non-comment characters to non-ascii (except for
 *		whitespace contiguous in the same line).  Doing this makes
 *		it simpler to compare comment blocks containing copyrite notice
 *		as well as to remove comments.
 *
 * Bugs:	except for "c", this does not know about inline comments.
 */

#include "copyrite.h"

#define	nonascii(c)	((c) | 0200)

/*
 * Mask comments in "c" language source.
 */
static
mask_c(
_AR1(char *,	buffer)
	)
_DCL(char *,	buffer)
{
	int	quote	= 0;
	int	comment	= FALSE;
	int	InLine	= FALSE;	/* true if not inline-comment */
	char	*base	= buffer;	/* beginning of current line */

	while (*buffer) {
		if (quote) {
			if (*buffer == '\\') {
				register int	number;
				*buffer = nonascii(*buffer);
				buffer++;
				number = 0;
				while (isdigit(*buffer)) {
					*buffer = nonascii(*buffer);
					buffer++;
					number++;
				}
				if (number)
					buffer--;
			} else if (*buffer == quote)
				quote = 0;
		} else if (comment) {
			if (*buffer == '*' && buffer[1] == '/') {
				comment = FALSE;
				buffer += 2;
				while (isspace(*buffer))
					if (*buffer++ == '\n') {
						base = buffer;
						buffer -= InLine;
						break;
					}
				continue;
			} else if (*buffer == '\n')
				InLine = FALSE;
		} else {
			if (*buffer == '"' || *buffer == '\'') {
				quote = *buffer;
			} else if (*buffer == '/' && buffer[1] == '*') {
				register char *t;
				comment = TRUE;
				for (t = buffer - 1; t >= base; t--)
					if (isspace(toascii(*t)))
						*t = toascii(*t);
					else
						break;
				InLine = (t != base);
			} 
		} 
		if (!comment)
			*buffer = nonascii(*buffer);
		if (toascii(*buffer++) == '\n')
			base = buffer;
	}
}
