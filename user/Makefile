# $Id: Makefile,v 4.0 1991/12/10 08:13:29 ste_cm Rel $

####### (Development) ##########################################################
THIS	= copyrite
INSTALL_MAN=../../install_man
COPY	= cp -p
GET	= checkout
PUT	= rm -f $@; $(COPY) $? $@

USE	= man
SECTION	= 1
PREFIX	= $(INSTALL_MAN)/$(USE)$(SECTION)/

.SUFFIXES:
.SUFFIXES: .man .cat
#
.man.cat:
	rm -f $@
	nroff -man $< >$@

####### (Standard Lists) #######################################################
ALL	=\
	$(PREFIX)$(THIS).$(SECTION)
SOURCES	=\
	$(THIS).man

####### (Standard Productions) #################################################
all:		sources			; @echo '** use "make install"'
clean:					; rm -f *.cat
clobber:	clean
destroy:	clobber
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done;exit 0'
run_tests:
sources:	$(SOURCES)
install:	$(ALL)
deinstall:				; rm -f $(ALL)

####### (Details of Productions) ###############################################
$(PREFIX)$(THIS).$(SECTION):	$(THIS).$(USE)	; $(PUT)

$(SOURCES):				; $(GET) $@
