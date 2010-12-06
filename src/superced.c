/*
 * Title:	supercede.c
 * Author:	T.E.Dickey
 * Created:	06 Jan 1992
 * Modified:
 *		19 Jun 2004, remove K&R code, indent'd.
 *		17 Jun 1994, tweaked owner-decoding to accept a leading "by ".
 *		01 Dec 1993, ifdefs.
 *		22 Sep 1993, gcc warnings.
 *		16 Jul 1992, mods to handle run-on prior notices.
 *
 * Function:	Tests to find any prior copyright-notices in the current buffer
 *		that can be overridden:
 *
 *		(a) same owner, different year and/or disclaimer
 *		(b) any owner, with 'force' set to true.
 *
 *		In the first case, the old notice is removed.  If a new notice
 *		can be applied, this procedure returns true.
 *
 * Limitations:	This assumes that the owner+disclaimer are in a comment block
 *		(set of contiguous comment lines) with no other text except for
 *		punctuation (e.g., to make a box).
 */

#include "copyrite.h"

MODULE_ID("$Id: superced.c,v 5.10 2010/12/05 20:27:01 tom Exp $")

/*
 * Copy/filter a comment-line to the output.  Trim all leading/trailing blanks
 * and box-punctuation.
 */
static char *
copy_line(LANG * lp_,
	  char *dst,
	  char *src,
	  int continuation)
{
    char *base = dst;

    if (in_comment(lp_, src)) {
	char *next = skip_cline(lp_, src);
	int boxed = 0;

	while (isascii(*src) && isspace(*src))
	    src++;
	while (isascii(*src) && ispunct(*src))
	    src++, boxed++;
	while (isascii(*src) && isspace(*src))
	    src++;

	while (in_comment(lp_, src) && src < next) {
	    if ((dst == base) && continuation)
		*dst++ = ' ';
	    *dst++ = *src++;
	}

	while (dst > base && isspace(dst[-1]))
	    dst--;
	if (boxed) {
	    while (dst > base && ispunct(dst[-1]))
		dst--;
	    while (dst > base && isspace(dst[-1]))
		dst--;
	}
    }
    *dst = EOS;
    return base;
}

/*
 * Copy the (presumably) initial line of a long buffer, allowing one blank
 * line before we get really started.
 */
static char *
copy_initial(LANG * lp_, char *dst, char *src)
{
    if (!*copy_line(lp_, dst, src, FALSE)) {
	src = skip_cline(lp_, src);
	(void) copy_line(lp_, dst, src, FALSE);
    }
    return src;
}

/*
 * Convert character to uppercase if it is alphabetic
 */
static int
Upper(int c)
{
    if (isalpha(c)) {
	if (islower(c))
	    c = UpperMacro(c);
    } else if (isspace(c))
	c = ' ';
    return c;
}

/*
 * Compare, looking for word(s), ignoring punctuation, whitespace-type and case
 */
static char *
any_case(char *cmp, const char *ref)
{
    while (*ref) {
	int a = Upper(*ref++), b = Upper(*cmp++);
	if (a != b)
	    return 0;
    }
    return cmp;
}

/*
 * Look for any instance of all-rights-reserved
 */
static char *
AllRites(char *buffer)
{
    char *t;
    if ((t = any_case(buffer, "all rights reserved")) == NULL)
	t = any_case(buffer, "all rights reserved.");
    return t;
}

/*
 * Look for any instance of copyright keyword
 */
static char *
CopyRite(char *buffer)
{
    char *t;
    if ((t = any_case(buffer, "copyright")) == NULL)
	t = any_case(buffer, "copr.");
    return t;
}

/*
 * Look for something like a list of years.  Allow "(c)" in the list.
 */
static char *
any_year(char *buffer, char *result)
{
    char *base = result;
    char *s, *t;
    int got_year = FALSE;

    s = skip_white(buffer);
    while (*s) {
	while (isascii(*s) && isspace(*s))
	    *result++ = *s++;
	if ((t = any_case(s, "(c)")) != NULL) {
	    while (s < t)
		*result++ = *s++;
	} else if (isdigit(*s)) {
	    got_year = TRUE;
	    while (isdigit(*s))
		*result++ = *s++;
	} else if (*s == ',') {
	    *result++ = *s++;
	} else {
	    s = skip_white(s);
	    break;
	}
    }
    while (result > base && isspace(result[-1]))
	result--;
    *result = EOS;
    if (!got_year)		/* ignore isolated "(c)" */
	*base = EOS;
    return *base != EOS ? s : 0;
}

