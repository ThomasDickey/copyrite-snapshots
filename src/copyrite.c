#ifndef	lint
static	char	Id[] = "$Header: /users/source/archives/copyrite.vcs/src/RCS/copyrite.c,v 2.0 1991/12/13 12:47:39 ste_cm Rel $";
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
#define	DIFF_TOOL "diff/slp/out="
#else
#define PATH_END '/'
#define	DIFF_TOOL "diff"
#endif

#define	STAT	struct stat
#define	SIZEOF(v) (sizeof(v)/sizeof(v[0]))

#define	WARN	FPRINTF(stderr,
#define	TELL	if (verbose >= 0) PRINTF
#define	VERBOSE	if (verbose >= 1) PRINTF

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
		{"ada",    "--",     "\n",   0,    '-',   0,   0 },
		{"c",      "/*",     "*/",   0,    '*',   0,   2 },
		{"dcl",    "$!",     "\n",   0,    '!',   0,   0 },
		{"ftn",    "C*",     "\n",   0,    '*',   0,   0 },
		{"lex",    "/*",     "*/",   "%{", '*',   0,   2 },
		{"lex2",   "/*",     "*/",   "%%", '*',   0,   2 },
		{"make",   "#",      "\n",   0,    '#',   0,   0 },
		{"man",    ".\\\"*", "\n",   0,    '*',   0,   0 },
		{"pas",    "(*",     "*)",   0,    '*',   0,   2 },
		{"shell",  "#",      "\n",   0,    '#',   1,   0 },
		{"text",   0,        0,      "--", 0,     0,   0 },
		{"x",      "!",      "\n",   0,    '!',   0,   0 }
	};

/* options */
static	int	c_opt	= FALSE;	/* enables "(c)" */
static	int	force	= FALSE;	/* force: overwrite input files */
static	char *	l_opt	= "none";	/* specify default for unknown-type */
static	int	L_opt	= FALSE;	/* symbolic-links */
static	int	no_op	= FALSE;	/* no-op to do diff-only */
static	int	r_opt	= 0;		/* true to recur on directories */
static	int	T_opt	= FALSE;	/* true if we touch-files */
static	int	w_opt	= 80;		/* width of comment-notice */
static	int	verbose	= FALSE;	/* level of verbosity */

/* global data */
static	char	*Owner, *Disclaim;
static	char	old_wd[MAXPATHLEN];

/************************************************************************
 *	local procedures						*
 ************************************************************************/

#ifdef	vms
static
void
StripVersion(
_AR1(char *,	name)
	)
_DCL(char *,	name)
{
	register char	*s = strrchr(name, ';');
	if (s != 0)
		*s = EOS;
}
#else
#define	StripVersion(name);
#endif

/*
 * Copy the date from the input file to the output, since they are
 * really the same, irregardless of their representation.
 */
static
copydate(
_ARX(STAT *,	sb)
_AR1(char *,	out_name)
	)
_DCL(STAT *,	sb)
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
 * Find the path-leaf from syntax only
 */
static
char *
LeafOf(
_AR1(char *,	path))
_DCL(char *,	path)
{
	register char *s = strrchr(path, PATH_END);
	if (s)
		path = s + 1;
	return path;
}

