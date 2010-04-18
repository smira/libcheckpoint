lib_LIBRARIES			= libcheckpoint.a libfcheckpoint.a

SOURCE_FILES			= checkpoint.c image.c debug.c setup.c areas.c \
				  restore.c files.c elf.c system.c vital.c benchmark.c \
				  image.h debug.h setup.h restore.h areas.h \
				  files.h elf_.h system.h vital.h benchmark.h \
				  minilzo/minilzo.c minilzo/minilzo.h minilzo/lzoconf.h \
				  minilzo/lzodefs.h

libcheckpoint_a_SOURCES		= $(SOURCE_FILES) cwrap.c

EXTRA_libcheckpoint_a_SOURCES  	= mpich/mpich_shm.c mpich/mpich.h mpich/mpich_gm_sync.c \
				  mpich/mpich_gm_sync.h mpich/mpich_gm.c mpich/mpich_gm.h \
				  mpich/mpich_gm_exclude.c mpich/mpich_gm_exclude.h \
				  mpich/mpich_p4.c mpich/mpich_p4_sync.c mpich/mpich_p4.h \
				  mpich/mpich_p4_sync.h

libcheckpoint_a_LIBADD		= @MPICH_EXTRA_OBJECTS@
libcheckpoint_a_DEPENDENCIES	= @MPICH_EXTRA_OBJECTS@

libfcheckpoint_a_SOURCES	= $(SOURCE_FILES) fortranwrap.c

EXTRA_libfcheckpoint_a_SOURCES  = $(EXTRA_libcheckpoint_a_SOURCES)

libfcheckpoint_a_LIBADD		= $(libcheckpoint_a_LIBADD)
libfcheckpoint_a_DEPENDENCIES	= $(libcheckpoint_a_DEPENDENCIES)



include_HEADERS			= checkpoint.h

AM_CPPFLAGS 			= -DETCDIR="\"$(sysconfdir)\"" -DMINILZO_HAVE_CONFIG_H $(MPICH_CPPFLAGS)
INCLUDES			= @GM_INCLUDE@ @P4_INCLUDE@ -Impich/ -I./

EXTRA_DIST			= mpich/gm_arch_io.h minilzo/COPYING minilzo/README.LZO

