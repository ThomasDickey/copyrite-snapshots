/*
 * Title:	insert_at.c
 * Author:	T.E.Dickey
 * Created:	07 Jan 1992
 * Modified:
 *		19 Jun 2004, remove K&R code, indent'd.
 *		01 Dec 1993, ifdefs.
 *
 * Function:	finds the point in a buffer at which we will insert a notice.
 *		This is also the beginning of the region in which we can strip
 *		comments.
 *
 *		Note that some language-types (e.g., "lex") can have more than
 *		one possible insertion-point.  We chose the one which is
 *		earliest in the file.
 */

#include "copyrite.h"

MODULE_ID("$Id: insertat.c,v 5.4 2004/06/19 11:23:10 tom Exp $")

char *
insert_at(LANG * lp_,
	  char *buffer)
{
    int ok = FALSE;
    char *name = lp_->name;
    char *at = buffer + strlen(buffer);

    for (; !strcmp(name, lp_->name); lp_++) {
	size_t used = 0;

	if (lp_->line) {
	    /* look for line-number */
	    register char *s;
	    register int line;
	    for (s = buffer, line = 0; line < lp_->line; line++)
		if (!*(s = skip_line(s)))
		    break;
	    if (line < lp_->line) {
		VERBOSE("(after line %d?)\n", lp_->line);
		continue;
	    }
	    used = (s - buffer);
	}

	if (lp_->after) {
	    register char *t = buffer + used;
	    int found = TRUE;

	    /* look for line beginning with marker */
	    while (!exact(t, lp_->after)) {
		if (!*(t = skip_line(t))) {
		    found = FALSE;
		    break;
		}
	    }
	    if (!found) {
		VERBOSE("(after %s)\n", lp_->after);
		continue;
	    }
	    t = skip_line(t);
	    used = (t - buffer);
	}
	if (used <= (at - buffer)) {
	    ok = TRUE;
	    at = buffer + used;
	}
    }

    return ok ? at : 0;
}
