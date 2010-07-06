#!/bin/sh
# $Id: run_test.sh,v 5.2 2010/07/05 09:25:15 tom Exp $
# Run a test to show that COPYRITE is working
PROG=`cd ../bin;pwd`/copyrite

LANG=C; export LANG
LC_ALL=C; export LC_ALL
LC_TIME=C; export LC_TIME
LC_CTYPE=C; export LC_CTYPE
LANGUAGE=C; export LANGUAGE
LC_COLLATE=C; export LC_COLLATE
LC_NUMERIC=C; export LC_NUMERIC
LC_MESSAGES=C; export LC_MESSAGES

echo '** '`date`
for opt in c w72 sw72
do
	TST=unx_$opt.tst
	REF=unx_$opt.ref
	rm -f $TST
	trap "rm -f $TST" 0
	for i in test*.*
	do
		$PROG -n$opt $i >>$TST
	done
	if test -f $REF
	then
		if cmp -s $TST $REF
		then
			echo '** ok   '$REF
			rm -f $TST
		else
			echo '?? diff '$REF' vs '$TST
			trap 0
		fi
	else
		echo '** save '$REF
		mv $TST $REF
	fi
done
