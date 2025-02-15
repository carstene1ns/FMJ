CC=wcc386
LN=wlink
AS=tasmx
COPT=-3r -d0 -zq -mf -oacilrt -s -bt=dos
LOPT=SYS dos4g OP QUIET
AOPT=-m -n -zn -z -q -ml -p -t -jsmart
OBJ=fmj.obj metal.obj files.obj fmjmenu.obj modload.obj modplay.obj grplib.obj sprite.obj fli.obj rotate.obj
DATAOBJ=MAIN_DAT.OBJ EN_DAT.OBJ

fmj.exe: $(OBJ) 
	*$(LN) N $@ $(LOPT) F { $(OBJ) $(DATAOBJ) }

cdinfo.exe: cdinfo.obj
	$(LN) N $@ $(LOPT) F $<

sprvue.exe: sprvue.obj
	$(LN) N $@ $(LOPT) F $<

.c.obj:
	$(CC) $(COPT) -fo=$@ $<

.asm.obj:
	$(AS) $(AOPT) $< $@

clean: .SYMBOLIC
	del cdinfo.exe
	del cdinfo.obj
	del sprvue.exe
	del sprvue.obj
	del fmj.exe
	del fmj.obj
	del metal.obj
	del files.obj
	del fmjmenu.obj
	del modload.obj
	del modplay.obj
	del grplib.obj
	del sprite.obj
	del fli.obj
	del rotate.obj
	del *.err
