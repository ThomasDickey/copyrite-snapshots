$! $Id: run_test.com,v 2.0 1991/12/13 12:37:07 ste_cm Rel $
$! Run a test to show that COPYRITE is working
$	VERIFY = F$VERIFY(0)
$	set := set
$	set symbol/scope=(noglobal)
$
$	prog := 'F$ENVIRONMENT("DEFAULT")
$	prog := "$''F$EXTRACT(0,F$LENGTH(prog)-1,prog)'.-.bin]copyrite"
$
$	call clean
$	n = 0
$ LOOP:
$	OPT = F$ELEMENT(N, ",", "c,w72")
$	if OPT .eqs. ","
$	then
$		VERIFY = F$VERIFY(VERIFY)
$		exit
$	endif
$
$	REF := "vms_''OPT'.ref"
$	TST := "vms_''OPT'.tst"
$	TMP := "vms_''OPT'.tmp"
$
$	write sys$error "** test:''REF'"
$	PROG -n'OPT -o'TST test.*;
$	if .not. $status
$	then
$		VERIFY = F$VERIFY(VERIFY)
$		exit
$	endif
$
$	if F$SEARCH("run_tests.tmp") .nes. ""
$	then
$		append run_tests.tmp;* 'tst
$		call remove run_tests.tmp;*
$	endif
$
$	if F$SEARCH("''REF'") .eqs. ""
$	then
$		write sys$output "** save ''REF'"
$		rename 'TST; 'REF;
$	else
$		diff/out='TMP 'TST 'REF
$		if $severity .ne. 1
$		then
$			write sys$output "?? diff ''REF' vs ''TST'"
$			type 'TMP
$		else
$			write sys$output "** ok   ''REF'"
$			call remove 'TST;*
$		endif
$		call remove 'TMP;*
$	endif
$	N = N + 1
$	goto LOOP
$	VERIFY = F$VERIFY(VERIFY)
$	exit
$
$ clean: subroutine
$	call remove *.tst;*,*.tmp;*
$	return
$ endsubroutine
$
$ remove: subroutine
$	set noon
$	old_msg = f$environment("message")
$	set message/noident/notext/nosever/nofacil
$	set file/protection=(o:rwed) *.tst;* *.tmp;*
$	if p1 .nes. "" then delete 'p1
$	if p2 .nes. "" then delete 'p2
$	if p3 .nes. "" then delete 'p3
$	if p4 .nes. "" then delete 'p4
$	if p5 .nes. "" then delete 'p5
$	if p6 .nes. "" then delete 'p6
$	if p7 .nes. "" then delete 'p7
$	if p8 .nes. "" then delete 'p8
$	set message 'old_msg
$	set on
$	return
$ endsubroutine
