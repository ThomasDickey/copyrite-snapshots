# $Id: Makefile,v 5.3 1995/05/13 23:31:19 tom Exp $
# make-file for COPYRITE

THIS	= copyrite
TOP	= ../..
include $(TOP)/td_lib/support/td_lib.mk

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

lint.out:	$(C_SRCS)	; $(DATE); $(LINT) $(LINTOPTS) $? >>$@

####### (Details of Productions) ###############################################
$B:	; mkdir $@

$B/$(THIS):	$(OBJS) $(LIB_DEPS)
	-$(RM) -f $@
	$(LINK) -o $@ $(OBJS) $(LIB_ARGS)

$B/$(THIS).txt:	$(THIS).txt
	-$(RM) -f $@
	$(COPY) $(@F) $@

$(OBJS):	$(THIS).h $(PORTUNIX_H)

$(SOURCES):			; $(GET) $@
