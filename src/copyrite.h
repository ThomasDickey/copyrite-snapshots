/* $Id: copyrite.h,v 5.6 2010/07/04 15:43:14 tom Exp $ */

#ifndef	_copyrite_h
#define	_copyrite_h

#define	CHR_PTYPES
#define	STR_PTYPES
#include	<port2vms.h>
#include	<time.h>

#ifdef	vms
#define	PATH_END ']'
#define	DIFF_TOOL "diff/slp/out="
#else
#define PATH_END '/'
#define	DIFF_TOOL "diff"
#endif

#define	TELL	if (verbose >= 0) PRINTF
#define	VERBOSE	if (verbose >= 1) PRINTF

typedef struct {
    const char *name;		/* language-name */
    const char *from;		/* begin-comment */
    const char *to;		/* end-comment */
    const char *after;		/* put notice after line beginning */
    int box;			/* make comment-box with this */
    int line;			/* put notice after line-number */
    int column;			/* begin notice-box in column-number */
    char *format;		/* sprintf-code to insert-year */
} LANG;

extern int verbose;

	/* **(cleanup.c)** */
extern void cleanup(char *name);

	/* **(format.c)** */
extern void FormatNotice(LANG * lp_,
			 char *Owner,
			 char *Disclaim,
			 int a_opt,
			 int c_opt,
			 int w_opt);

	/* **(hasident.c)** */
extern char *has_ident(const char *name, char *first, char *last);

	/* **(insert_at.c)** */
extern char *insert_at(LANG * lp_, char *buffer);

	/* **(maskit.c)** */
extern int maskit(LANG * lp_, char *buffer);
extern void unmask(LANG * lp_, char *buffer);
extern unsigned uncomment(LANG * lp_, char *buffer, const char *name);

	/* **(parse.c)** */
extern char *exact(char *cmp, const char *ref);
#ifdef	vms
extern void strip_ver(char *name);
extern char *same_name(char *cmp, char *ref);
#else
#define	strip_ver(name)
#define	same_name(cmp,ref) exact(cmp,ref)
#endif

extern char *is_inline(char *cmp, int ref);
extern const char *leaf_of(const char *src);
extern char *skip_text(char *src);
extern char *skip_white(char *src);
extern char *skip_line(char *src);
extern int in_comment(LANG * lp_, char *src);
extern char *skip_cline(LANG * lp_, char *src);

	/* **(readit.c)** */
extern char *readit(const char *name,
		    Stat_t * sb);

	/* **(removeit.c)** */
extern char *removeit(LANG * lp_, char *buffer, char *first, char *last);

	/* **(supercede.c)** */
extern int supercede(LANG * lp_,
		     char *buffer,
		     char *owner,
		     char *disclaimer,
		     char *year,
		     int force,
		     int remove_old,
		     unsigned *changed);

#endif /* _copyrite_h */
