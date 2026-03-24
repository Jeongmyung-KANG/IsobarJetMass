#include <iostream> 
#include "fastjet/ClusterSequenceArea.hh"  // use this instead of the "usual" ClusterSequence to get area support
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/Selector.hh"
#include "fastjet/tools/JetMedianBackgroundEstimator.hh"
#include "fastjet/tools/Subtractor.hh"
#include "TFile.h"
#include "TObject.h"
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TTree.h"
#include "TParticle.h"
#include "TMath.h"
#include "TRandom.h"
#include "TROOT.h"
#include "TF1.h"
#include "TH1.h"
#include "TH2.h"
#include "TF2.h"
#include "TVector2.h"

using namespace std;
using namespace fastjet;

TString pythiaPath;
TFile *fin;
TTree *tree; 
TClonesArray *tca_pythiaTracks; 

const double jetRadius = 0.4; 

void init(TString finName, TString treeName, TString branchName){ 
    pythiaPath = finName; 
    fin = TFile::Open(finName);
    tree = (TTree*)fin->Get(treeName); 
    tca_pythiaTracks = new TClonesArray("TParticle", 100000); 
    tree->SetBranchAddress("tracks", &tca_pythiaTracks); 
}

int main(){ 
    init("mc_res_1015_1.root", "tree", "tracks");

    int nPythiaEvents = tree->GetEntries(); 

    for (int i = 0; i < nPythiaEvents; i++) { 
        tree->GetEntry(i); 
        int nPythiaTracks = tca_pythiaTracks->GetEntriesFast();
        vector<PseudoJet> pythiaTracks;
        for (int j = 0; j < nPythiaTracks; j++){ 
            TParticle *track = (TParticle*)tca_pythiaTracks->At(j);
            bool isTrackPtCut = (track->Pt() > 0.2);
            // ..etc
            if (!isTrackPtCut) continue;
            PseudoJet passedTrack = PseudoJet(track->Px(), track->Py(), track->Pz(), track->Energy());
            passedTrack.set_user_index(1234); // will be very useful
            pythiaTracks.push_back(passedTrack);
        }
        
        JetDefinition   jet_def(antikt_algorithm, jetRadius);
        GhostedAreaSpec  area_spec(1.0);
        AreaDefinition  area_def(active_area, GhostedAreaSpec(1.0, 1, 0.01));

        ClusterSequenceArea csa_pythiaJets(pythiaTracks, jet_def, area_def); 
        vector<PseudoJet> pythiaJets = sorted_by_pt(csa_pythiaJets.inclusive_jets());

        cout << pythiaTracks.size() << " " << pythiaJets.size() << endl;
        tca_pythiaTracks->Delete();
    }   
    return 0;
}