/*
 * Skip text/whitespace/line
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

static
char *
skip_line(
_AR1(char *,	src))
_DCL(char *,	src)
{
	while ((*src != EOS) && (*src++ != '\n'));
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

#ifdef	vms
static
char *
same_name(
_ARX(char *,	cmp)
_AR1(char *,	ref)
	)
_DCL(char *,	cmp)
_DCL(char *,	ref)
{
	char	temp[MAXPATHLEN];
	char	*s, *d, cc;
	int	ok = FALSE;

	StripVersion(strcpy(temp, ref));
	for (s = temp, d = cmp; *s && *d; s++, d++);	/* verify lengths */

	if (cc = *d) {		/* non-null iff normal ident */
		*d = EOS;
		ok = !strucmp(temp, cmp);
		*d = cc;
	}
	return ok ? d : 0;
}
#else
#define	same_name(cmp,ref) exact(cmp,ref)
#endif

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

	unsigned need;
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
		/* patch: count col special for escaped '%' */
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
				if (lp_->from && to_newline) {
					col = strlen(strcpy(dst, lp_->from));
					dst += col;
				}
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
	static	struct	{
		char	*pattern, *name;
	} table[] = {
		"*.a",		"ada",
		"*.ada",	"ada",
		"*.ea",		"ada",	/* Interbase */
		"*.c",		"c",
		"*.h",		"c",
		"*.e",		"c",	/* Interbase */
		"*.gdl",	"c",	/* Interbase */
		"*.qli",	"c",	/* Interbase */
		"llib-l*",	"c",
		"*.com",	"dcl",
		"*.f",		"ftn",
		"*.ftn",	"ftn",
		"*.for",	"ftn",
		"*.l",		"lex",
		"*.y",		"lex",
		"*.man",	"man",
		"*.mms",	"make",
		"*.mk",		"make",
		"makefile",	"make",
		"imakefile",	"make",
		"amakefile",	"make",
		"*.p",		"pas",
		"*.pas",	"pas",
		"*.sh",		"shell",
		"*.csh",	"shell"
	};
	char	*it;
	char	temp[MAXPATHLEN];
	register int	j;

	StripVersion(name = LeafOf(strlcpy(temp, name)));

	/* first, try to decode it based only on name + command-options */
	if (force && strcmp(l_opt, "none"))
		it = l_opt;
	else {
		it = 0;
		for (j = 0; j < SIZEOF(table); j++) {
			if (!strwcmp(table[j].pattern, name)) {
				it = table[j].name;
				break;
			}
		}
		/* test special cases in which suffix does not suffice */
		if (it != 0) {
			if (!strcmp(it, "dcl") && (*buffer != '$'))
				it = 0;
		} else {
			char	*type = ftype(name);
			if (*type && type[1] && isdigit(type[1]))
				it = "man";
		}
	}

	/* decode special cases based on the file's contents */
	if (it == 0) {
		if (*buffer == '!')
			it = "x";
		else if (*buffer == ':' || *buffer == '#' || *buffer == '\n')
			it = "shell";
		else
			it = l_opt;
	}

	for (j = 0; j < SIZEOF(Languages); j++)
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

	name = LeafOf(name);

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
			if ((s = same_name(s, name))
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
			if (s = same_name(s, name))
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
	FCLOSE(ifp);
}

/*
 * Write the file with notice inserted
 */
static
void
writeit(
_ARX(char *,	out_name)
_ARX(char *,	in_name)
_ARX(STAT *,	sb)
_ARX(LANG *,	it)
_ARX(char *,	buffer)
_AR1(int,	used)
	)
_DCL(char *,	out_name)
_DCL(char *,	in_name)
_DCL(STAT *,	sb)
_DCL(LANG *,	it)
_DCL(char *,	buffer)
_DCL(int,	used)
{
	auto	FILE	*ofp;
	auto	struct	tm tm;
	auto	char	*s;
	auto	int	f_got;
#ifdef	unix
	auto	int	fd;
#endif

	/*
	 * Open a temporary file in the same directory as the input file,
	 * which we will write to.  For VMS, we write a new version of the
	 * input file to simplify manipulation of its protection.
	 */
#ifdef	vms
	if (s = strrchr(strlcpy(out_name, in_name), ';'))
		*(++s) = EOS;
	else
		(void)strcat(out_name, ";");
	if (!(ofp = fopen(out_name, "w")))
		failed(out_name);
#else	/* unix */
	if (s = strrchr(strcpy(out_name, in_name), PATH_END))
		s++;
	else
		s = out_name;
	*s = EOS;
	if ((fd = mkstemp(strcat(out_name, "XXXXXX"))) < 0)
		failed("mkstemp");
	if (!(ofp = fdopen(fd, "w")))
		failed("fdopen");
#endif	/* vms/unix */

	/*
	 * Write the portion of the file before the notice
	 */
	if (used) {
		(void)fwrite(buffer, sizeof(char), used, ofp);
		VERBOSE("\n# skip %d lines (%d bytes)", it->line, used);
	}

	/*
	 * Apply the copyright-notice
	 */
	if (T_opt) {
		time_t	now = time((time_t *)0);
		tm = *localtime(&now);
	} else
		tm = *localtime(&sb->st_mtime);

	FPRINTF(ofp, it->format, tm.tm_year + 1900);
	f_got = fwrite(buffer+used, sizeof(char), strlen(buffer+used), ofp);
	VERBOSE("\n# remainder of %s (%d bytes)", in_name, f_got);
	FCLOSE(ofp);
}

