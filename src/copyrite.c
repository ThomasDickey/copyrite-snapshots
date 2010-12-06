/*
 * Title:	copyrite.c
 * Author:	T.E.Dickey
 * Created:	11 Dec 1991
 * Modified:
 *		19 Jun 2004, remove K&R code, indent'd.
 *		13 Jan 1998, add "awk", "html" types.  Add "-a" option.
 *		20 Jul 1997, allow multi-paragraph disclaimer.
 *		24 Dec 1996, add c++ types
 *		30 Nov 1996, add .m4, and DEC-runoff types
 *		01 Dec 1993, ifdefs, TurboC warnings.
 *		22 Sep 1993, gcc warnings.
 *		06 May 1993, corrected vms mtime reference.
 *		27 Apr 1993, added COPYING, README to text-types.
 *		17 Jul 1992, check for unchanged-files.
 *		30 Jun 1992, added ".tmpl" entry for EBPM-template
 *		04 Jun 1992, added ".lsp" entry for Interleaf-lisp
 *		04 Jun 1992, in no-op mode it is ok to put temp-file in /tmp.
 *		15 May 1992, added entry for Common-Lisp
 *
 * Function:	Prepends/updates copyright-notice to one or more files.
 *
 * patch:	should modify 'insert_at()' to combine 'after' and 'line'
 *		so I can specify that insertion takes place after both are
 *		satisfied, rather than sequentially.
 *
 * patch:	this should handle 8-bit characters
 */

#ifdef	__TURBOC__
#define	OPN_PTYPES
#define	mkstemp(s) open(s, O_RDWR)
#endif

#include "copyrite.h"

MODULE_ID("$Id: copyrite.c,v 5.16 2010/12/05 20:27:27 tom Exp $")

#ifdef	vms
#define	ST_MTIME	st_ctime
#else
#define	ST_MTIME	st_mtime
#endif

#define DATA(name, from, to, after, box, line, col) \
	{name, from, to, after, box, line, col, 0}
	/* *INDENT-OFF* */
static	LANG	Languages[] = {
		/*name     from      to      after box    line col */
	DATA(	"ada",    "--",     "\n",   0,    '-',   0,   0 ),
	DATA(	"awk",    "#",      "\n",   "#",  '#',   0,   0 ),
	DATA(	"c",      "/*",     "*/",   0,    '*',   0,   2 ),
	DATA(	"c++",    "//",     "\n",   0,    '*',   0,   0 ),
	DATA(	"dcl",    "$!",     "\n",   0,    '!',   0,   0 ),
	DATA(	"ftn",    "C*",     "\n",   0,    '*',   0,   0 ),
	DATA(	"html",   "<!--",   "-->",  0,    '*',   0,   2 ),
	DATA(	"lex",    "/*",     "*/",   "%{", '*',   0,   2 ),
	DATA(	"lex",    "/*",     "*/",   "%%", '*',   0,   2 ),
	DATA(	"lsp",    ";",      "\n",   0,    '*',   0,   2 ),
	DATA(	"make",   "#",      "\n",   0,    '#',   0,   0 ),
	DATA(	"man",    ".\\\"",  "\n",   0,    '*',   0,   0 ),
	DATA(	"m4",     "dnl",    "\n",   0,    '*',   0,   0 ),
	DATA(	"pas",    "(*",     "*)",   0,    '*',   0,   2 ),
	DATA(	"rno",    ".;*",    "\n",   0,    '*',   0,   0 ),
	DATA(	"shell",  "#",      "\n",   0,    '#',   1,   0 ),
	DATA(	"text",   0,        0,      "--", 0,     0,   0 ),
	DATA(	"tmpl",   "#",      "\n",   "'#", '#',   0,   0 ),
	DATA(	"tmpl",   ";",      "\n",   "';", '#',   0,   0 ),
	DATA(	"x",      "!",      "\n",   0,    '!',   0,   0 ),
	DATA(	"?",      "?",      "\n",   0,    '?',   0,   0 ) /* end */
};
	/* *INDENT-ON* */

