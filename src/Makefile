# $Id: Makefile,v 5.1 1993/09/22 16:47:10 dickey Exp $
# make-file for COPYRITE

THIS	= copyrite
TOP	= ../..
include $(TOP)/portunix/support/portunix.mk

####### (Standard Lists) #######################################################
ALL	= \
	$B/$(THIS) \
	$B/$(THIS).txt

H_SRCS	=\
	$(THIS).h

C_SRCS	=\
	$(THIS).c\
	cleanup.c\
	format.c\
	hasident.c\
	insertat.c\
	maskit.c\
	parse.c\
	readit.c\
	removeit.c\
	superced.c

OBJS	=\
	$(THIS).o\
	cleanup.o\
	format.o\
	hasident.o\
	insertat.o\
	maskit.o\
	parse.o\
	readit.o\
	removeit.o\
	superced.o

SOURCES	= Makefile descrip.mms $(C_SRCS) $(H_SRCS) $(THIS).txt

####### (Standard Productions) #################################################
all\
run_test:	$B $(SOURCES) $(ALL)
clean:				; rm -f $(CLEAN)
clobber:	clean		; rm -f $(ALL)
destroy:			; $(DESTROY)
sources:	$(SOURCES)
lint.out:	$(C_SRCS)	; $(DATE); tdlint $(LINTOPTS) $? >>$@
lincnt.out:	$(C_SRCS)	; $(DATE); lincnt $(C_SRCS) >>$@

####### (Details of Productions) ###############################################
$B:	; mkdir $@

$B/$(THIS):	$(OBJS) $(LIBS)
	$(LINK) -o $@ $(OBJS) $(LIBS)

$B/$(THIS).txt:
	$(COPY) $(@F) $@

$(OBJS):	$(THIS).h $(PORTUNIX_H)

$(SOURCES):			; $(GET) $@