/*
 * Look for portions of the notice that we can ignore
 */
static char *
any_mark(char *buffer)
{
    if (any_case(buffer, "by "))
	return buffer + 2;
    else if (*buffer == '.' || *buffer == ',')
	return buffer + 1;
    return buffer;
}

/*
 * Look for a portion of the notice that we can treat as the owner.  This is
 * a string followed by a line in which no alphameric character is found.
 * It can end earlier if the "all rights reserved" string is found.
 *
 * Note: this returns a pointer to the next line after that containing the
 *	owner-string.  If the year follows the owner, e.g.,
 *		Copr. by FooBar, 1999
 *	this will not give only the owner-string.  To fix, we split-off trailing
 *	numbers as a year in 'find_notice()' iff we did not find a year.
 *
 * Note also: the file may have multiple prior notices, some w/o disclaimer.
 *	To handle this, test for the magic word "copyright".
 */
static char *
any_owner(LANG * lp_,
	  char *buffer,
	  char *result)
{
    char *t;
    char *base = result;
    int lines = 0, force = FALSE;

    while (*copy_line(lp_, result, buffer, lines++)) {

	for (t = base; *t; t++) {
	    if (AllRites(t)
		|| CopyRite(t)) {
		force = TRUE;
		buffer = skip_cline(lp_, buffer);
		*t = EOS;
		while (t > base && isspace(t[-1]))
		    *--t = EOS;
		break;
	    }
	}
	if (force)
	    break;

	result += strlen(result);
	buffer = skip_cline(lp_, buffer);
    }

    return *base ? buffer : 0;
}

/*
 * Finds a copyright-notice, which consists of the following parts, in any
 * order (except that "copyright" must come first):
 *
 *	"COPYRIGHT"			-- first
 *	{year} ["by"|"."|","] {owner}	-- any order
 *	"ALL RIGHTS RESERVED." 		-- last (optional)
 *
 * If we find the notice, copy the contents of the year, owner and disclaimer
 * fields into the call-arguments and return a pointer past the end of the
 * notice.
 */
static char *
find_notice(LANG * lp_,
	    char *buffer,
	    char *got_year,
	    char *got_owner,
	    char *got_discl)
{
    char *s, *t;
    int fields;

    *got_year =
	*got_owner =
	*got_discl = EOS;

    if ((buffer = CopyRite(buffer)) != NULL) {
	for (fields = 0; fields < 3; fields++) {
	    s = any_mark(skip_white(buffer));
	    if (!*got_year)
		if ((t = any_year(s, got_year)) != NULL) {
		    buffer = t;
		    continue;
		}
	    if (!*got_owner)
		if ((t = any_owner(lp_, s, got_owner)) != NULL) {
		    buffer = t;
		    continue;
		}
	}
    }

    /*
     * The year may have been obscured by the owner, by following it.
     * If the owner-string ends with a number, assume it is a year (could
     * be a zip-code!).
     */
    if (*got_owner && !*got_year) {
	char *last = got_owner + strlen(got_owner);
	char *mark;
	while (last > got_owner &&
	       (ispunct(last[-1])
		|| isspace(last[-1])))
	    last--;
	*last = EOS;
	while (last > got_owner &&
	       (isdigit(last[-1])
		|| isspace(last[-1])
		|| ispunct(last[-1])))
	    last--;
	mark = last;
	while (ispunct(*last) || isspace(*last))
	    last++;
	(void) strcpy(got_year, last);
	*mark = EOS;
    }

    /* Copy the disclaimer, if we found a notice */
    if (*got_year && *got_owner) {
	char *result = got_discl;
	int lines;

	/* allow one blank line */
	buffer = copy_initial(lp_, result, buffer);

	if ((t = AllRites(result)) != NULL) {
	    buffer = skip_cline(lp_, buffer);
	    for (t = skip_white(t), s = got_discl; (*s++ = *t++) != EOS;) ;
	    result = got_discl;
	    buffer = copy_initial(lp_, result, buffer);
	}

	if (CopyRite(result)) {
	    *result = EOS;
	} else {
	    lines = (*result != EOS);
	    buffer = skip_cline(lp_, buffer);
	    do {
		result += strlen(result);
		(void) copy_line(lp_, result, buffer, lines++);
		if (CopyRite(result))
		    *result = EOS;
		if (!*result)
		    break;
		buffer = skip_cline(lp_, buffer);
	    } while (*result);
	}
	return buffer;
    }
    return 0;
}

/*
 * Returns true if the given character is not normally considered a candidate
 * for mismatch in 'same_text()'.
 */
