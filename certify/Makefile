# $Id: Makefile,v 4.0 1991/12/10 08:07:19 ste_cm Rel $
#

####### (Development) ##########################################################
GET	= checkout

####### (Standard Lists) #######################################################
SOURCES	=\
	README\
	config_sheet.txt\
	release_notes.txt

####### (Standard Productions) #################################################
all:
clean:
clobber:
destroy:
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done'
run_tests:
sources:	$(SOURCES)
install:
deinstall:
lint.out:
lincnt.out:

####### (Details of Productions) ###############################################
$(SOURCES):	; $(GET) $@
