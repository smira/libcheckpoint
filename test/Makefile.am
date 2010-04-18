if MPI_BUILD
SUBDIRS 		= mpi
else
noinst_PROGRAMS		= test test2 mult test_areas
endif

WRAP_SYMBOLS		= @WRAP_FUNCS@

test_SOURCES 		= test.c
test_LDADD		= -L../src -lcheckpoint @MPICH_LDFLAGS@
test_LDFLAGS		= -Wl,-static $(WRAP_SYMBOLS) 

test_areas_SOURCES	= test_areas.c ../src/areas.c
test_areas_LDADD	= -L../src -lcheckpoint @MPICH_LDFLAGS@
test_areas_LDFLAGS	= -Wl,-static $(WRAP_SYMBOLS)

mult_SOURCES 		= mult.c
mult_LDADD		= -L../src -lcheckpoint @MPICH_LDFLAGS@
mult_LDFLAGS		= -Wl,-static $(WRAP_SYMBOLS)

test2_SOURCES 		= test2.c
test2_LDADD		= -L../src -lcheckpoint @MPICH_LDFLAGS@
test2_LDFLAGS		= -Wl,-static $(WRAP_SYMBOLS)

CLEANFILES		= *.ckpt.[0-9]

EXTRA_DIST		= bench_driver.php
