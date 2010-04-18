bin_PROGRAMS		= ckptdump 

WRAP_SYMBOLS		= @WRAP_FUNCS@

ckptdump_SOURCES 	= ckptdump.c ../src/image.c ../src/image.h ../src/system.c ../src/system.h \
			  ../src/areas.c ../src/areas.h ../src/debug.c ../src/debug.h ../src/minilzo/minilzo.c
ckptdump_LDFLAGS	= -Wl,-static $(WRAP_SYMBOLS) 


