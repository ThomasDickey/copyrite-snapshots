/*
 * Title:	readit.c
 * Author:	T.E.Dickey
 * Created:	06 Jan 1992, from 'copyrite.c'
 * Modified:
 *		19 Jun 2004, remove K&R code, indent'd.
 *		01 Dec 1993, ifdefs.
 *		22 Sep 1993, gcc warnings.
 *
 * Function:	Reads a file into memory
 */

#include "copyrite.h"

MODULE_ID("$Id: readit.c,v 5.8 2010/07/04 15:40:10 tom Exp $")

/*
 * Test for binary-file
 */
static int
isbinary(char *buffer, size_t length)
{
    size_t j;
    int c;

    for (j = 0; j < length; j++) {
	c = buffer[j];
	if (!isascii(c))
	    return TRUE;
	if (!isprint(c) && !isspace(c))
	    return TRUE;
    }
    return FALSE;
}

/*
 * Load a single file to memory
 */
char *
readit(const char *in_name, Stat_t * sb)
{
    static size_t f_max;
    static char *f_bfr;

    size_t f_got;
    size_t f_use;
    FILE *ifp;

    VERBOSE("\n# size: %ld bytes", (long) (sb->st_size));

    /*
     * load file into memory
     */
    if (sb->st_size <= 0) {
	TELL("(empty)\n");
	return 0;
    }
    if (f_max <= (size_t) sb->st_size) {
	f_max = (f_max * 9) / 8;
	if (f_max <= (size_t) sb->st_size)
	    f_max = (size_t) (sb->st_size * 9) / 8 + BUFSIZ;
	f_bfr = doalloc(f_bfr, f_max);
	VERBOSE("\n# load %s in %lu bytes", in_name, (unsigned long) f_max);
    }
    if ((ifp = fopen(in_name, "r")) != NULL) {
	/* patch: later, try test-read of first block for binary-text */
	f_got = fread(f_bfr, sizeof(char), (size_t) (sb->st_size), ifp);

	if ((f_got == 0) && ferror(ifp)) {
	    TELL("(no data)\n");
	    FCLOSE(ifp);
	    return 0;
	}
	FCLOSE(ifp);

	f_bfr[f_use = f_got] = EOS;
	VERBOSE("\n# used %lu bytes for %s", (unsigned long) f_use, in_name);
    } else {
	perror(in_name);
	return 0;
    }

    /*
     * if binary, skip this
     */
    if (isbinary(f_bfr, f_use)) {
	TELL("(binary)\n");
	return 0;
    }
    return f_bfr;
}
