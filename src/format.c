/*
 * Title:	format.c
 * Author:	T.E.Dickey
 * Created:	06 Jan 1992, from 'copyrite.c'
 * Modified:
 *		01 Dec 1993, ifdefs.
 *		22 Sep 1993, gcc warnings.
 *
 * Function:	Formats a comment (box) for the given language.
 */

#include "copyrite.h"

MODULE_ID("$Id: format.c,v 5.5 1994/06/23 23:47:32 tom Exp $")

#define	BLANK ' '

/*
 * Protect ourselves against '%' in template
 */
static
char *
NoPercent(
_AR1(char *,	src))
_DCL(char *,	src)
{
	unsigned count = 0;
	char	*s;

	for (s = src; *s; s++)
		if (*s == '%')
			count++;

	if (count) {
		char *dst = doalloc((char *)0,
				    (unsigned)(strlen(src) + count + 1));
		for (s = dst; (*s = *src++) != EOS; s++)
			if (*s == '%')
				*(++s) = '%';
		src = dst;
	}
	return src;
}

/*
 * Generate a notice-format for the given language
 */
void	FormatNotice(
	_ARX(LANG *,	lp_)
	_ARX(char *,	Owner)
	_ARX(char *,	Disclaim)
	_ARX(int,	c_opt)
	_AR1(int,	w_opt)
		)
	_DCL(LANG *,	lp_)
	_DCL(char *,	Owner)
	_DCL(char *,	Disclaim)
	_DCL(int,	c_opt)
	_DCL(int,	w_opt)
{
	static	int   t_len;
	static	char *t_bfr;
	static	char *fmt = "Copyright %s%%04d %s%s%sAll Rights Reserved.\n%s%s";

	unsigned need;
	auto	int	to_newline, first, mark_it;
	auto	int	col, r_margin, state;
	auto	char	*it, *dst, *src;

	if (lp_->format != 0)
		return;

	if (!t_bfr) {
		char	*circle = c_opt ? "(c) " : "";
		int	period	= !ispunct(Owner[strlen(Owner)-1]);
		int	newline	= strlen(Owner) + strlen(fmt) + 5 > w_opt;

		t_bfr	= doalloc((char *)0,
				  (unsigned)(
					  strlen(fmt)
					+ strlen(Owner)
					+ strlen(Disclaim)
					+ 80));

		/* patch: this assumes that the format-string will have the
		 * same length as the formatted-string, e.g., "%04d" => "1991".
		 * If I changed this to creating format on-the-fly, I could
		 * have longer lists of years.
		 */
		FORMAT(t_bfr, fmt,
			circle,
			NoPercent(Owner),
			period ? "." : "",
			newline ? "\n" : "  ",
			*Disclaim ? "\n" : "",
			NoPercent(Disclaim));
		t_len = strlen(t_bfr);
	}

	/* patch: later, do dynamic allocation/append instead of this estimate*/
	need = (unsigned)(t_len + t_len + w_opt * 3);
	it = doalloc((char *)0, need);
	*it = EOS;

	/* format the text into the desired box-comment */
	to_newline = ((lp_->to != 0) && (lp_->to[0] == '\n'));
	r_margin   = lp_->to   ? (1 + strlen(lp_->to))   : 0;

	state   = 0;	/* no-comment */
	first   = TRUE;
	col     = 0;
	src     = t_bfr;
	dst     = it;
	mark_it = lp_->box;

	while (*src) {
		switch (state) {
		case 0:	/* begin-comment */
			if (lp_->from) {
				col = strlen(lp_->from);
				(void)strcpy(dst, lp_->from);
				dst += col;
			}
			if (first) {
				first = FALSE;
				if (lp_->from) {
					while (col++ <= (w_opt - r_margin + 1))
						*dst++ = mark_it;
					*dst++ = '\n';
					col = 0;
					if (to_newline) {
						col = strlen(lp_->from);
						(void)strcpy(dst, lp_->from);
						dst += col;
					}
				} else
					*dst++ = '\n';
			}
			/* fall-thru */

		case 1:	/* begin continuation */
			if (col < lp_->column) {
				while (col < lp_->column-1) {
					*dst++ = BLANK;
					col++;
				}
				*dst++ = mark_it;
				col++;
			}
			if (lp_->from) {
				*dst++ = BLANK;
				col++;
			}
			state = 2;
			/* fall-thru */

		case 2:	/* in-line */
			break;

		case 3:	/* fill the remainder of the line */
			if (mark_it) {
				while (col <= (w_opt - r_margin)) {
					*dst++ = BLANK;
					col++;
				}
				*dst++ = mark_it;
			}
			*dst++  = '\n';
			col     = 0;
			state   = to_newline ? 0 : 1;
			continue;
		}

		/* check to see if the latest word will fit */
		if ((src == t_bfr || !isspace(src[-1])))
			;
		else if ((skip_text(src)-src) + col > (w_opt - r_margin)) {
			state = 3;
			if (dst[-1] != '\n') {
				dst--;	/* back up before this blank */
				col--;
			}
			continue;
		}

		if (*src == '\n') {
			src++;
			state = 3;
			continue;
		}
		/* patch: count col special for escaped '%' */
		*dst++ = *src++;
		col++;
	}

	/* fill the last line */
	if (col != 0) {
		if (mark_it) {
			while (col <= (w_opt - r_margin)) {
				*dst++ = BLANK;
				col++;
			}
		}
		if (lp_->to) {
			if (mark_it) {
				*dst++ = mark_it;
				*dst++ = '\n';
				col = 0;
				if (lp_->from && to_newline) {
					col = strlen(strcpy(dst, lp_->from));
					dst += col;
				}
				while (col < lp_->column-1) {
					*dst++ = BLANK;
					col++;
				}
				while (col++ <= (w_opt - r_margin + to_newline))
					*dst++ = mark_it;
			}
			if (!to_newline) {
				for (src = lp_->to; *src; src++)
					*dst++ = *src;
			}
		}
		*dst++ = '\n';
	}

	if (!lp_->to)
		*dst++ = '\n';

	*dst = EOS;
	lp_->format = it;
}
