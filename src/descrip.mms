# $Id: descrip.mms,v 5.0 1993/05/06 11:33:42 ste_cm Rel $
# MMS script for Copy-Right program.

PROGRAM	= COPYRITE

OBJ_ARGS =, -
	[]cleanup.obj, -
	format.obj, -
	hasident.obj, -
	insertat.obj, -
	maskit.obj, -
	parse.obj, -
	readit.obj, -
	removeit.obj, -
	superced.obj

.INCLUDE PORTUNIX_ROOT:[SUPPORT]PROGRAM_RULES

ALL ::	[-.BIN]$(PROGRAM).TXT	; @ write sys$output "** made $?"
CLOBBER ::			; - remove -f [-.BIN]$(PROGRAM).TXT

[-.BIN]$(PROGRAM).OBJ : $(I)PORTUNIX.H
[-.BIN]$(PROGRAM).TXT : $(PROGRAM).TXT	; copy $(PROGRAM).TXT $@;

INSTALL ::	$(B)$(PROGRAM).TXT	; @ write sys$output "** made $?"
DEINSTALL ::				; - remove -f $(B)$(PROGRAM.TXT)

$(B)$(PROGRAM).TXT : [-.BIN]$(PROGRAM).TXT ; copy $? $@;