/*
 * Translate a single file
 */
editfile(	/* duplicates name from 'portunix' library */
_ARX(char *,	in_name)
_FNX(int,	func)
_AR1(STAT *,	sb)
	)
_DCL(char *,	in_name)
_DCL(int,	(*func)())
_DCL(STAT *,	sb)
{
	static	unsigned f_max;
	static	char	*f_bfr;

	auto	LANG	*it;
	auto	int	f_got;
	auto	unsigned f_use;
	auto	FILE	*ifp;

	auto	char	my_path[MAXPATHLEN];
	auto	char	my_name[MAXPATHLEN];
	auto	char	*s;
	auto	int	used, mode;

	/*
	 * Compute a relative pathname for display purposes (simpler to read)
	 *
	 * Note that on UNIX, 'transtree()' does not give me an absolute
	 * pathname.
	 */
#ifdef	vms
	vms_relpath(my_path, old_wd, strlcpy(my_path, in_name));
#else	/* unix */
	if (getwd(my_path)) {
		relpath(my_path, old_wd, pathcat(my_path, my_path, in_name));
	} else	/* assume directory is not readable */
		(void)strcpy(my_path, in_name);
#endif	/* vms/unix */

	TELL("%s ", my_path);
	VERBOSE("\n# size: %d bytes", sb->st_size);

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
	if (ifp = fopen(in_name, "r")) {
		/* patch: later, try test-read of first block for binary-text */
		f_got = fread(f_bfr, sizeof(char), (int)sb->st_size, ifp);
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

	/*
	 * if no ident, skip this
	 */
	if (!force && !Identifier(in_name, f_bfr)) {
		TELL("(no ident)\n");
		return 0;
	}

	/* if conflicting notice, skip this */
	if (Conflict(f_bfr)) {
		TELL("(prior)\n");
		return 0;
	}

	/*
	 * Determine the language-type, so we know how to comment the notice
	 */
	if (!(it = DecodeLanguage(in_name, f_bfr))) {
		TELL("(unknown)\n");
		return 0;
	}

	/*
	 * We probably will write the marked-up file -- if we can find the
	 * insertion point.
	 */
	used = 0;

	if (it->line) {
		/* look for line-number */
		register int	line;
		for (s = f_bfr, line = 0; line < it->line; line++)
			if (!*(s = skip_line(s)))
				break;
		if (line < it->line) {
			TELL("(after line %d?)\n", it->line);
			return 0;
		}
		used = (s - f_bfr);
	}

	if (it->after) {
		register char	*s = f_bfr + used;

		/* look for line beginning with marker */
		while (strncmp(s, it->after, strlen(it->after))) {
			if (!*(s = skip_line(s))) {
				TELL("(after %s)\n", it->after);
				return 0;
			}
		}
		s = skip_line(s);
		used = (s - f_bfr);
	}

	writeit(my_name, in_name, sb, it, f_bfr, used);

	if (no_op) {
		if (no_op < 2) {
			char	temp[BUFSIZ + (MAXPATHLEN * 2)];
#ifdef	vms
			char	my_verb[BUFSIZ];
			char	my_temp[MAXPATHLEN];
			FILE	*tfp;

			*temp = EOS;
			catarg(temp, strlcpy(my_temp, in_name));
			catarg(temp, my_name);

			if (s = strrchr(strcpy(my_temp, my_path), PATH_END))
				s++;
			else
				s = my_temp;
			if (!tmpnam(my_temp))
				failed("tmpnam");
			if (!strrchr(my_temp, '.'))
				(void)strcat(my_temp, ".");
			FORMAT(my_verb, "%s%s", DIFF_TOOL, my_temp);
			PRINTF("\n%% %s%s %.*s\n", DIFF_TOOL,
				verbose > 1 ? my_temp : "sys$output",
				strrchr(my_path,';')-my_path+1, my_path);
			if (verbose > 1)
				shoarg(stdout, my_verb, temp);
			FFLUSH(stdout);
			FFLUSH(stderr);
			(void)execute(my_verb, temp);
			if (tfp = fopen(my_temp, "r")) {
				while ((f_got = fread(temp, sizeof(char), sizeof(temp), tfp)) > 0)
					fwrite(temp, sizeof(char), f_got, stdout);
				FCLOSE(tfp);
				(void)unlink(my_temp);
			}
#else	/* unix */
			*temp = EOS;
			catarg(temp, my_path);
			catarg(temp, my_name);
			PRINTF("\n%% %s %s\n", DIFF_TOOL, my_path);
			FFLUSH(stdout);
			FFLUSH(stderr);
			(void)execute(DIFF_TOOL, temp);
#endif	/* vms/unix */
		}
		if (unlink(my_name))
			failed(my_name);
	} else {
		/* rename temp-file to the output */
		if (!T_opt)
			copydate(sb, my_name);
#ifdef	unix
		if (rename(my_name, in_name) < 0)
			failed("rename");
#endif
	}
	mode = sb->st_mode & 0777;
	VERBOSE("\n%% chmod %03o %s", mode, in_name);
	if (!no_op) {
		if (chmod(my_name, mode) < 0)
			failed("chmod");
	}

	TELL("\n");
	return 1;
}

usage(_AR0)
{
	static	char	*tbl[] = {
		"usage: copyrite [options] files",
		"",
		"options:",
		" -c         enable \"(c)\" marker (non-statutory)",
		" -e FILE    redirect standard error to the specified file",
		" -f         (force) markup files w/o RCS or SCCS ident",
		" -l LANG    specify default language for unknown cases (none)",
		" -L         follow symbolic links",
		" -m FILE    specify owner+disclaimer",
		" -n         (no-op) write result to temp-file, do diffs",
		" -o FILE    redirect standard output to the specified file",
		" -q         (quiet) suppress informational messages",
		" -R         recur into directories",
		" -t         touch files with current date",
		" -v         (verbose)",
		" -w NUMBER  set width of notice-comment (default: 80)",
		0};
	register int	j = 0;
	while (tbl[j])
		WARN "%s\n", tbl[j++]);
	WARN "\nlanguages:");
	for (j = 0; j < SIZEOF(Languages); j++)
		WARN "%c %s", j ? ',' : ' ', Languages[j].name);
	WARN "\n");
	exit(FAIL);
	/*NOTREACHED*/
}

