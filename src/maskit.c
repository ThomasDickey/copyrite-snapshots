/*
 * Title:	maskit.c
 * Author:	T.E.Dickey
 * Created:	06 Jan 1992
 * Modified:
 *		19 Jun 2004, remove K&R code, indent'd.
 *		24 Dec 1996, C++ masking.
 *		01 Dec 1993, ifdefs.
 *		22 Sep 1993, gcc warnings.
 *
 * Function:	masks all non-comment characters to non-ascii (except for
 *		whitespace contiguous in the same line).  Doing this makes
 *		it simpler to compare comment blocks containing copyrite notice
 *		as well as to remove comments.
 *
 * Bugs:	except for "c", this does not know about inline comments.
 */

#include "copyrite.h"

MODULE_ID("$Id: maskit.c,v 5.6 2004/06/19 11:19:10 tom Exp $")

#define	nonascii(c)	((c) | 0200)

/*
 * Process a "c" escape
 */
static char *
escape_c(char *buffer)
{
    register int number;
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
    return buffer;		/* the last character in the escape */
}

/*
 * Mask comments in "c" or "c++" language source.
 */
static void
mask_c(char *buffer, int plus)
{
    int quote = 0;
    int comment = FALSE;
    int pNote = FALSE;
    int InLine = FALSE;		/* true if not inline-comment */
    char *base = buffer;	/* beginning of current line */

    while (*buffer) {
	if (quote) {
	    if (*buffer == '\\')
		buffer = escape_c(buffer);
	    else if (*buffer == quote)
		quote = 0;
	} else if (pNote) {
	    if (*buffer == '\n') {
		pNote = FALSE;
		InLine = FALSE;
	    }
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
	    if (*buffer == '\\') {
		buffer = escape_c(buffer);
	    } else if (*buffer == '"' || *buffer == '\'') {
		quote = *buffer;
	    } else if (buffer[0] == '/') {
		if (plus
		    && buffer[1] == '/') {
		    pNote = TRUE;
		} else if (buffer[1] == '*') {
		    comment = TRUE;
		}
	    }
	    if (comment) {
		register char *t;
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

/*
 * Mask Fortran-comments to make the leading 'C' a punctuation-mark.  That
 * way, we don't lose the original character, but still make comments in a
 * block a punctuation-box.
 */
static void
mask_ftn(char *buffer)
{
    while (*buffer) {
	char *s = skip_line(buffer), cC = *buffer;
	if (cC == 'c')
	    *buffer = '!';
	else if (cC == 'C')
	    *buffer = '@';
	else if (cC != '*') {
	    register char *t;
	    for (t = buffer; t < s; t++)
		*t = nonascii(*t);
	}
	buffer = s;
    }
}

/*
 * Mask comments in line-oriented languages
 */
static void
mask_lines(LANG * lp_, char *buffer)
{
    while (*buffer) {
	char *s = skip_line(buffer), *t = skip_white(buffer);
	if (!exact(t, lp_->from)) {
	    for (t = buffer; t < s; t++)
		*t = nonascii(*t);
	}
	buffer = s;
    }
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/
int
maskit(LANG * lp_, char *buffer)
{
    char *at;

    /*
     * If we have no comment-delimiters, or if we cannot find the point
     * at which to insert notice, don't modify anything.  Note that the
     * latter is a fatal error.
     */
    if (!(at = insert_at(lp_, buffer)))
	return FALSE;

    while (buffer < at) {
	*buffer = nonascii(*buffer);
	buffer++;
    }

    if (!lp_->to)
	return TRUE;

    if (!strcmp(lp_->name, "c")
	|| !strcmp(lp_->name, "lex"))
	mask_c(buffer, FALSE);
    else if (!strcmp(lp_->name, "c++"))
	mask_c(buffer, TRUE);
    else if (!strcmp(lp_->name, "ftn"))
	mask_ftn(buffer);
    else
	mask_lines(lp_, buffer);
    /* patch: what about pas? */

    return TRUE;
}

/*
 * Restore a buffer to ascii state (reverses 'maskit()').
 */
void
unmask(LANG * lp_, char *buffer)
{
    register char *s;

    if (!strcmp(lp_->name, "ftn")) {
	for (s = buffer; *s; s = skip_line(s)) {
	    if (*s == '!')
		*s = 'c';
	    else if (*s == '@')
		*s = 'C';
	}
    }

    s = buffer;
    while (*s) {
	*s = toascii(*s);
	s++;
    }
}

/*
 * Remove all comments from a buffer, except for those specifying an identifier
 */
int
uncomment(LANG * lp_, char *buffer, char *name)
{
    int changes = 0;
    register char *s, *d;

    if (!lp_->to)
	return changes;

    s = buffer;
    d = buffer;
    while (*s) {
	if (in_comment(lp_, s)) {
	    char *base = s;
	    changes++;
	    if (lp_->to[0] == '\n') {	/* remove lines */
		register char *t;
		do {
		    t = skip_cline(lp_, s);
		    if (has_ident(name, s, t)) {
			while (s < t) {
			    *d++ = *s++;
			}
		    }
		} while (in_comment(lp_, s = t));
	    } else {		/* remove block */
		while (in_comment(lp_, s))
		    s++;
		if (has_ident(name, base, s)) {
		    while (base < s)
			*d++ = *base++;
		}
	    }
	} else
	    *d++ = *s++;
    }
    *d = EOS;
    return changes;
}
