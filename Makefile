# $Id: Makefile,v 5.0 1993/05/05 08:30:16 ste_cm Rel $
# Top-level make-file for COPYRITE

THIS	= copyrite
TOP	= ..
include $(TOP)/portunix/support/portunix.mk

####### (Standard Lists) #######################################################
SOURCES	=\
	Makefile\
	descrip.mms\
	COPYING\
	README

MFILES	=\
	certify/Makefile\
	src/Makefile\
	test/Makefile\
	user/Makefile

DIRS	=\
	bin

####### (Standard Productions) #################################################
all::		$(DIRS) $(SOURCES)
sources::	$(SOURCES)

all\
clean\
clobber\
destroy\
run_test\
sources::	$(MFILES)
	cd certify;	$(MAKE) $@
	cd src;		$(MAKE) $@
	cd test;	$(MAKE) $@
	cd user;	$(MAKE) $@

lint.out\
lincnt.out:	$(MFILES)
	cd src;		$(MAKE) $@

clean\
clobber::			; rm -f $(CLEAN)
clobber\
destroy::			; rm -rf $(DIRS)
destroy::			; $(DESTROY)

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
$(SOURCES)\
$(MFILES):			; $(GET) $@

$(DIRS):			; mkdir $@
bin/$(THIS):	all

$(INSTALL_BIN)/$(THIS):		bin/$(THIS)	; $(PUT)
$(INSTALL_BIN)/$(THIS).txt:	bin/$(THIS).txt	; $(PUT)
