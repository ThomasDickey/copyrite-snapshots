/*
 * Title:	cleanup.c
 * Author:	T.E.Dickey
 * Created:	07 Jan 1992
 * Modified:
 *		19 Jun 2004, remove K&R code, indent'd.
 *		01 Dec 1993, ifdefs, use 'catchall()' for portability.
 *
 * Function:	acts as a cleanup-module for 'copyrite', by registering the
 *		name of the temporary-file to remove if a signal is caught.
 */

#define SIG_PTYPES
#include "copyrite.h"

MODULE_ID("$Id: cleanup.c,v 5.4 2004/06/19 11:23:31 tom Exp $")

static char last_name[MAXPATHLEN];

static
SIGNAL_FUNC(catch)
{
    (void) signal(sig, SIG_IGN);
    if (*last_name)
	(void) unlink(last_name);
    exit(sig);
}

void
cleanup(char *name)
{
    if (name != 0) {
	(void) strcpy(last_name, name);
	catchall(catch);
    } else {
	catchall(SIG_DFL);
	*last_name = EOS;
    }
}