#undef DATA

/* options */
static int a_opt = FALSE;	/* disables "All Rights Reserved" */
static int c_opt = FALSE;	/* enables "(c)" */
static int f_opt = FALSE;	/* force: override ident-test */
static int F_opt = FALSE;	/* force: override owner-test */
static const char *l_opt = "none";	/* specify default for unknown-type */
static int L_opt = FALSE;	/* symbolic-links */
static int no_op = FALSE;	/* no-op to do diff-only */
static int remove_opt;		/* remove all notices */
static int recur_opt;		/* true to recur on directories */
static int strip_opt;		/* true to strip comments */
static int T_opt = FALSE;	/* true if we touch-files */
static int w_opt = 80;		/* width of comment-notice */
int verbose = FALSE;		/* level of verbosity */

/* global data */
static char *Owner, *Disclaim;
static char old_wd[MAXPATHLEN];

/************************************************************************
 *	local procedures						*
 ************************************************************************/

/*
 * Copy the date from the input file to the output, since they are
 * really the same, irregardless of their representation.
 */
static void
copydate(Stat_t * sb,
	 char *out_name)
{
#ifdef	vms
    struct timeval tv[2];
    tv[0].tv_sec = time((time_t *) 0);
    tv[0].tv_usec = 0;
    tv[1].tv_sec = sb->ST_MTIME;
    tv[1].tv_usec = 0;
    (void) utimes(out_name, tv);
#else /* unix or MSDOS */
    (void) setmtime(out_name, sb->ST_MTIME, sb->st_atime);
#endif /* patch */
}

/*
 * Test for RCS or SCCS identifier
 */
