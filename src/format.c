/*
 * Title:	format.c
 * Author:	T.E.Dickey
 * Created:	06 Jan 1992, from 'copyrite.c'
 * Modified:
 *		20 Jul 1997, use  dyn_string rather than estimated buffer size.
 *		01 Dec 1993, ifdefs.
 *		22 Sep 1993, gcc warnings.
 *
 * Function:	Formats a comment (box) for the given language.
 */

#include "copyrite.h"

#include <dyn_str.h>

MODULE_ID("$Id: format.c,v 5.6 1997/06/20 23:59:45 tom Exp $")

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
	static	DYN  *tmp;
	static	size_t  t_len;
	static	char *t_bfr;
	static	char *fmt = "Copyright %s%%04d %s%s%sAll Rights Reserved.\n%s%s";

	auto	int	to_newline, first, mark_it;
	auto	int	col, r_margin, state;
	auto	char	*src;

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
	dyn_init(&tmp, t_len);

	/* format the text into the desired box-comment */
	to_newline = ((lp_->to != 0) && (lp_->to[0] == '\n'));
	r_margin   = lp_->to   ? (1 + strlen(lp_->to))   : 0;

	state   = 0;	/* no-comment */
	first   = TRUE;
	col     = 0;
	src     = t_bfr;
	mark_it = lp_->box;

	while (*src) {
		switch (state) {
		case 0:	/* begin-comment */
			if (lp_->from) {
				col = strlen(lp_->from);
				tmp = dyn_append(tmp, lp_->from);
			}
			if (first) {
				first = FALSE;
				if (lp_->from) {
					while (col++ <= (w_opt - r_margin + 1)) {
						tmp = dyn_append_c(tmp, mark_it);
					}
					tmp = dyn_append_c(tmp, '\n');
					col = 0;
					if (to_newline) {
						col = strlen(lp_->from);
						tmp = dyn_append(tmp, lp_->from);
					}
				} else {
					tmp = dyn_append_c(tmp, '\n');
				}
			}
			/* fall-thru */

		case 1:	/* begin continuation */
			if (col < lp_->column) {
				while (col < lp_->column-1) {
					tmp = dyn_append_c(tmp, BLANK);
					col++;
				}
				tmp = dyn_append_c(tmp, mark_it);
				col++;
			}
			if (lp_->from) {
				tmp = dyn_append_c(tmp, BLANK);
				col++;
			}
			state = 2;
			/* fall-thru */

		case 2:	/* in-line */
			break;

		case 3:	/* fill the remainder of the line */
			if (mark_it) {
				while (col <= (w_opt - r_margin)) {
					tmp = dyn_append_c(tmp, BLANK);
					col++;
				}
				tmp = dyn_append_c(tmp, mark_it);
			}
			tmp = dyn_append_c(tmp, '\n');
			col     = 0;
			state   = to_newline ? 0 : 1;
			continue;
		}

		/* check to see if the latest word will fit */
		if ((src == t_bfr || !isspace(src[-1])))
			;
		else if ((skip_text(src)-src) + col > (w_opt - r_margin)) {
			state = 3;
			/* FIXME */
			if (dyn_length(tmp)
			 && dyn_string(tmp)[dyn_length(tmp)-1] != '\n') {
				col--;	/* back up before this blank */
				tmp->cur_length--;
			}
			continue;
		}

		if (*src == '\n') {
			src++;
			state = 3;
			continue;
		}
		/* patch: count col special for escaped '%' */
		tmp = dyn_append_c(tmp, *src++);
		col++;
	}

	/* fill the last line */
	if (col != 0) {
		if (mark_it) {
			while (col <= (w_opt - r_margin)) {
				tmp = dyn_append_c(tmp, BLANK);
				col++;
			}
		}
		if (lp_->to) {
			if (mark_it) {
				tmp = dyn_append_c(tmp, mark_it);
				tmp = dyn_append_c(tmp, '\n');
				col = 0;
				if (lp_->from && to_newline) {
					col = strlen(lp_->from);
					tmp = dyn_append(tmp, lp_->from);
				}
				while (col < lp_->column-1) {
					tmp = dyn_append_c(tmp, BLANK);
					col++;
				}
				while (col++ <= (w_opt - r_margin + to_newline)) {
					tmp = dyn_append_c(tmp, mark_it);
				}
			}
			if (!to_newline) {
				for (src = lp_->to; *src; src++) {
					tmp = dyn_append_c(tmp, *src);
				}
			}
		}
		tmp = dyn_append_c(tmp, '\n');
	}

	if (!lp_->to) {
		tmp = dyn_append_c(tmp, '\n');
	}

	tmp = dyn_append_c(tmp, EOS);
	lp_->format = stralloc(dyn_string(tmp));
}
