#CXX = g++
CXX = $(shell root-config --cxx)
ROOTFLAGS = $(shell root-config --cflags)
FASTJET_FLAGS = $(shell fastjet-config --cxxflags)
FASTJET_LIBS = $(shell fastjet-config --libs)

CFLAGS = ${ROOTFLAGS} -g -I tools ${FASTJET_FLAGS}
ROOTLIBS = $(shell root-config --libs) -lEG


all: test.bin

test.bin: test.o 
	${CXX} ${CFLAGS} $^ -o$@ ${ROOTLIBS} ${FASTJET_LIBS}

test.o: JetFindingPrac.cxx
	${CXX} -fPIC  -c ${CFLAGS} $< -o $@


clean:
	rm -vf *.so *.o *.d test.bin
