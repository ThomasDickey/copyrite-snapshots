# $Id: descrip.mms,v 2.0 1991/12/13 14:09:16 ste_cm Rel $
# VMS make-file for testing COPYRITE

####### (Development) ##########################################################
DONE	= @ write sys$output "** made $@"

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
all :		$(SOURCES)	; $(DONE)
clean :
	- remove -fv *.out;* *.out-;* *.log;*
clobber :	clean		; $(DONE)
destroy :
	- remove -fv *.*;*
sources :	$(SOURCES)	; $(DONE)
run_tests :	$(SOURCES)	; @$@

####### (Details of Productions) ###############################################
$(SOURCES) :
	@ write sys$output "need file: $@"
