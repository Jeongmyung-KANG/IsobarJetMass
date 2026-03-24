
#CXX = g++
CXX = $(shell root-config --cxx)
ROOTFLAGS = $(shell root-config --cflags)

CFLAGS = ${ROOTFLAGS} -g 

ROOTLIBS = $(shell root-config --libs) -lEG


all: test.bin

test.bin: JetFindingPrac.o
	${CXX} ${CFLAGS} $^ -o$@ ${ROOTLIBS} `fastjet-config --cxxflags --libs --plugins`

JetFindingPrac.o: JetFindingPrac.cxx
	${CXX} -fPIC  -c ${CFLAGS} $< -o $@

clean:
	rm -vf *.so *.o *.d mixingTest.bin dict.C

#g++ -o mixingTest.bin mixingTest.cxx StRoot/MyAnalysis/ITrack.cxx StRoot/MyAnalysis/JEvent.cxx StRoot/MyAnalysis/TrackLevelContainor.cxx -I/usr/local/root/include -I/StRoot/MyAnalysis -L/usr/local/root/lib -lCore -lRIO -lTree -lMathCore -lPhysics -lHist
#	echo "Starting to compile."
#g++ -c StRoot/MyAnalysis/ITrack.cxx  -I/usr/local/root/include -I/StRoot/MyAnalysis `root-config --libs`
#echo "Finished compiling "
#g++ -c StRoot/MyAnalysis/JEvent.cxx -I/usr/local/root/include -I/StRoot/MyAnalysis `root-config --libs`
#echo "Finished compiling JEvent.o"
#g++ -c StRoot/MyAnalysis/TrackLevelContainor.cxx -I/usr/local/root/include -I/StRoot/MyAnalysis `root-config --libs`
#echo "Finished compiling TrackLevelContainor.o"
#g++ -g -o mixingTest.bin mixingTest.cxx JEvent.o TrackLevelContainor.o  -I/usr/local/root/include -I/StRoot/MyAnalysis `root-config --libs`
#echo "Everything is compiled now???"





