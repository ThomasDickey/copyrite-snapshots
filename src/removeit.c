#ifndef	NO_IDENT
static	char	*Id = "$Id: removeit.c,v 5.2 1993/12/01 20:24:58 tom Exp $";
#endif

/*
 * Title:	removeit.c
 * Author:	T.E.Dickey
 * Created:	07 Jan 1992
 * Modified:
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

/*
 * Find the beginning of the current line, restricted to staying within the
 * current comment-block.
 */
static
char *
beginning_of(
_ARX(char *,	buffer)
_AR1(char *,	current)
	)
_DCL(char *,	buffer)
_DCL(char *,	current)
{
	while (current > buffer && isascii(current[-1]) && current[-1] != '\n')
		current--;
	return current;
}

/*
 * Test to see if there is a comment-line preceding the current point, which
 * is filler (i.e., only punctuation and/or whitespace).
 */
static
int
filler_before(
_ARX(char *,	buffer)
_AR1(char *,	current)
	)
_DCL(char *,	buffer)
_DCL(char *,	current)
{
	char	*previous;

	if (current <= buffer || !isascii(current[-1]))
		return FALSE;	/* cannot go further */

	previous = beginning_of(buffer, current-1);
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
static
int
begins_comment(
_ARX(LANG *,	lp_)
_AR1(char *,	current)
	)
_DCL(LANG *,	lp_)
_DCL(char *,	current)
{
	while (*current == ' ' || *current == '\t')
		current++;
	return exact(current, lp_->from) != 0;
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/
char *
removeit(
_ARX(LANG *,	lp_)
_ARX(char *,	buffer)
_ARX(char *,	first)
_AR1(char *,	last)
	)
_DCL(LANG *,	lp_)
_DCL(char *,	buffer)
_DCL(char *,	first)
_DCL(char *,	last)
{
	register char	*s, *d;

	VERBOSE("\n# remove old notice (%d,%d)", first-buffer, last-buffer);

	/*
	 * Scan back to find the beginning of the block containing the notice
	 */
	first = beginning_of(buffer, first);
	while (filler_before(buffer, first))
		first = beginning_of(buffer, first-1);

	/*
	 * If the block [first,last] encloses a comment-beginning but not a
	 * comment-ending, we may have to reinsert a comment-beginning,
	 * depending upon the language.
	 */
	if (lp_->to != 0 && lp_->to[0] != '\n') {	/* may need it */
		int	last_is_in   = in_comment(lp_, last),
			first_begins = begins_comment(lp_, first);
		char	*supply = 0;

		if (!last_is_in
		 && !first_begins)
			supply = lp_->to;
		else if (last_is_in
		 && first_begins
		 && !begins_comment(lp_,last))
			supply = lp_->from;

		VERBOSE("\n# last:%s first:%s supply \"%s\"",
			last_is_in   ? "in"     : "NOT-in",
			first_begins ? "begins" : "NOT-begins",
			supply       ? supply   : "");
		if (supply) {	/* patch: assume length < last-first */
			(void)strcpy(first, supply);
			first += strlen(supply);
		}
	}

	/*
	 * Finally, delete the notice.
	 */
	s = last;
	d = first;
	while ((*d++ = *s++) != EOS)
		;
	return first - 1;
}
