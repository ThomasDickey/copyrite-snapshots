# $Id: Makefile,v 3.0 1991/12/13 14:52:53 ste_cm Rel $
# Top-level make-file for COPYRITE
#

####### (Development) ##########################################################
INSTALL_BIN = ../install_bin
INSTALL_MAN = ../install_man
COPY	= cp -p
MAKE	= make $(MFLAGS) -k$(MAKEFLAGS) CFLAGS="$(CFLAGS)" COPY="$(COPY)"
THIS	= copyrite

####### (Standard Lists) #######################################################
SOURCES	=\
	Makefile\
	descrip.mms\
	COPYRIGHT\
	README

MFILES	=\
	certificate/Makefile\
	src/Makefile\
	test/Makefile\
	user/Makefile

FIRST	=\
	bin\
	$(SOURCES)\
	$(MFILES)

####### (Standard Productions) #################################################
all::		bin $(SOURCES)
sources::	$(SOURCES)

all\
clean\
clobber\
destroy\
run_tests\
sources::	$(MFILES)
	cd certificate;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd test;	$(MAKE) $@
	cd user;	$(MAKE) $@

lint.out\
lincnt.out:	$(MFILES)
	cd src;		$(MAKE) $@

clean\
clobber::
	rm -f *.bak *.log *.out core
clobber\
destroy::
	rm -rf bin
destroy::
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done;exit 0'

IT	= \
	$(INSTALL_BIN)/$(THIS) \
	$(INSTALL_BIN)/$(THIS).txt

install::	all
install::	$(IT)
deinstall::			; rm -f $(IT)

install\
deinstall::
	cd user;	$(MAKE) $@ INSTALL_MAN=`cd ..;cd $(INSTALL_MAN);pwd`

####### (Details of Productions) ###############################################
.first:		$(FIRST)

$(SOURCES)\
$(MFILES):			; checkout -x $@

bin:				; mkdir $@
bin/$(THIS):	all

$(INSTALL_BIN)/$(THIS):		bin/$(THIS)	; $(COPY) bin/$(@F) $@
$(INSTALL_BIN)/$(THIS).txt:	bin/$(THIS).txt	; $(COPY) bin/$(@F) $@
