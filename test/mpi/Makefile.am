CC 			= mpicc
CXX 			= mpiCC
F77 			= mpif77

WRAP_SYMBOLS		= @WRAP_FUNCS@

CLEANFILES		= *.ckpt.[0-9] *.[0-9].log

noinst_PROGRAMS 	= clica clen3 md_systolic matvec

clica_SOURCES 		= clica.c
clica_LDADD		= -L../../src -L../../automatic/online -lcheckpoint -lckptmpionline @MPICH_LDFLAGS@
clica_LDFLAGS		= -static-libgcc -Wl,-static $(WRAP_SYMBOLS) 

clen3_SOURCES 		= clen3.f
clen3_FFLAGS 		= -malign-double -fforce-mem -fforce-addr -fstrength-reduce -funroll-loops
clen3_LDADD		= -L../../src -L../../automatic/online -lfcheckpoint -lckptmpionline @MPICH_LDFLAGS@
clen3_LDFLAGS		= -static-libgcc -Wl,-static $(WRAP_SYMBOLS) 

md_systolic_SOURCES 	= md_systolic.cxx
md_systolic_LDADD	= -L../../src -L../../automatic/online -lcheckpoint -lckptmpionline @MPICH_LDFLAGS@
md_systolic_LDFLAGS	= -static-libgcc -Wl,-static $(WRAP_SYMBOLS) 

matvec_SOURCES 		= matvec.c
matvec_LDADD		= -lm -L../../src -L../../automatic/online -lcheckpoint -lckptmpionline @MPICH_LDFLAGS@
matvec_LDFLAGS		= -static-libgcc -Wl,-static $(WRAP_SYMBOLS) 
