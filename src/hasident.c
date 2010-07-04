/*
 * Title:	identifier.c
 * Author:	T.E.Dickey
 * Created:	06 Jan 1992
 * Modified:
 *		19 Jun 2004, remove K&R code, indent'd.
 *		01 Dec 1993, ifdefs, TurboC warnings.
 *		22 Sep 1993, gcc warnings.
 *		16 Oct 1992, filename may have embedded blanks.
 *		16 Jul 1992, allow for SCCS identifier w/o module name.
 *
 * Function:	Scans a buffer for an RCS or SCCS identifier
 */

#include "copyrite.h"

MODULE_ID("$Id: hasident.c,v 5.6 2010/07/04 15:19:35 tom Exp $")

char *
has_ident(const char *name,
	  char *first,
	  char *last)
{
    char *base;
    char *s, *t, *d, c;

    name = leaf_of(name);

    s = first;
    while ((t = base = strchr(s, '$')) != 0 && (t < last)) {
	t++;
	if (((s = exact(t, "Id:")) != 0
	     || (s = exact(t, "Header:")) != 0)
	    && is_inline(t, '$')) {
	    /* RCS identifier can have pathname prepended */
	    s = skip_white(s);
	    d = skip_text(s);
	    c = *d;
	    *d = EOS;
	    while (is_inline(s, '/'))
		s++;
	    *d = c;
	    if ((s = same_name(s, name)) != 0
		&& (s = exact(s, ",v")) != 0
		&& isspace(*s))
		return base;
	}
	s = t;
    }

    s = first;
    while ((t = base = strchr(s, '@')) != 0 && (t < last)) {
	t++;
	if ((s = exact(t, "(#)")) != NULL) {
	    t = s;
	    /* some versions of SCCS don't do module-name */
	    if ((s = same_name(t, name)) != NULL)
		return base;

	    t = skip_text(t);	/* module-name, if any */
	    t = skip_white(t);
	    if ((s = same_name(t, name)) != NULL)
		return base;
	}
	s = t;
    }
    return 0;
}
