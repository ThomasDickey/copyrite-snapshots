# $Id: descrip.mms,v 2.0 1991/12/13 08:33:15 ste_cm Rel $
# MMS script for Copy-Right program.
#
PROGRAM	= COPYRITE
.INCLUDE PORTUNIX_ROOT:[SUPPORT]PROGRAM_RULES

ALL ::	[-.BIN]$(PROGRAM).TXT	; @ write sys$output "** made $?"
CLOBBER ::			; - remove -f [-.BIN]$(PROGRAM).TXT

[-.BIN]$(PROGRAM).OBJ : $(I)PORTUNIX.H
[-.BIN]$(PROGRAM).TXT : $(PROGRAM).TXT	; copy $(PROGRAM).TXT $@;
