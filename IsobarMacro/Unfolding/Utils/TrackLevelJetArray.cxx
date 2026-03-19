#include "TrackLevelJetArray.h"

ClassImp(TrackLevelJetArray)

TrackLevelJetArray::TrackLevelJetArray(): TObject() {}
TrackLevelJetArray::~TrackLevelJetArray(){}

void TrackLevelJetArray::SetPxPyPzE(double x, double y, double z, double _e) {px = x; py = y; pz = z; e = _e; pt = TMath::Sqrt(x*x + y*y); m = TMath::Sqrt(_e*_e - x*x - y*y - z*z);}
