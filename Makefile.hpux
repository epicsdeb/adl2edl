
####### Compiler, tools and options

CXX	=	aCC
CXXFLAGS=	-AA -D_REENTRANT -O +Onolimit -g -DHPUX
LINK	=	aCC -Wl,-a,archive
LFLAGS	=	-AA -O -s
LIBS	=	-L/usr/lib/X11R6 -lm -lXm -lXp -lXt -lXext -lX11 -lSM -lICE -l:libc.sl


####### Files

HEADERS =	fonts.h translator.h
SOURCES =	fonts.cc translator.cc graphic.cc control.cc monitor.cc
OBJECTS =	fonts.o translator.o graphic.o control.o monitor.o
TARGET	=	adl2edl


####### Implicit rules

.SUFFIXES: .cc 

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o $@ $<

####### Build rules


all: $(TARGET)

$(TARGET): $(OBJECTS) 
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(LIBS)

fonts.o: fonts.cc
translator.o:	translator.cc
graphic.o:	graphic.cc
control.o:	control.cc
monitor.o:	monitor.cc


