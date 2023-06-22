cmake_minimum_required( VERSION 3.19 )

set( SRC bmp.c
	bmp.h
	gif.c
	gif.h )

add_library( bitmap ${SRC} )
