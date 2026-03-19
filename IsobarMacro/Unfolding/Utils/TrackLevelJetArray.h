#ifndef TRACKLEVELJETARRAY_H
#define TRACKLEVELJETARRAY_H

#include "TObject.h"
#include <iostream>
#include <TMath.h>

class TrackLevelJetArray : public TObject{
    public :
        TrackLevelJetArray();
        ~TrackLevelJetArray(); 

        //void SetPxPyPzE(double x, double y, double z, double _e) {px = x; py = y; pz = z; e = _e; pt = TMath::Sqrt(x*x + y*y); m = TMath::Sqrt(_e*_e - x*x - y*y - z*z);}
        //void SetPhiEta(double p, double e) {phi = p; eta = e;}
        //void SetAreaFourVector(double _ax, double _ay, double _az, double _ae) {ax = _ax; ay = _ay; az = _az; ae = _ae;}

        void SetPxPyPzE(double x, double y, double z, double _e);
        void SetPhiEta(double p, double e);
        void SetAreaFourVector(double _ax, double _ay, double _az, double _ae);

        double Px() const {return px;}

        ClassDef(TrackLevelJetArray, 1);
    
    private :
        double px; 
        double py;
        double pz;
        double e;
        double pt;
        double eta;
        double phi; 
        double m;
        double area;
        double ax, ay, az, ae; 
};


#endif 
