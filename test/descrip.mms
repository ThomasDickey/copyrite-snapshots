# $Id: descrip.mms,v 5.1 1994/06/23 23:50:27 tom Exp $
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
	unx_c.ref	vms_c.ref \
	unx_w72.ref	vms_w72.ref \
	unx_sw72.ref	vms_sw72.ref

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
