/* $Id: copyrite.h,v 5.1 1993/09/22 17:07:03 dickey Exp $ */

#ifndef	_copyrite_h
#define	_copyrite_h

#define	CHR_PTYPES
#define	STR_PTYPES
#include	<portunix.h>
#include	<time.h>

#ifdef	vms
#define	PATH_END ']'
#define	DIFF_TOOL "diff/slp/out="
#else
#define PATH_END '/'
#define	DIFF_TOOL "diff"
#endif

#define	WARN	FPRINTF(stderr,
#define	TELL	if (verbose >= 0) PRINTF
#define	VERBOSE	if (verbose >= 1) PRINTF

typedef	struct	{
	char	*name;		/* language-name */
	char	*from,		/* begin-comment */
		*to,		/* end-comment */
		*after;		/* put notice after line beginning */
	int	box,		/* make comment-box with this */
		line,		/* put notice after line-number */
		column;		/* begin notice-box in column-number */
	char	*format;	/* sprintf-code to insert-year */
	} LANG;

extern	int	verbose;

	/* **(cleanup.c)** */
extern	void	cleanup  (_ar1(char *,	name));

	/* **(format.c)** */
extern	void FormatNotice(_arx(LANG *,	lp_)
			  _arx(char *,	Owner)
			  _arx(char *,	Disclaim)
			  _arx(int,	c_opt)
			  _ar1(int,	w_opt));

	/* **(has_ident.c)** */
extern	char *	has_ident(_arx(char *,	name)
			  _arx(char *,	first)
			  _ar1(char *,	last));

	/* **(insert_at.c)** */
extern	char *	insert_at(_arx(LANG *,	lp_)
			  _ar1(char *,	buffer));

	/* **(maskit.c)** */
extern	int	maskit   (_arx(LANG *,	lp_)
			  _ar1(char *,	buffer));
extern	void	unmask   (_arx(LANG *,	lp_)
			  _ar1(char *,	buffer));
extern	int	uncomment(_arx(LANG *,	lp_)
			  _arx(char *,	buffer)
			  _ar1(char *,	name));

	/* **(parse.c)** */
extern	char *	exact	 (_arx(char *,	cmp)
			  _ar1(char *,	ref));
#ifdef	vms
extern	void	strip_ver(_ar1(char *,	name));
extern	char *	same_name(_arx(char *,	cmp)
			  _ar1(char *,	ref));
#else
#define	strip_ver(name)
#define	same_name(cmp,ref) exact(cmp,ref)
#endif

extern	char *	is_inline (_arx(char *,	cmp)
			   _ar1(int,	ref));
extern	char * 	leaf_of   (_ar1(char *,	src));
extern	char *	skip_text (_ar1(char *,	src));
extern	char *	skip_white(_ar1(char *,	src));
extern	char *	skip_line (_ar1(char *,	src));
extern	int	in_comment(_arx(LANG *, lp_)
			   _ar1(char *,	src));
extern	char *	skip_cline(_arx(LANG *, lp_)
			   _ar1(char *,	src));

	/* **(readit.c)** */
extern	char *	readit    (_arx(char *,	name)
			   _ar1(STAT *,	sb));

	/* **(removeit.c)** */
extern	char *	removeit  (_arx(LANG *,	lp_)
			   _arx(char *,	buffer)
			   _arx(char *,	first)
			   _ar1(char *,	last));

	/* **(supercede.c)** */
extern	int	supercede (_arx(LANG *,	lp_)
			   _arx(char *,	buffer)
			   _arx(char *,	owner)
			   _arx(char *,	disclaimer)
			   _arx(char *,	year)
			   _arx(int,	force)
			   _arx(int,	remove_old)
			   _ar1(int *,	changed));

#endif	/* _copyrite_h */
