# $Id: Makefile,v 5.0 1993/04/27 14:41:03 ste_cm Rel $

THIS	= copyrite
TOP	= ../..
include $(TOP)/portunix/support/portunix.mk

####### (Development) ##########################################################
INSTALL_MAN=$(TOP)/install_man

USE	= man
SECTION	= 1
PREFIX	= $(INSTALL_MAN)/$(USE)$(SECTION)/

####### (Standard Lists) #######################################################
IT	=\
	$(PREFIX)$(THIS).$(SECTION)
SOURCES	=\
	$(THIS).man

####### (Standard Productions) #################################################
all:		$(SOURCES)		; @echo '** use "make install"'
clean:					; rm -f $(CLEAN)
clobber:	clean			; rm -f $(ALL)
destroy:				; $(DESTROY)
run_test:
sources:	$(SOURCES)
install:	$(IT)
deinstall:				; rm -f $(IT)

####### (Details of Productions) ###############################################
$(PREFIX)$(THIS).$(SECTION):	$(THIS).$(USE)	; $(PUT)

$(SOURCES):				; $(GET) $@
