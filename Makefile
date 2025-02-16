# FMJ openwatcom Makefile
.OPTIMIZE

CC=wcc386
LN=wlink
AS=tasmx
COPT=-3r -d0 -zq -mf -oacilrt -s -bt=dos -iinc
LOPT=SYS dos4g OP QUIET
AOPT=-m -n -zn -z -q -ml -p -t -jsmart -iinc
OBJ=fmj.obj metal.obj files.obj fmjmenu.obj &
    modload.obj modplay.obj grplib.obj sprite.obj fli.obj
DATAOBJ=\src\MAIN_DAT.OBJ \src\EN_DAT.OBJ

fmj.exe: $(OBJ) 
	*$(LN) N $@ $(LOPT) F { $(OBJ) $(DATAOBJ) }

sprvue.exe: sprvue.obj
	$(LN) N $@ $(LOPT) F $<

.c: \src\;\tools\

.c.obj:
	$(CC) $(COPT) -fo=$@ $<

.asm: \src\

.asm.obj:
	$(AS) $(AOPT) $< $@

clean: .SYMBOLIC
	del *.obj
	del *.err
	del fmj.exe
	del sprvue.exe