static
int dummy()
{
}

/*ARGSUSED*/
_MAIN
{
	char	*m_opt	= 0;
	register int	j;

	while ((j = getopt(argc, argv, "ce:fl:Lm:no:qRtvw:")) != EOF) {
		switch (j) {
		case 'c':	c_opt++;			break;
		case 'e':	if (!freopen(optarg, "a", stderr))
					failed(optarg);
				break;
		case 'f':	force++;			break;
		case 'l':	l_opt = optarg;			break;
		case 'L':	L_opt++;			break;
		case 'm':	m_opt = optarg;			break;
		case 'n':	no_op++;			break;
		case 'o':	if (!freopen(optarg, "a", stdout))
					failed(optarg);
				break;
		case 'q':	verbose--;			break;
		case 'R':	r_opt++;			break;
		case 't':	T_opt++;			break;
		case 'v':	verbose++;			break;
		case 'w':	w_opt = atoi(optarg);		break;
		default:	usage();
		}
	}

	if (!getwd(old_wd))
		failed("getwd: old_wd");

	if (no_op && (verbose < 0))
		verbose = 0;

	if (!m_opt) {
		char	temp[BUFSIZ];
#ifdef	vms
		(void)strcpy(strrchr(strcpy(temp, argv[0]), '.'), ".txt");
#else	/* unix */
		if (which(temp, sizeof(temp), argv[0], ".") <= 0)
			failed("which am I");
		(void)strcat(temp, ".txt");
#endif
		m_opt = stralloc(temp);
	}

	LoadTemplate(m_opt);

	if (optind >= argc) {
		if (r_opt)
			edittree(EDITDIR_ARG, dummy, r_opt, L_opt);
		else
			usage();
	}
	for (j = optind; j < argc; j++)
		edittree(argv[j], dummy, r_opt, L_opt);
	exit(SUCCESS);
	/*NOTREACHED*/
}
