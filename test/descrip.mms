# $Id: descrip.mms,v 5.0 1993/05/06 10:58:23 ste_cm Rel $
# VMS make-file for testing COPYRITE

####### (Development) ##########################################################
DONE	= @ write sys$output "** made $@"

####### (Standard Lists) #######################################################
SCRIPTS	=\
	run_test.sh	run_test.com

DATAFILES =\
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
all :		$(SOURCES)	; $(DONE)
clean :
	- remove -fv *.out;* *.out-;* *.log;*
clobber :	clean		; $(DONE)
destroy :
	- remove -fv *.*;*
sources :	$(SOURCES)	; $(DONE)
run_test :	$(SOURCES)	; @$@

####### (Details of Productions) ###############################################
$(SOURCES) :
	@ write sys$output "need file: $@"
