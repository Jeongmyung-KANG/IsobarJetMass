CXX      := g++
CXXFLAGS := -fPIC
LD       := g++
LDFLAGS  := -shared

ROOTCFLAGS := $(shell root-config --cflags)
ROOTLIBS   := $(shell root-config --libs)

TARGET := libTools.so

SRCS := TrackLevelJetArray.cxx 
OBJS := $(SRCS:.cxx=.o)

DICT := ToolsDict.cxx
DICTOBJ := ToolsDict.o

HDRS := TrackLevelJetArray.h LinkDef.h

all: $(TARGET)

$(DICT): $(HDRS)
	rootcint -f $@ -c $(HDRS)

$(TARGET): $(OBJS) $(DICTOBJ)
	$(LD) $(LDFLAGS) -o $@ $^ $(ROOTLIBS)

%.o: %.cxx
	$(CXX) $(CXXFLAGS) $(ROOTCFLAGS) -c $< -o $@

clean:
	rm -f *.o *.so *Dict.*

.PHONY: all clean
