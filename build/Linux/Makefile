# Linux Makefile for swmm5

# This will build swmm as an executable binary file

# The following targets are defined:
# make swmm5
#     -Builds swmm5 (executable binary)
# make clean
#     -Removes object files
# make install
#     -Copy swmm5 to $(prefix)


#name of the binary executable to be created
libname = swmm5

# Location of SWMM toolkit includes
swmmsrcdir = ../../src
VPATH = $(swmmsrcdir)

# Install directories
prefix = "/usr/bin/swmm5"
execdir = $(prefix)
libdir = $(prefix)

swmm_objs=swmm5.o climate.o controls.o culvert.o datetime.o dynwave.o dwflow.o error.o exfil.o findroot.o \
       flowrout.o forcmain.o gage.o gwater.o hash.o hotstart.o iface.o infil.o inflow.o \
       input.o toolkitAPI.o inputrpt.o keywords.o kinwave.o landuse.o lid.o lidproc.o link.o massbal.o \
       mathexpr.o mempool.o node.o odesolve.o output.o project.o qualrout.o \
       rain.o rdii.o report.o roadway.o routing.o runoff.o shape.o snow.o stats.o \
       statsrpt.o subcatch.o surfqual.o table.o toposort.o transect.o treatmnt.o xsect.o

swmm5: $(swmm_objs)
		gcc $(swmm_objs) -fopenmp -lm -lpthread -o $(libname)

clean:
		rm *.o

install:
		COPY $(libname) $(libdir)
