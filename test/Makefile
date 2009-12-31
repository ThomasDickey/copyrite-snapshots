# $Id: Makefile,v 5.2 1995/05/13 23:32:38 tom Exp $
# make-file for testing COPYRITE

TOP	= ../..
include $(TOP)/td_lib/support/td_lib.mk

####### (Development) ##########################################################

.SUFFIXES:
.SUFFIXES: .none

####### (Standard Lists) #######################################################
SCRIPTS	=\
	run_test.sh	run_test.com

DATAFILES =\
	test0.c test1.c test2.c test3.c test4.c \
	test.ada \
	test.c \
	test.com \
	test.ftn \
	test.sh \
	test.txt \
	test.y

REF_FILES =\
	unx_c.ref	vms_c.ref \
	unx_w72.ref	vms_w72.ref \
	unx_sw72.ref	vms_sw72.ref

SOURCES	= Makefile descrip.mms $(SCRIPTS) $(DATAFILES) $(REF_FILES)

####### (Standard Productions) #################################################
all:		$(SOURCES)	; @echo 'Type "make run_test"'
clean:				; rm -f $(CLEAN)
clobber:	clean
destroy:			; $(DESTROY)
sources:	$(SOURCES)
lint.out:
lincnt.out:
run_test:	$(SOURCES)	; $(RUN_TESTS)

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
