# $Id: Makefile,v 5.0 1993/04/27 14:46:24 ste_cm Rel $

TOP	= ../..
include $(TOP)/portunix/support/portunix.mk

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
lint.out:
lincnt.out:

####### (Details of Productions) ###############################################
$(SOURCES):			; $(GET) $@
