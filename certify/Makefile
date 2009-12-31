# $Id: Makefile,v 5.1 1995/05/13 23:36:40 tom Exp $

TOP	= ../..
include $(TOP)/td_lib/support/td_lib.mk

####### (Standard Lists) #######################################################
SOURCES	=\
	README\
	config.txt\
	release.txt

####### (Standard Productions) #################################################
all:		$(SOURCES) $(ALL)
clean:				; rm -f $(CLEAN)
clobber:	clean		; rm -f $(ALL)
destroy:			; $(DESTROY)
run_test:
sources:	$(SOURCES)
install:
deinstall:

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
