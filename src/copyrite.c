#ifndef	lint
static	char	Id[] = "$Header: /users/source/archives/copyrite.vcs/src/RCS/copyrite.c,v 1.14 1991/12/10 16:05:28 dickey Exp $";
#endif

/*
 * Title:	copyrite.c
 * Author:	T.E.Dickey
 * Created:	11 Dec 1991
 *
 * Function:	Prepends/updates copyright-notice to one or more files.
 */

#define	STR_PTYPES
#include	<portunix.h>
#include	<ctype.h>
#include	<time.h>
extern	char	*mktemp();

#ifdef	vms
#define	PATH_END ']'
#else
#define PATH_END '/'
#endif

#define	WARN	FPRINTF(stderr,
#define	TELL	if (verbose >= 0) PRINTF

typedef	struct	{
	char	*name;
	char	*from,		/* begin-comment */
		*to,		/* end-comment */
		*after;		/* put notice after line beginning */
	int	box,		/* make comment-box with this */
		line,		/* put notice after line-number */
		column;		/* begin notice-box in column-number */
	char	*format;	/* sprintf-code to insert-year */
	} LANG;

static	LANG	Languages[] = {
		/*name     from      to      after box    line col */
		{"none",   0,        0,      "--", 0,     0,   0 },
		{"ada",    "--",     "\n",   0,    '-',   0,   0 },
		{"c",      "/*",     "*/",   0,    '*',   0,   2 },
		{"ftn",    "C*",     "\n",   0,    '*',   0,   0 },
		{"man",    ".\\\"*", "\n",   0,    '*',   0,   0 },
		{"x",      "!",      "\n",   0,    '!',   0,   0 },
		{"make",   "#",      "\n",   0,    '#',   0,   0 },
		{"shell",  "#",      "\n",   0,    '#',   1,   0 },
		{"dcl",    "$!",     "\n",   0,    '!',   0,   0 }
	};

/* options */
static	int	c_opt	= FALSE;	/* enables "(c)" */
static	int	force	= FALSE;	/* force: overwrite input files */
static	int	L_opt	= FALSE;	/* symbolic-links */
static	int	no_op	= FALSE;	/* no-op to do diff-only */
static	int	r_opt	= 0;		/* true to recur on directories */
static	int	T_opt	= FALSE;	/* true if we touch-files */
static	int	w_opt	= 80;		/* width of comment-notice */
static	int	verbose	= FALSE;	/* level of verbosity */

/* global data */
static	char	*Owner, *Disclaim;

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Copy the date from the input file to the output, since they are
 * really the same, irregardless of their representation.
 */
static
copydate(
_ARX(struct stat *,sb)
_AR1(char *,	out_name)
	)
_DCL(struct stat *,sb)
_DCL(char *,	out_name)
{
	auto	struct	timeval tv[2];
	tv[0].tv_sec  = time((time_t *)0);
	tv[0].tv_usec = 0;
#ifdef	vms
	tv[1].tv_sec  = sb->st_ctime;
#else	/* unix */
	tv[1].tv_sec  = sb->st_mtime;
#endif	/* vms/unix */
	tv[1].tv_usec = 0;
	(void) utimes(out_name, tv);
}

/*
 * Test for binary-file
 */
