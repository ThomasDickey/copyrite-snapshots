# $Id: Makefile,v 5.0 1993/04/27 14:41:03 ste_cm Rel $
# make-file for testing COPYRITE

TOP	= ../..
include $(TOP)/portunix/support/portunix.mk

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
	unix_c.ref	vms_c.ref \
	unix_w72.ref	vms_w72.ref \
	unix_sw72.ref	vms_sw72.ref

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
