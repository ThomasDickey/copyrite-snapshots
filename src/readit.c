/*
 * Title:	readit.c
 * Author:	T.E.Dickey
 * Created:	06 Jan 1992, from 'copyrite.c'
 * Modified:
 *		01 Dec 1993, ifdefs.
 *		22 Sep 1993, gcc warnings.
 *
 * Function:	Reads a file into memory
 */

#include "copyrite.h"

MODULE_ID("$Id: readit.c,v 5.5 1995/05/13 23:28:08 tom Exp $")

/*
 * Test for binary-file
 */
static
int	isbinary(
	_ARX(char *,	buffer)
	_AR1(unsigned,	length)
		)
	_DCL(char *,	buffer)
	_DCL(unsigned,	length)
{
	register int	j, c;
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
readit(
	_ARX(char *,	in_name)
	_AR1(Stat_t *,	sb)
		)
	_DCL(char *,	in_name)
	_DCL(Stat_t *,	sb)
{
	static	unsigned f_max;
	static	char	*f_bfr;

	auto	int	f_got;
	auto	unsigned f_use;
	auto	FILE	*ifp;

	VERBOSE("\n# size: %ld bytes", (long) (sb->st_size));

	/*
	 * load file into memory
	 */
	if (sb->st_size <= 0) {
		TELL("(empty)\n");
		return 0;
	}
	if (f_max <= sb->st_size) {
		f_max = (f_max * 9)/8;
		if (f_max <= sb->st_size)
			f_max = (sb->st_size * 9)/8 + BUFSIZ;
		f_bfr = doalloc(f_bfr, f_max);
		VERBOSE("\n# load %s in %d bytes", in_name, f_max);
	}
	if ((ifp = fopen(in_name, "r")) != NULL) {
		/* patch: later, try test-read of first block for binary-text */
		f_got = fread(f_bfr, sizeof(char), (size_t)(sb->st_size), ifp);
		FCLOSE(ifp);
		if (f_got < 0) {
			TELL("(no data)\n");
			return 0;
		}
		f_bfr[f_use = f_got] = EOS;
		VERBOSE("\n# used %d bytes for %s", f_use, in_name);
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
