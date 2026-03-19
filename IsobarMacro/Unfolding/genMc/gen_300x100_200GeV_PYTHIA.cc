#include "Pythia8/Pythia.h"
#include <TH1.h>
#include <TMath.h>
#include <TFile.h>
#include <TTree.h>
#include <TClonesArray.h>
#include <TParticle.h>
#include <TString.h>
#include <TTimeStamp.h>
#include <TMath.h>

#include <string.h>
#include <iostream> // Include this to work with command-line arguments
using namespace std;
using namespace Pythia8;


bool GetIsChargedFinalParticle(int Pid, int firstDaughter){

    bool isCharged = (Pid == 211 || Pid == 321 || Pid == 2212 || Pid == 3122 || Pid == 3222 || Pid == 3112 || Pid == 3334 || Pid == 3312 || Pid == 13 || Pid == 11);
    bool isFinal = (firstDaughter == 90);
    if (isCharged) {
          return true;   
    }

    else return false; 
}

bool GetIsKinCuts(double eta, double pt){ 
    bool isEtaCut = TMath::Abs(eta) < 1.0;
    bool isPtCut = pt > 0.2;

    if (isEtaCut && isPtCut) {
        return true;
    }

    else return false;
}


int main(int argc, char* argv[]) {
  if (argc != 4) {
    cout << "Usage: " << argv[0] << " pthatmin pthatmax" << endl;
    return 1;
  }

  // Convert pthatmin and pthatmax from command-line arguments to float
  const float pthatmin = atof(argv[1]);
  const float pthatmax = atof(argv[2]);
  const float numiter = atof(argv[3]);

 // TFile* outFile = new TFile(TString::Format("/Volumes/PortableSSD/Jeongmyung/200GeV_PythiaTrack%.f%.f_%.f.root", pthatmin, pthatmax, numiter), "RECREATE"); 
  TFile *outFile = new TFile(Form("mc_10M_res_%.0f%.0f_%.0f.root", pthatmin, pthatmax, numiter), "RECREATE");
  TTree* tree; 
  tree = new TTree("treeName1"," tree Title 1");
  
  const int   nEvents = 10000; //Number of maximum events
  const float ymax = 1.0; //Maximum rapidity  
  
  Pythia8::Pythia pythia;
  
  TH1F *hPartonMass = new TH1F("m", "m", 100, 0, 100); 
  pythia.readString("HardQCD:all = on");
  pythia.readString("Beams:idA = 2212");
  pythia.readString("Beams:idB = 2212 ");
  pythia.readString("Beams:eCM = 5020."); // 200 GeV 
  pythia.readString("PhaseSpace:pTHatMin = " + to_string(pthatmin));
  pythia.readString("PhaseSpace:pTHatMax = " + to_string(pthatmax));
  //IMPORTANT! This will only create a seed on compilation, not on execution. Re-running the same executable will generate the same seed.
  pythia.readString("Random:setSeed = on"); 
  pythia.readString("Random:seed = " + to_string(numiter));

  cout<<"Random:seed = " + to_string(numiter)<<endl;

  int entries = pythia.event.size();

  pythia.init();

  TClonesArray* trackArray = new TClonesArray("TParticle", nEvents);
  tree->Branch("tracks", "TClonesArray", &trackArray);

  int nRealEvents = 0;
  Event& event = pythia.event;

  for (int iEvent = 0; iEvent < nEvents; ++iEvent) {
    if (!pythia.next()) continue;  
    if (iEvent < 1) {pythia.info.list(); pythia.event.list();}

    int tmp_index = 0; 
	  int  numTrigger = 0;

    for (int i = 0; i < pythia.event.size(); ++i) {
      Bool_t kResonance = kFALSE;
      
      int pdg = pythia.event[i].id(); 
      int fd_pdg = pythia.event[pythia.event[i].daughter1()].id();
      float eta = pythia.event[i].eta();
      float pt = pythia.event[i].pT(); 
      
      bool isChargedFinal = GetIsChargedFinalParticle(pdg, fd_pdg); 
      bool isKinCuts = GetIsKinCuts(eta, pt); 
	    bool isTrigger (pt > 9 && pt < 30);  

      if (pythia.event[i].isCharged() == true && pythia.event[i].isFinal() && TMath::Abs(eta) < 1 && pt > 0.2 ){
          TParticle* track =
            new((*trackArray)[tmp_index]) TParticle();
 //         cout << i << " " << tmp_index << " " << pt << " " << eta << " " <<  pdg << " " << fd_pdg << endl;
          track->SetPdgCode(pythia.event[i].id()); //pid of particle
          track->SetMother(0, pythia.event[i].mother1()); //Label of mother particle
          track->SetMother(1, pythia.event[pythia.event[i].mother1()].id()); //pid of mother particle
          track->SetFirstDaughter(pythia.event[pythia.event[i].daughter1()].id());
          track->SetMomentum(pythia.event[i].px(), pythia.event[i].py(),
                            pythia.event[i].pz(), pythia.event[i].e());//4 vector
          track->SetStatusCode(pythia.event[i].status());
          tmp_index ++;  
		  if (isTrigger){
			numTrigger ++; 
		  }
          //if (pythia.event[pythia.event[i].daughter1()].id() != 90) cout<<i<<"  "<<pythia.event[i].id()<<" first Dalla"<<pythia.event[i].daughter1()<<" fisrt Dallar index "<<pythia.event[pythia.event[i].daughter1()].id()<<endl; 
      }
    }
	
	if (numTrigger == 1){
    tree->Fill();
	}

    trackArray->Delete();
  }	

  pythia.stat();
  outFile->Write();
  outFile->Close();
}
