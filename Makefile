# $Id: Makefile,v 5.1 1995/05/13 23:37:02 tom Exp $
# Top-level make-file for COPYRITE

THIS	= copyrite
TOP	= ..

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
