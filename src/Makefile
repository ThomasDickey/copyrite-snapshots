# $Id: Makefile,v 1.1 1991/12/10 08:40:32 dickey Exp $
# make-file for COPYRITE

####### (Development) ##########################################################
MAKE	= make $(MFLAGS) -k$(MAKEFLAGS)	GET=$(GET)
GET	= checkout
TOP	= ../../

B	= ../bin
I	= $(TOP)portunix/interface
J	= $(TOP)cm_tools/interface

THIS	= copyrite

LIBS	=\
	$(TOP)portunix/lib/portunix.a\
	$(TOP)cm_tools/lib/lib.a

DATE	= @ echo '**** '`date` >>$@

INCLUDES= -I$I -I$J
CPP_OPTS= $(DEFINES) $(INCLUDES)
.c.o:	; $(CC) -c $(CFLAGS) $(CPP_OPTS) $<

####### (Standard Lists) #######################################################
ALL	= $B/$(THIS)

C_SRCS	=\
	$(THIS).c

OBJS	=\
	$(THIS).o

SOURCES	= Makefile descrip.mms $(C_SRCS)

####### (Standard Productions) #################################################
all:		$B $(ALL) $(SOURCES)
clean:				; rm -f *.out *.o
clobber:	clean		; rm -f $(ALL)
destroy:	clobber
	sh -c 'for i in *;do case $$i in RCS);; *) rm -f $$i;;esac;done'

run_tests:
sources:	$(SOURCES)
lint.out:	$(C_SRCS)
	$(DATE)
	tdlint -ltd -lportunix $(INCLUDES) $? >>$@
lincnt.out:	$(C_SRCS)
	$(DATE)
	lincnt $(C_SRCS) >>$@

####### (Details of Productions) ###############################################
$B:	; mkdir $@

$B/$(THIS):	$(OBJS) $(LIBS)
	$(CC) -o $@ $(OBJS) $(LIBS)

$(THIS).o:	$I/portunix.h

$(SOURCES):			; $(GET) $@
