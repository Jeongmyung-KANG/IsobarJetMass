#include "TrackLevelJetArray.h"

ClassImp(TrackLevelJetArray)

TrackLevelJetArray::TrackLevelJetArray(): __constiPrioIndexVector(0), TObject() {
    __px = -1;
    __py = -1; 
    __pz = -1;
    __e  = -1; 
    __pt = -1; 
    __eta = -100;
    __phi = -100;
    __m =-200;
    __area = -100;
    __ax = -100; __ay = -100; __az = -100; __ae = -100; 
    __ptc = -100;
    __m2c = -500; 
    __userIndex = -1;

}
TrackLevelJetArray::~TrackLevelJetArray(){
    __constiPrioIndexVector.clear(); 
}

void TrackLevelJetArray::SetPxPyPzE(double x, double y, double z, double _e) {__px = x; __py = y; __pz = z; _e = __e; __pt = TMath::Sqrt(x*x + y*y); __m = TMath::Sqrt(_e*_e - x*x - y*y - z*z);}
void TrackLevelJetArray::SetPhiEta(double p, double e) {__phi = p, __eta = e;}
void TrackLevelJetArray::SetArea(double a) {__area = a;}
void TrackLevelJetArray::SetAreaFourVector(double x, double y, double z, double e) {__ax =x; __ay = y; __az = z; __ae = e;}
void TrackLevelJetArray::SetUserIndex(int inputIndex) {
    
    __userIndex = inputIndex;
    //std::cout << "input index : " << "input : " << inputIndex << " result : " << __userIndex << std::endl; 
    }
void TrackLevelJetArray::SetConstiPtSum(double r) {__constiPtSum = r;}
void TrackLevelJetArray::SetPtc(double r) {__ptc = r;}
void TrackLevelJetArray::SetM2c(double r) {__m2c = r;}
