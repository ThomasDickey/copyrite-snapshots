# $Id: Makefile,v 2.0 1991/12/13 14:09:12 ste_cm Rel $
# make-file for testing COPYRITE
#

####### (Development) ##########################################################
GET	= checkout
DATE	= @ echo '**** '`date` >>$@

.SUFFIXES:
.SUFFIXES: .none

####### (Standard Lists) #######################################################
SCRIPTS	=\
	run_tests.sh	run_tests.com

DATAFILES =\
	test.c \
	test.com \
	test.sh \
	test.txt \
	test.y

REF_FILES =\
	unix_c.ref	vms_c.ref \
	unix_w72.ref	vms_w72.ref

SOURCES	= Makefile descrip.mms $(SCRIPTS) $(DATAFILES) $(REF_FILES)

####### (Standard Productions) #################################################
all:				; @echo 'Type "make run_tests"'
clean:				; rm -f *.out
clobber:	clean
destroy:
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done'
sources:	$(SOURCES)
lint.out:
lincnt.out:
run_tests:	$(SOURCES)	; sh -c '$@.sh 2>&1 | tee -a $@.out'

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
