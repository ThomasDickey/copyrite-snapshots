--$Id: test.ada,v 5.0 1992/01/06 11:29:55 ste_cm Rel $

with    TO_DMS_C;
with    SYSTEM;

package body DMS_DUMP is

    procedure DUMP_ATTR_TITLE is

	procedure C_DUMP_ATTR_TITLE;

--$if verdix	-- vms is the next case
	pragma INTERFACE (C, C_DUMP_ATTR_TITLE, "dump_attr_title");
--$elif vms
--$$	pragma INTERFACE (C, C_DUMP_ATTR_TITLE);
--$$	pragma IMPORT_PROCEDURE(
--$$		INTERNAL        => C_DUMP_ATTR_TITLE,
--$$		EXTERNAL        => "DUMP_ATTR_TITLE"
--$$	    );
--$if	verdix
	this is always commented out!
--$endif
--$endif

    begin

	C_DUMP_ATTR_TITLE;
--$if ! vms
	text_io.put_line("done");
	--$endif
    end DUMP_ATTR_TITLE;


end DMS_DUMP;