static
isbinary(
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
 * Find the given character within the current line, or return null
 */
static
char *
inline(
_ARX(char *,	cmp)
_AR1(int,	ref)
	)
_DCL(char *,	cmp)
_DCL(int,	ref)
{
	while (*cmp && *cmp != '\n')
		if (*cmp++ == ref)
			return cmp;
	return 0;
}

/*
 * Skip text
 */
static
char *
skip_text(
_AR1(char *,	src))
_DCL(char *,	src)
{
	while (*src && !isspace(*src))
		src++;
	return src;
}

static
char *
skip_white(
_AR1(char *,	src))
_DCL(char *,	src)
{
	while (*src && isspace(*src))
		src++;
	return src;
}

/*
 * Match an arbitrary (exact-case) keyword
 */
static
char *
exact(
_ARX(char *,	cmp)
_AR1(char *,	ref)
	)
_DCL(char *,	cmp)
_DCL(char *,	ref)
{
	while (*ref)
		if (*cmp == 0 || (*ref++ != *cmp++))
			return 0;
	return cmp;
}

/*
 * Convert character to uppercase if it is alphabetic
 */
static
int
Upper(
_AR1(int,	c))
_DCL(int,	c)
{
	if (isalpha(c)) {
		if (islower(c))
			c = toupper(c);
	} else if (isspace(c))
		c = ' ';
	return c;
}

/*
 * Compare, looking for word(s), ignoring punctuation, whitespace-type and case
 */
static
char *
any_case(
_ARX(char *,	cmp)
_AR1(char *,	ref)
	)
_DCL(char *,	cmp)
_DCL(char *,	ref)
{
	while (*ref) {
		int	a = Upper(*ref++),
			b = Upper(*cmp++);
		if (a != b)
			return 0;
	}
	return cmp;
}

/*
 * Look for something like a list of years.  Allow "(c)" around it
 */
static
char *
any_year(
_AR1(char *,	buffer)
	)
_DCL(char *,	buffer)
{
	register char	*s, *t;
	int	got_year = FALSE;

	s = buffer;
	while (*s) {
		s = skip_white(s);
		if (t = any_case(s, "(c)")) {
			s = t;
			continue;
		}
		if (isdigit(*s)) {
			got_year = TRUE;
			while (isdigit(*s))
				s++;
		} else if (*s == ',') {
			s++;
		} else {
			s = skip_white(s);
			break;
		}
	}
	return got_year ? s : 0;
}

/*
 * Look for a prior instance of copyright notice in the current file
 */
static
int
Conflict(
_AR1(char *,	buffer)
	)
_DCL(char *,	buffer)
{
	register char *s = buffer, *t;

	while (*s) {
		if (t = any_case(s, "copyright")) {
			if (t = any_year(t))
				return TRUE;
		}
		s++;
	}
	return FALSE;
}

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
		for (s = dst; *s = *src++; s++)
			if (*s == '%')
				*(++s) = '%';
		src = dst;
	}
	return src;
}

/*
 * Generate a notice-format for the given language
 */
