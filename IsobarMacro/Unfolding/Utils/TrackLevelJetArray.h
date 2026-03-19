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
        void SetArea (double a); 
        void SetAreaFourVector(double _ax, double _ay, double _az, double _ae);
        void SetUserIndex(int i); 
        void SetPtc(double r);    
        
        double px() const {return __px;} 
        double py() const {return __py;}
        double pz() const {return __pz;}
        double e() const  {return __e;}
        double ptc() const {return __ptc;}
        double phi() const {return __phi;}
        double eta() const {return __eta;}
        double m() const {return __m;}
        double area() const {return __area;}

        ClassDef(TrackLevelJetArray, 1);
    
    private :
        double __px; 
        double __py;
        double __pz;
        double __e;
        double __pt;
        double __eta;
        double __phi; 
        double __m;
        double __area;
        double __ax, __ay, __az, __ae; 
        double __ptc;
        bool __userIndex;
};


#endif 
