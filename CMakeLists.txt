cmake_minimum_required(VERSION 3.18)

# set system and tools
set(CMAKE_SYSTEM_NAME DOS)
set(CMAKE_SYSTEM_PROCESSOR x86)
set(CMAKE_ASM_MASM_COMPILER uasm)

#set(CMAKE_VERBOSE_MAKEFILE 1)

# actual project
project(FMJ C ASM_MASM)

add_executable(FMJ
	src/files.c
	src/fli.c
	src/fmj.c
	src/fmjmenu.c
	src/grplib.asm
	src/metal.asm
	src/modload.c
	src/sprite.asm)
target_include_directories(FMJ PRIVATE inc)
target_compile_options(FMJ PRIVATE
	# c99, flat memory model, optimizations
	$<$<COMPILE_LANGUAGE:C>:-za99 -mf -oacilrt>
	# 386, quiet, case sensitive, disable internal macros
	$<$<COMPILE_LANGUAGE:ASM_MASM>:-3 -nologo -Cp -nomlib>)
# external objects
target_sources(FMJ PRIVATE src/en_dat.obj src/main_dat.obj src/modplay.obj)

add_executable(sprvue
	tools/sprvue.c)
target_include_directories(sprvue PRIVATE inc)