static int
ignore_punc(char *src)
{
    return (strchr(";:.,", *src) != 0);
}

/*
 * Returns a pointer to the last character of a string that may have an period
 * (or equivalent punction-mark).
 */
static char *
end_of(char *src)
{
    char *tmp = src + strlen(src);
    while (tmp > src && isspace(tmp[-1]))
	tmp--;
    if (tmp > src && ignore_punc(tmp - 1))
	tmp--;
    return tmp;
}

static int
not_same(char *cmp, char *ref)
{
    VERBOSE("\n<%.78s\n>%.78s", cmp, ref);
    return FALSE;
}

/*
 * Compare the two strings.  They must be the same, ignoring case, except for
 * leading whitespace and an optional trailing period.
 *
 * Note: two empty-strings will always be different!
 */
static int
same_text(char *cmp, char *ref)
{
    int count = 0;
    char *last_cmp = end_of(cmp), *last_ref = end_of(ref);

    while (cmp < last_cmp && ref < last_ref) {
	if (isspace(*cmp) && isspace(*ref)) {
	    cmp = skip_white(cmp);
	    ref = skip_white(ref);
	    count++;
	} else if (!isspace(*cmp) && !isspace(*ref)) {
	    if (Upper(*cmp++) != Upper(*ref++))
		return not_same(cmp - 1, ref - 1);
	    count++;
	} else
	    return not_same(cmp, ref);
    }

    if ((cmp < last_cmp && !isspace(*cmp) && !ignore_punc(cmp))
	|| (ref < last_ref && !isspace(*ref) && !ignore_punc(ref)))
	return not_same(cmp, ref);

    return count;		/* nonzero iff both non-empty */
}

static void
Trace(const char *name,
      int same,
      char *text)
{
    VERBOSE("\n# %-6.6s%s{%.*s}",
	    name,
	    same ? "same-" : "",
	    *text ? (verbose > 1 ? (int) strlen(text) : 60) : 1,
	    text);
}

/************************************************************************
 *	public entrypoints						*
 ************************************************************************/

/* returns true iff we can apply notice */
int
supercede(LANG * lp_,
	  char *buffer,
	  char *owner,
	  char *discl,
	  char *year,
	  int force,
	  int remove_old,
	  unsigned *changed)
{
    char *s = buffer, *t;

    static size_t b_max;
    static char *got_year, *got_owner, *got_discl;

    int ok = FALSE;
    int found = FALSE;
    int prior = FALSE;
    size_t b_use = strlen(buffer) + 1;

    /*
     * Make local buffers that are guaranteed to be big enough to copy any
     * fraction of the input buffer.
     */
    if (b_use >= b_max) {
	b_max = (b_max * 9) / 8;
	if (b_max <= b_use)
	    b_max = (b_use * 9) / 8 + BUFSIZ;
	got_year = doalloc(got_year, b_max);
	got_owner = doalloc(got_owner, b_max);
	got_discl = doalloc(got_discl, b_max);
    }

    while (*s) {
	if ((t = find_notice(lp_, s, got_year, got_owner, got_discl)) != NULL) {
	    int eq_year = same_text(year, got_year);
	    int eq_owner = same_text(owner, got_owner);
	    int eq_discl = same_text(discl, got_discl);

	    /* Do this so I can force "by" to appear for
	     * notices by people, etc.
	     */
	    if (!eq_owner)
		eq_owner = same_text(skip_white(any_mark(owner)), got_owner);
	    Trace("year", eq_year, got_year);
	    Trace("owner", eq_owner, got_owner);
	    Trace("text", eq_discl, got_discl);

	    found = TRUE;
	    if (eq_owner) {
		ok = TRUE;
		if (remove_old
		    || !(eq_discl && eq_year)) {
		    *changed += (unsigned) (t - s);
		    s = removeit(lp_, buffer, s, t);
		} else {
		    /*
		     * Notice would be equivalent (but
		     * possibly at a different location).
		     */
		    *changed += (unsigned) force;
		    return force;
		}
	    } else if (force) {
		ok = TRUE;
		if (remove_old) {
		    *changed += (unsigned) (t - s);
		    s = removeit(lp_, buffer, s, t);
		}
	    } else
		prior = TRUE;
	}
	s++;
    }

    if (!ok && prior)
	VERBOSE("\n? cannot modify ");

    /*
     * Force the file to look different if we found a notice that we could
     * overwrite, or if we found no notice (and are not trying to remove it)
     */
    if (found)
	*changed += 1;
    else
	*changed += (unsigned) !remove_old;

    return !(found && !ok);
}
