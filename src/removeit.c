/*
 * Title:	removeit.c
 * Author:	T.E.Dickey
 * Created:	07 Jan 1992
 * Modified:
 *		19 Jun 2004, remove K&R code, indent'd.
 *		23 Jun 1994, fixes for blank lines.
 *		01 Dec 1993, ifdefs.
 *		22 Sep 1993, gcc warnings.
 *
 * Function:	this is used to remove an old copyright-notice, which consists
 *		of all or part of a comment block.  The arguments 'first' and
 *		'last' are pointers into the buffer respectively in the first
 *		line of the notice (where the word 'Copyright' was found), and
 *		after the line which completed the notice (a comment line
 *		empty of non-white/non-punctuation characters).
 *
 * Returns:	a pointer to the first character before the deletion, so that
 *		autoincrement will point back to the next character after the
 *		deletion.
 */

#include "copyrite.h"

MODULE_ID("$Id: removeit.c,v 5.10 2025/01/07 01:09:07 tom Exp $")

/*
 * Find the beginning of the current line, restricted to staying within the
 * current comment-block.
 */
static char *
beginning_of(char *buffer, char *current)
{
    while (current > buffer && isascii(current[-1]) && current[-1] != '\n')
	current--;
    return current;
}

/*
 * Test to see if there is a comment-line preceding the current point, which
 * is filler (i.e., only punctuation and/or whitespace).
 */
static int
filler_before(char *buffer, char *current)
{
    char *previous;

    if (current <= buffer || !isascii(current[-1]))
	return FALSE;		/* cannot go further */

    previous = beginning_of(buffer, current - 1);
    while (previous < current) {
	register char c = *previous++;
	if (!(isspace(c) || ispunct(c)))
	    return FALSE;
    }
    return TRUE;
}

/*
 * Test to see if the current line begins a comment
 */
static int
begins_comment(LANG * lp_, char *current)
{
    while (*current == ' ' || *current == '\t')
	current++;
    return exact(current, lp_->from) != NULL;
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/
char *
removeit(LANG * lp_, char *buffer, char *first, char *last)
{
    register char *s, *d;

    VERBOSE("\n# remove old notice (%d,%d)",
	    (int) (first - buffer),
	    (int) (last - buffer));

    /*
     * Scan back to find the beginning of the block containing the notice
     */
    first = beginning_of(buffer, first);
    while (filler_before(buffer, first))
	first = beginning_of(buffer, first - 1);

    /*
     * Eat up trailing blanks if we don't have a comment-marker, and if
     * nothing follows the text that we're deleting.
     */
    if (lp_->to == NULL) {
	char *next = last;
	while (*next != EOS && isascii(*next) && isspace(*next)) {
	    if (*next == '\n')
		last = next;
	    next++;
	}
	if (*next == EOS)
	    last = next;
	/*
	 * If the block [first,last] encloses a comment-beginning but not a
	 * comment-ending, we may have to reinsert a comment-beginning,
	 * depending upon the language.
	 */
    } else if (lp_->to[0] != '\n') {	/* may need it */
	int last_is_in = in_comment(lp_, last);
	int first_begins = begins_comment(lp_, first);
	const char *supply = NULL;

	if (!last_is_in
	    && !first_begins)
	    supply = lp_->to;
	else if (last_is_in
		 && first_begins
		 && !begins_comment(lp_, last)) {
	    char *next = last;
	    /* If the comment that we're currently in has no
	     * non-punctuation content, try to eat it also, rather
	     * than supplying a comment-leader.
	     */
	    while (isspace(*next) || ispunct(*next)) {
		if (in_comment(lp_, next)) {
		    next++;
		} else {
		    break;
		}
	    }
	    if (in_comment(lp_, next))
		supply = lp_->from;
	    else
		last = next;
	}

	VERBOSE("\n# last:%s first:%s supply \"%s\"",
		last_is_in ? "in" : "NOT-in",
		first_begins ? "begins" : "NOT-begins",
		supply ? supply : "");
	if (supply) {		/* patch: assume length < last-first */
	    (void) strcpy(first, supply);
	    first += strlen(supply);
	}
    }

    /*
     * Finally, delete the notice.
     */
    s = last;
    d = first;
    while ((*d++ = *s++) != EOS) ;
    return first - 1;
}
