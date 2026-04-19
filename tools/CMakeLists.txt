cmake_minimum_required(VERSION 3.24...4.3)

#set(CMAKE_VERBOSE_MAKEFILE 1)

project(FMJ-Tools C)

set(CMAKE_C_STANDARD 99)

find_package(SDL3)

add_compile_options(-Wall -DWITH_SDL)

add_executable(sprvue sprvue.c)
target_include_directories(sprvue PRIVATE ../inc)
target_link_libraries(sprvue PRIVATE SDL3)

add_executable(dump_dat
	dump_dat.c
	en_dat.o
	main_dat.o)
target_include_directories(dump_dat PRIVATE ../inc)
