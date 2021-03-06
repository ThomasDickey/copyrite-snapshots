/*
 * Title:	parse.c
 * Author:	T.E.Dickey
 * Created:	06 Jan 1992, from 'copyrite.c'
 * Modified:
 *		19 Jun 2004, remove K&R code, indent'd.
 *		01 Dec 1993, ifdefs.
 *		22 Sep 1993, gcc warnings.
 *
 * Function:	Prepends/updates copyright-notice to one or more files.
 */

#include "copyrite.h"

MODULE_ID("$Id: parse.c,v 5.6 2010/07/04 15:19:53 tom Exp $")

#define	newline(c)	(toascii(c) == '\n')

/*
 * Match an arbitrary (exact-case) keyword
 */
char *
exact(char *cmp, const char *ref)
{
    while (*ref)
	if (*cmp == 0 || (*ref++ != *cmp++))
	    return 0;
    return cmp;
}

#ifdef	vms
void
strip_ver(char *name)
{
    register char *s = strrchr(name, ';');
    if (s != 0)
	*s = EOS;
}

char *
same_name(char *cmp, char *ref)
{
    char temp[MAXPATHLEN];
    char *s, *d, cc;
    int ok = FALSE;

    strip_ver(strcpy(temp, ref));
    for (s = temp, d = cmp; *s && *d; s++, d++) ;	/* verify lengths */

    if (cc = *d) {		/* non-null iff normal ident */
	*d = EOS;
	ok = !strucmp(temp, cmp);
	*d = cc;
    }
    return ok ? d : 0;
}
#endif /* vms */

/*
 * Find the given character within the current line, or return null
 */
char *
is_inline(char *cmp, int ref)
{
    while (*cmp && !newline(*cmp))
	if (*cmp++ == ref)
	    return cmp;
    return 0;
}

/*
 * Find the path-leaf from syntax only
 */
const char *
leaf_of(const char *path)
{
    const char *s = strrchr(path, PATH_END);
    if (s)
	path = s + 1;
    return path;
}

/*
 * Skip text/whitespace/line
 */
char *
skip_text(char *src)
{
    while (*src && isascii(*src) && !isspace(*src))
	src++;
    return src;
}

char *
skip_white(char *src)
{
    while (*src && isascii(*src) && isspace(*src))
	src++;
    return src;
}

char *
skip_line(char *src)
{
    while ((*src != EOS) && !newline(*src++)) ;
    return src;
}

/*
 * Returns true iff the current pointer is in a comment
 */
int
in_comment(LANG * lp_, char *src)
{
    if (*src != EOS && isascii(*src)) {
	/*
	 * Check for special case such as a newline following the
	 * end of a C-language comment.
	 */
	const char *to = lp_->to;
	if (to != 0 && *to != '\n' && *src == '\n')
	    return in_comment(lp_, src + 1);
	return TRUE;
    }
    return FALSE;
}

/*
 * Skip to the end of the current line, limited to the current comment-block
 * (assumes that comments are highlighted with 'maskit()').
 */
char *
skip_cline(LANG * lp_, char *src)
{
    while (in_comment(lp_, src) && !newline(*src++)) ;
    return src;
}
