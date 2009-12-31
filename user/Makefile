# $Id: Makefile,v 5.1 1995/05/13 23:37:01 tom Exp $

THIS	= copyrite
TOP	= ../..
include $(TOP)/td_lib/support/td_lib.mk

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