static LANG *
DecodeLanguage(const char *name,
	       const char *buffer)
{
	/* *INDENT-OFF* */
	static	struct	{
		const char	*pattern;
		const char	*name;
	} table[] = {
		{"*.a",		"ada"},
		{"*.ada",	"ada"},
		{"*.awk",	"awk"},
		{"*.ea",	"ada"},	/* Interbase */
		{"*.c",		"c"},
		{"*.cc",	"c++"},
		{"*.cpp",	"c++"},
		{"*.CC",	"c++"},
		{"*.h",		"c"},
		{"*.hh",	"c++"},
		{"*.e",		"c"},	/* Interbase */
		{"*.gdl",	"c"},	/* Interbase */
		{"*.qli",	"c"},	/* Interbase */
		{"llib-l*",	"c"},
		{"*.com",	"dcl"},
		{"*.f",		"ftn"},
		{"*.ftn",	"ftn"},
		{"*.for",	"ftn"},
		{"*.html",	"html"},
		{"*.l",		"lex"},
		{"*.lsp",	"lsp"},
		{"*.y",		"lex"},
		{"*.man",	"man"},
		{"*.mms",	"make"},
		{"*.mk",	"make"},
		{"makefile",	"make"},
		{"imakefile",	"make"},
		{"amakefile",	"make"},
		{"*.p",		"pas"},
		{"*.pas",	"pas"},
		{"*.pic",	"rno"},
		{"*.req",	"rno"},
		{"*.rno",	"rno"},
		{"*.sh",	"shell"},
		{"*.csh",	"shell"},
		{"copyright",	"text"},
		{"copying",	"text"},
		{"readme",	"text"}
	};
	/* *INDENT-ON* */

    const char *it;
    char *leaf;
    char temp[MAXPATHLEN];
    unsigned j;

    leaf = temp + (leaf_of(strlwrcpy(temp, name)) - temp);
    strip_ver(leaf);

    /* first, try to decode it based only on name + command-options */
    if (f_opt && strcmp(l_opt, "none"))
	it = l_opt;
    else {
	it = 0;
	for (j = 0; j < SIZEOF(table); j++) {
	    if (!strwcmp(table[j].pattern, leaf)) {
		it = table[j].name;
		break;
	    }
	}
	/* test special cases in which suffix does not suffice */
	if (it != 0) {
	    if (!strcmp(it, "dcl") && (*buffer != '$'))
		it = 0;
	} else {
	    char *type = ftype(leaf);
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
	    FormatNotice(Languages + j, Owner, Disclaim, a_opt, c_opt, w_opt);
	    return Languages + j;
	}
    return 0;
}

/*
 * Load the copyright-template. The first line is the owner. Subsequent lines
 * are the disclaimer, if any.
 */
static void
LoadTemplate(char *name)
{
    FILE *ifp = fopen(name, "r");
    char temp[BUFSIZ];
    int first = TRUE;

    if (!ifp)
	failed(name);
    while (fgets(temp, sizeof(temp), ifp)) {
	if (!strclean(temp)) {
	    if (first
		|| !*Disclaim
		|| Disclaim[strlen(Disclaim) - 1] == '\n')
		continue;
	    strcpy(temp, "\n\n");
	}
	if (first) {
	    first = FALSE;
	    Owner = stralloc(temp);
	    Disclaim = stralloc("");
	} else {
	    unsigned need = (unsigned) (strlen(Disclaim) + 2 + strlen(temp));
	    Disclaim = doalloc(Disclaim, need);
	    if ((*Disclaim != EOS)
		&& (Disclaim[strlen(Disclaim) - 1] != '\n')
		&& (*temp != '\n'))
		(void) strcat(Disclaim, " ");
	    (void) strcat(Disclaim, temp);
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
static unsigned
WriteIt(char *out_name,
	const char *in_name,
	int the_year,
	LANG * it,
	char *buffer,
	int used)
{
    FILE *ofp;
    char *s;
    size_t f_got;
    unsigned changes = 0;
#ifndef	vms
    int fd;
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
	(void) strcat(out_name, ";");
    if (!(ofp = fopen(out_name, "w")))
	failed(out_name);
#else /* unix */
    if ((s = strrchr(strcpy(out_name, (no_op
				       ? "/tmp/"
				       : in_name)), PATH_END)) != NULL)
	s++;
    else
	s = out_name;
    *s = EOS;
    if (((fd = mkstemp(strcat(out_name, "XXXXXX"))) < 0) != 0)
	failed("mkstemp");
    if (!(ofp = fdopen(fd, "w")))
	failed("fdopen");
#endif /* vms/unix */
    cleanup(out_name);

    /*
     * Write the portion of the file before the notice
     */
    if (used) {
	f_got = fwrite(buffer, sizeof(char), (size_t) used, ofp);
	VERBOSE("\n# skip %d lines (%lu bytes)", it->line, (unsigned long) f_got);
    }

    /*
     * Apply the copyright-notice
     */
    if (!remove_opt)
	FPRINTF(ofp, it->format, the_year);

    f_got = fwrite(buffer + used, sizeof(char), strlen(buffer + used), ofp);
    VERBOSE("\n# remainder of %s (%lu bytes)", in_name, (unsigned long) f_got);
    FCLOSE(ofp);

    return changes;
}

/*
 * Translate a single file
 * duplicates name from 'portunix' library.
 */
/*ARGSUSED*/
int
editfile(const char *in_name,
	 int func(FILE *, FILE *, Stat_t *),	/* unused */
	 Stat_t * sb)
{
    LANG *it;
    struct tm tm;
    char my_path[MAXPATHLEN];
    char my_name[MAXPATHLEN];
    char Year[80];
    char *s;
    mode_t mode;
    unsigned changed = 0;
    char *f_bfr;

    (void) func;

    /*
     * Compute a relative pathname for display purposes (simpler to read)
     *
     * Note that on UNIX, 'transtree()' does not give me an absolute
     * pathname.
     */
#ifdef	vms
    (void) vms_relpath(my_path, old_wd, strlcpy(my_path, in_name));
#else /* unix */
    if (getwd(my_path)) {
	(void) relpath(my_path, old_wd, pathcat(my_path, my_path, in_name));
    } else			/* assume directory is not readable */
	(void) strcpy(my_path, in_name);
#endif /* vms/unix */

    TELL("%s ", my_path);
    if (!(f_bfr = readit(in_name, sb)))
	return 0;

    if (T_opt) {
	time_t now = time((time_t *) 0);
	tm = *localtime(&now);
    } else
	tm = *localtime(&sb->ST_MTIME);
    FORMAT(Year, "%s%04d", c_opt ? "(c) " : "", tm.tm_year + 1900);

    /*
     * if no ident, skip this
     */
    if (!f_opt && !has_ident(in_name, f_bfr, f_bfr + strlen(f_bfr))) {
	TELL("(no ident)\n");
	return 0;
    }

    /*
     * Determine the language-type, so we know how to comment the notice
     */
    if (!(it = DecodeLanguage(in_name, f_bfr))) {
	TELL("(unknown)\n");
	return 0;
    }
    if (!maskit(it, f_bfr)) {	/* highlight the comment-blocks */
	TELL("(insert-loc)\n");
	return 0;
    }

    if (!supercede(it, f_bfr, Owner, Disclaim, Year,
		   F_opt, remove_opt,
		   &changed)) {
	TELL("(prior)\n");
	return 0;
    }
    if (strip_opt)
	changed += uncomment(it, f_bfr, in_name);

    unmask(it, f_bfr);		/* restore to normal ascii form */

    /*
     * We probably will write the marked-up file -- if we can find the
     * insertion point.
     */
    if (!(s = insert_at(it, f_bfr))) {
	TELL("(insert-loc 2)");
	return 0;
    }

    changed += WriteIt(my_name, in_name,
		       (int) (tm.tm_year + 1900), it, f_bfr, (int) (s - f_bfr));

    if (!changed) {
	TELL("(unchanged)\n");
	(void) unlink(my_name);
	return 0;
    }

    if (no_op) {
	if (no_op < 2) {
	    char temp[BUFSIZ + (MAXPATHLEN * 2)];
#ifdef	vms
	    int f_got;
	    char my_verb[BUFSIZ];
	    char my_temp[MAXPATHLEN];
	    FILE *tfp;

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
		(void) strcat(my_temp, ".");
	    FORMAT(my_verb, "%s%s", DIFF_TOOL, my_temp);
	    PRINTF("\n%% %s%s %.*s\n", DIFF_TOOL,
		   verbose > 1 ? my_temp : "sys$output",
		   strrchr(my_path, ';') - my_path + 1, my_path);
	    if (verbose > 1)
		shoarg(stdout, my_verb, temp);
	    FFLUSH(stdout);
	    FFLUSH(stderr);
	    (void) execute(my_verb, temp);
	    if (tfp = fopen(my_temp, "r")) {
		while ((f_got = fread(temp,
				      sizeof(char),
				      sizeof(temp), tfp)) > 0)
		      fwrite(temp, sizeof(char), f_got, stdout);
		FCLOSE(tfp);
		(void) unlink(my_temp);
	    }
#else /* unix */
	    *temp = EOS;
	    catarg(temp, my_path);
	    catarg(temp, my_name);
	    PRINTF("\n%% %s %s\n", DIFF_TOOL, my_path);
	    FFLUSH(stdout);
	    FFLUSH(stderr);
	    (void) execute(DIFF_TOOL, temp);
#endif /* vms/unix */
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
	cleanup((char *) 0);
    }
    mode = sb->st_mode & 0777;
    VERBOSE("\n%% chmod %03o %s", mode, in_name);
    if (!no_op) {
	if (chmod(in_name, mode) < 0)
	    failed("chmod");
    }

    TELL("\n");
    return 1;
}

static void
usage(void)
{
    static const char *tbl[] =
    {
	"usage: copyrite [options] files",
	"",
	"options:",
	" -a         disable \"All Rights\" (non-standard)",
	" -c         enable \"(c)\" marker (non-statutory)",
	" -e FILE    redirect standard error to the specified file",
	" -f         (force) markup files w/o RCS or SCCS ident",
	" -F         (force) markup files with unknown notices",
	" -l LANG    specify default language for unknown cases (none)",
	" -L         follow symbolic links",
	" -m FILE    specify owner+disclaimer",
	" -n         (no-op) write result to temp-file, do diffs",
	" -o FILE    redirect standard output to the specified file",
	" -q         (quiet) suppress informational messages",
	" -r         remove notice",
	" -R         recur into directories",
	" -s         strip comments except for notice",
	" -t         touch files with current date",
	" -v         (verbose)",
	" -w NUMBER  set width of notice-comment (default: 80)",
	0};
    int j = 0;
    int length = 8;

    while (tbl[j])
	fprintf(stderr, "%s\n", tbl[j++]);
    fprintf(stderr, "\nlanguages:\n\t");
    for (j = 0; j < (int) SIZEOF(Languages) - 1; j++) {
	if (j > 0 && !strcmp(Languages[j].name, Languages[j - 1].name))
	    continue;
	fprintf(stderr, "%c %s", (j && length > 8) ? ',' : ' ', Languages[j].name);
	length += (int) strlen(Languages[j].name) + 2;
	if (length > 72) {
	    fprintf(stderr, ",\n\t");
	    length = 8;
	}
    }
    fprintf(stderr, "\n");
    exit(FAIL);
    /*NOTREACHED */
}

/*ARGSUSED*/
static int
dummy(FILE *o,
      FILE *i,
      Stat_t * s)
{
    (void) o;
    (void) i;
    (void) s;
    return 0;
}

/*ARGSUSED*/
_MAIN
{
    char m_temp[BUFSIZ];
    int total = 0;
    char *m_opt = 0;
    int j;

    while ((j = getopt(argc, argv, "ace:fFl:Lm:no:qrRstvw:")) != EOF) {
	switch (j) {
	case 'a':
	    a_opt++;
	    break;
	case 'c':
	    c_opt++;
	    break;
	case 'e':
	    if (!freopen(optarg, "a", stderr))
		failed(optarg);
	    break;
	case 'f':
	    f_opt++;
	    break;
	case 'F':
	    F_opt++;
	    break;
	case 'l':
	    l_opt = optarg;
	    break;
	case 'L':
	    L_opt++;
	    break;
	case 'm':
	    m_opt = optarg;
	    break;
	case 'n':
	    no_op++;
	    break;
	case 'o':
	    if (!freopen(optarg, "a", stdout))
		failed(optarg);
	    break;
	case 'q':
	    verbose--;
	    break;
	case 'r':
	    remove_opt++;
	    break;
	case 'R':
	    recur_opt++;
	    break;
	case 's':
	    strip_opt++;
	    break;
	case 't':
	    T_opt++;
	    break;
	case 'v':
	    verbose++;
	    break;
	case 'w':
	    w_opt = atoi(optarg);
	    break;
	default:
	    usage();
	}
    }

    if (!getwd(old_wd))
	failed("getwd: old_wd");

    if (no_op && (verbose < 0))
	verbose = 0;

    if (!m_opt) {
#ifdef	vms
	(void) strcpy(strrchr(strcpy(m_temp, argv[0]), '.'), ".txt");
#else /* unix */
	if (which(m_temp, sizeof(m_temp), argv[0], ".") <= 0)
	    failed("which am I");
#ifdef MSDOS
	{
	    char *s = fleaf(m_temp);
	    if ((s = strchr(s, '.')) != 0)
		*s = EOS;
	}
#endif
	(void) strcat(m_temp, ".txt");
#endif
	m_opt = m_temp;
    }

    LoadTemplate(m_opt);

    if (optind >= argc) {
	if (recur_opt)
	    total = edittree(EDITDIR_ARG, dummy, recur_opt, L_opt);
	else
	    usage();
    }
    for (j = optind; j < argc; j++)
	total = edittree(argv[j], dummy, recur_opt, L_opt);
    VERBOSE("\n** %s %d file%s\n",
	    no_op ? "would change" : "change",
	    total,
	    total != 1 ? "s" : "");
    exit(SUCCESS);
    /*NOTREACHED */
}
