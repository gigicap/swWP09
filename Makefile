SRCDIR  =       ./src
LIBDIR  =       ./lib
BINDIR  =       ./bin
MAINDIR =       ./main


PACKAGES = gtk+-2.0
PKG_CFLAGS = $(shell pkg-config --cflags $(PACKAGES))
PKG_INCLUDES = $(shell pkg-config --libs $(PACKAGES))

#CFLAGS	=  `pkg-config --cflags --libs gtk+-2.0`

#CC	=	gcc $(CFLAGS)
CC	=	gcc $(PKG_CFLAGS)

COPTS	=	-fPIC -DUNIX -DLINUX -O2

#C++     =       g++ -std=gnu++11 -g `root-config --cflags --libs`
C++     =       g++  -g `root-config --cflags --libs`

PKG_LDEP = $(shell pkg-config --libs-only-l $(PACKAGES))
DEPLIBS	=	-lCAENDigitizer -lCAENDPPLib -lCAENVME -lcaenhvwrapper -lrt $(PKG_LDEP)

#LIBS	=	-L..

INCLUDEDIR =	-I./include


LIBOBJS   = $(patsubst %.c, %.o, $(wildcard $(SRCDIR)/*.c))
LIBNAME   = $(LIBDIR)/libCaenico.so


INCLUDES =	$(wildcard include/*.h)

CPROGRAMS = RunControl Consumer Producer Plotter HPGeCoolerMonitor SlowControl HPGeMonitor GCALMonitor BaFMonitor
 
CPPPROGRAMS = tupleMakerCSPEC tupleMakerGCAL  SiStripPedestal

#########################################################################

all	:	$(CPROGRAMS) $(CPPPROGRAMS)

$(LIBOBJS):	$(INCLUDES) Makefile
$(LIBNAME) : 	$(LIBOBJS)
		ld -G -o $(LIBNAME) $(DEPLIBS) $(LIBOBJS)



$(CPROGRAMS) : 	$(LIBNAME) 
		$(CC)  -o $(BINDIR)/$@ $(COPTS) $(INCLUDEDIR) $(PKG_INCLUDES) $(LIBNAME) $(DEPLIBS) $(MAINDIR)/$@.c


$(CPPPROGRAMS):  
		$(C++)  -o $(BINDIR)/$@ $(MAINDIR)/$@.C	


clean	:
		/bin/rm -f $(LIBNAME) $(LIBOBJS) $(PROGRAMS)

%.o	:	%.c
		$(CC) $(COPTS) $(INCLUDEDIR) -c -o $@ $<