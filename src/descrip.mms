# $Id: descrip.mms,v 1.1 1991/12/10 08:09:14 dickey Exp $
# MMS script for Copy-Right program.
#
PROGRAM	= COPYRITE
.INCLUDE PORTUNIX_ROOT:[SUPPORT]PROGRAM_RULES
[-.BIN]$(PROGRAM).OBJ : $(I)PORTUNIX.H