static
void
FormatNotice(
_AR1(LANG *,	lp_))
_DCL(LANG *,	lp_)
{
	static	int   t_len;
	static	char *t_bfr;
	static	char *fmt = "Copyright %s%%04d %s%s%sAll Rights Reserved.\n%s%s";

	auto	int	to_newline, first, mark_it;
	auto	int	col, r_margin, state;
	auto	char	*it, *dst, *src;

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

		FORMAT(t_bfr, fmt,
			circle,
			NoPercent(Owner),
			period ? "." : "",
			newline ? "\n" : "  ",
			*Disclaim ? "\n" : "",
			NoPercent(Disclaim));
		t_len = strlen(t_bfr);
	}
	if (lp_->format != 0)
		return;

	it = doalloc((char *)0, (unsigned)(t_len + t_len + w_opt * 3));
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
				(void)strcat(dst, lp_->from);
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
						(void)strcat(dst, lp_->from);
						dst += col;
					}
				}
			}
			/* fall-thru */

		case 1:	/* begin continuation */
			if (col < lp_->column) {
				while (col < lp_->column-1) {
					*dst++ = ' ';
					col++;
				}
				*dst++ = mark_it;
				col++;
			}
			if (lp_->from) {
				*dst++ = ' ';
				col++;
			}
			state = 2;
			/* fall-thru */

		case 2:	/* in-line */
			break;

		case 3:	/* fill the remainder of the line */
			if (mark_it) {
				while (col <= (w_opt - r_margin)) {
					*dst++ = ' ';
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
			continue;
		}

		if (*src == '\n') {
			src++;
			state = 3;
			continue;
		}
		*dst++ = *src++;
		col++;
	}

	/* fill the last line */
	if (col != 0) {
		if (mark_it) {
			while (col <= (w_opt - r_margin)) {
				*dst++ = ' ';
				col++;
			}
		}
		if (lp_->to) {
			if (mark_it) {
				*dst++ = mark_it;
				*dst++ = '\n';
				col = 0;
				while (col < lp_->column-1) {
					*dst++ = ' ';
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
	*dst = EOS;

	lp_->format = it;
}

/*
 * Test for RCS or SCCS identifier
 */
static
LANG *
DecodeLanguage(
_ARX(char *,	name)
_AR1(char *,	buffer)
	)
_DCL(char *,	name)
_DCL(char *,	buffer)
{
	char	*type = ftype(name);
	char	*it	= "none";

	register int	j;

	if (!strcmp(type, ".c")
	 || !strcmp(type, ".e")
	 || !strcmp(type, ".h"))
		it = "c";
	else if (
	    !strcmp(type, ".a")
	 || !strcmp(type, ".ada"))
		it = "ada";
	else if (
	    !strcmp(type, ".com"))
		it = "dcl";
	else if (
	    !strcmp(type, ".mms"))
		it = "make";
	else if (*buffer == '!')
		it = "x";
	else if (
	    !strucmp(name, "makefile")
	 || !strucmp(name, "amakefile")
	 || !strucmp(name, "imakefile"))
		it = "make";
	else if (!strucmp(name, "copyright"))
		it = "none";
	else if (*buffer == ':' || *buffer == '#' || *buffer == '\n')
		it = "shell";

	for (j = 0; j < sizeof(Languages)/sizeof(Languages[0]); j++)
		if (!strcmp(Languages[j].name, it)) {
			FormatNotice(Languages+j);
			return Languages+j;
		}
	return 0;
}

/*
 * Test for RCS or SCCS identifier
 */
static
Identifier(
_ARX(char *,	name)
_AR1(char *,	buffer)
	)
_DCL(char *,	name)
_DCL(char *,	buffer)
{
	char	*s, *t, *d, c;

	if (s = strrchr(name, PATH_END))
		name = s + 1;

	s = buffer;
	while (t = strchr(s, '$')) {
		t++;
		if (((s = exact(t, "Id:"))
		  || (s = exact(t, "Header:")))
		 && inline(t,'$')) {
			/* RCS identifier can have pathname prepended */
			s = skip_white(s);
			d = skip_text(s);
			c = *d;
			*d = EOS;
			while (inline(s, '/'))
				s++;
			*d = c;
			if ((s = exact(s, name))
			 && (s = exact(s, ",v"))
			 && (s == d))
				return TRUE;
		}
		s = t;
	}

	s = buffer;
	while (t = strchr(s, '@')) {
		t++;
		if (s = exact(t, "(#)")) {
			s = skip_text(s);	/* module-name, if any */
			s = skip_white(s);
			if (s = exact(s, name))
				return TRUE;
		}
		s = t;
	}
	return FALSE;
}

/*
 * Load the copyright-template. The first line is the owner. Subsequent lines
 * are the disclaimer, if any.
 */
static
LoadTemplate(
_AR1(char *,	name))
_DCL(char *,	name)
{
	FILE	*ifp = fopen(name, "r");
	char	temp[BUFSIZ];
	int	first	= TRUE;

	if (!ifp)
		failed(name);
	while (fgets(temp, sizeof(temp), ifp)) {
		if (!strclean(temp))
			continue;
		if (first) {
			first = FALSE;
			Owner = stralloc(temp);
			Disclaim = stralloc("");
		} else {
			unsigned need = strlen(Disclaim) + 2 + strlen(temp);
			Disclaim = doalloc(Disclaim, need);
			if (*Disclaim)
				(void)strcat(Disclaim, " ");
			(void)strcat(Disclaim, temp);
		}
	}
	if (!Owner) {
		FPRINTF(stderr, "? %s is empty\n", name);
		exit(FAIL);
	}
	FormatNotice(Languages);
	FCLOSE(ifp);
}

/*
 * Translate a single file
 */
static
do_file(
_ARX(char *,	in_name)
_AR1(struct stat *,sb)
	)
_DCL(char *,	in_name)
_DCL(struct stat *,sb)
{
	static	unsigned f_max;
	static	char	*f_bfr;

	auto	LANG	*it;
	auto	int	f_got;
	auto	unsigned f_use;
	auto	FILE	*ifp;

	auto	char	my_name[MAXPATHLEN];
	auto	char	*s;
	auto	int	fd, used;
	auto	FILE	*ofp;
	auto	struct	tm tm;

	/*
	 * load file into memory
	 */
	if (sb->st_size <= 0) {
		TELL("** %s (empty)\n", in_name);
		return;
	}
	if (f_max <= sb->st_size) {
		f_max = (f_max * 9)/8;
		if (f_max <= sb->st_size)
			f_max = (sb->st_size * 9)/8 + BUFSIZ;
		f_bfr = doalloc(f_bfr, f_max);
	}
	if (ifp = fopen(in_name, "r")) {
		f_got = fread(f_bfr, sizeof(char), (int)sb->st_size, ifp);
		FCLOSE(ifp);
		if (f_got < 0) {
			TELL("** %s (no data)\n", in_name);
			return;
		}
		f_bfr[f_use = f_got] = EOS;
	} else {
		perror(in_name);
		return;
	}

	/*
	 * if binary, skip this
	 */
	if (isbinary(f_bfr, f_use)) {
		TELL("** %s (binary)\n", in_name);
		return;
	}

	/*
	 * if no ident, skip this
	 */
	if (!force && !Identifier(in_name, f_bfr)) {
		TELL("** %s (no ident)\n", in_name);
		return;
	}

	/* if conflicting notice, skip this */
	if (Conflict(f_bfr)) {
		TELL("** %s (prior)\n", in_name);
		return;
	}

	/*
	 * Open a temporary file in the same directory as the input file,
	 * which we will write to.
	 */
	if (s = strrchr(strcpy(my_name, in_name), PATH_END))
		s++;
	else
		s = my_name;
	*s = EOS;
	if ((fd = mkstemp(strcat(my_name, "XXXXXX"))) < 0)
		failed("mkstemp");
	if (!(ofp = fdopen(fd, "w")))
		failed("fdopen");

	/*
	 * Determine the language-type, so we know how to comment the notice
	 */
	it = DecodeLanguage(in_name, f_bfr);

	/*
	 * Mark-up the file
	 */
	used = 0;
	if (it->line) {
		for (s = f_bfr; used < it->line && *s; s++)
			if (*s == '\n')
				used++;
		(void)fwrite(f_bfr, sizeof(char), used = (s - f_bfr), ofp);
	}

	if (T_opt) {
		time_t	now = time((time_t *)0);
		tm = *localtime(&now);
	} else
		tm = *localtime(&sb->st_mtime);

	FPRINTF(ofp, it->format, tm.tm_year + 1900);
	(void)fwrite(f_bfr+used, sizeof(char), strlen(f_bfr+used), ofp);
	FCLOSE(ofp);
	(void)chmod(my_name, (int)(sb->st_mode & 0777));

	if (no_op) {
		char	temp[BUFSIZ];
		PRINTF("%% chmod %03o %s\n", sb->st_mode & 0777, my_name);
		FORMAT(temp, "diff %s %s", my_name, in_name);
		PRINTF("%% %s\n", temp);
		FFLUSH(stdout);
		FFLUSH(stderr);
		(void)system(temp);
		(void)unlink(my_name);
	} else {
		/* rename temp-file to the output */
		if (!T_opt)
			copydate(sb, my_name);
		if (rename(my_name, in_name) < 0)
			failed("in_name");
	}
	PRINTF("** %s\n", in_name);
}

usage(_AR0)
{
	static	char	*tbl[] = {
		"usage: copyrite [options] files",
		"",
		"options:",
		" -c         enable \"(c)\" marker (non-statutory)",
		" -f         (force) markup files w/o RCS or SCCS ident",
		" -L         follow symbolic links",
		" -n         (no-op) write result to temp-file, do diffs",
		" -q         (quiet) suppress informational messages",
		" -r         recur into directories",
		" -t FILE    specify owner+disclaimer",
		" -T         touch files with current date",
		" -v         (verbose)",
		" -w NUMBER  set width of notice-comment (default: 80)",
		0};
	register int	j = 0;
	while (tbl[j])
		WARN "%s\n", tbl[j++]);
	exit(FAIL);
	/*NOTREACHED*/
}

/*ARGSUSED*/
_MAIN
{
	char	*t_opt	= 0;
	register int	j;

	while ((j = getopt(argc, argv, "cfLnqrtTvw:")) != EOF) {
		switch (j) {
		case 'c':	c_opt++;			break;
		case 'f':	force++;			break;
		case 'L':	L_opt++;			break;
		case 'n':	no_op++;			break;
		case 'q':	verbose--;			break;
		case 'r':	r_opt++;			break;
		case 't':	t_opt = optarg;			break;
		case 'T':	T_opt++;			break;
		case 'v':	verbose++;			break;
		case 'w':	w_opt = atoi(optarg);		break;
		default:	usage();
		}
	}

	if (!t_opt) {
		char	temp[BUFSIZ];
#ifdef	vms
		(void)strcpy(temp, argv[0]);
		temp[strrchr(temp, '.')-temp] = EOS;
#else	/* unix */
		if (which(temp, sizeof(temp), argv[0], ".") <= 0)
			failed("which am I");
		(void)strcat(temp, ".txt");
#endif
		t_opt = stralloc(temp);
	}

	LoadTemplate(t_opt);

	if (optind >= argc) {
		if (r_opt)
			transtree(OPENDIR_ARG, do_file, r_opt, L_opt);
		else
			usage();
	}
	for (j = optind; j < argc; j++)
		transtree(argv[j], do_file, r_opt, L_opt);
	exit(SUCCESS);
	/*NOTREACHED*/
}
