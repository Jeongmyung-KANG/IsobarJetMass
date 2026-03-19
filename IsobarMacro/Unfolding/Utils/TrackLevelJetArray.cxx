#include "TrackLevelJetArray.h"

ClassImp(TrackLevelJetArray)

TrackLevelJetArray::TrackLevelJetArray(): TObject() {}
TrackLevelJetArray::~TrackLevelJetArray(){}

void TrackLevelJetArray::SetPxPyPzE(double x, double y, double z, double _e) {__px = x; __py = y; __pz = z; _e = __e; __pt = TMath::Sqrt(x*x + y*y); __m = TMath::Sqrt(_e*_e - x*x - y*y - z*z);}
void TrackLevelJetArray::SetPhiEta(double p, double e) {__phi = p, __eta = e;}
void TrackLevelJetArray::SetArea(double a) {__area = a;}
void TrackLevelJetArray::SetAreaFourVector(double x, double y, double z, double e) {__ax =x; __ay = y; __az = z; __ae = e;}
void TrackLevelJetArray::SetUserIndex(int i) {__userIndex = i;}
void TrackLevelJetArray::SetPtc(double r) {__ptc = r;}
