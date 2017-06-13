# MINGW32 Makefile for swmm5

# This will build swmm as DLL
# (swmm5.dll) under MINGW32 GCC, and a standalone

# The following targets are defined:
# make make-dll
#     -Builds swmm5.dll
# make install
#     -Copy swmm5.dll to $(prefix)
# make clean
#     -Removes object and library files, returning the build directory
#      to its pristine state.

# You may wish to change the install path 'prefix',
# or the compiler flags, but these defaults should be fine.


# Target filenames
swmmrootname := swmm5
libname := $(swmmrootname).dll
#exename := $(swmmrootname).exe

# Location of SWMM toolkit includes
swmmsrcdir = ../../src
VPATH = $(swmmsrcdir)


# Install directories
prefix = "C:\\PROJECTCODE\\Stormwater-Management-Model\\target"
execdir = $(prefix)
libdir = $(prefix)

# Compiler and flags
CC = gcc
CFLAGS = -g -O3 
LDFLAGS = -L . -Wl,--kill-at,--enable-stdcall-fixup,-rpath,$(libdir)                                                            

# Files for the shared object library
swmm_objs=swmm5.o climate.o controls.o culvert.o datetime.o dynwave.o dwflow.o error.o exfil.o findroot.o \
       flowrout.o forcmain.o gage.o gwater.o hash.o hotstart.o iface.o infil.o inflow.o \
       input.o toolkitAPI.o inputrpt.o keywords.o kinwave.o landuse.o lid.o lidproc.o link.o massbal.o \
       mathexpr.o mempool.o node.o odesolve.o output.o project.o qualrout.o \
       rain.o rdii.o report.o roadway.o routing.o runoff.o shape.o snow.o stats.o \
       statsrpt.o subcatch.o surfqual.o table.o toposort.o transect.o treatmnt.o xsect.o

	   # swmm header files
swmm_heads=consts.h datetime.h enums.h error.h exfil.h findroot.h \
       funcs.h globals.h hash.h headers.h infil.h toolkitAPI.h keywords.h \
       lid.h macros.h mathexpr.h mempool.h objects.h odesolve.h \
       text.h swmm5.h

#.PHONY: all$(exename)
make-dll: $(libname) 

$(libname): $(swmm_objs)
	$(CC) $(CFLAGS) $(LDFLAGS) -D DLL -fopenmp -lm -lpthread -shared -o $@ $^ -Wl,--output-def,$(swmmrootname).def

#make-exe: $(exename) 
#$(exename):
#	$(CC) $(CFLAGS) -D CLE -fopenmp -lm -lpthread -o $@ $^ -l$(LDFLAGS)

clean:
	del $(swmm_main).o $(swmm_objs) $(exename) $(libname) $(swmmrootname).def
	
install:
	COPY $(libname) $(libdir)
	COPY $(libname) "C:\\PROJECTCODE\\pyswmm\\pyswmm\\swmmLinkedLibs\\Windows"




