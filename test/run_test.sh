#!/bin/sh
# $Id: run_test.sh,v 5.1 1994/06/23 23:52:21 tom Exp $
# Run a test to show that COPYRITE is working
PROG=`cd ../bin;pwd`/copyrite
#
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
