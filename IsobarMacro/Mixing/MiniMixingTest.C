


#include <iostream>
#include <filesystem> 
#include <filesystem> 
#include <cassert>
#include "TFile.h"
#include "TTree.h"
#include "TString.h" 
#include "TMath.h"
#include "TClonesArray.h"
#include "TRandom3.h"
#include "TH1.h"
#include "TParticle.h"
#include "TVector3.h"
#include "TSystem.h"
#include "TChain.h"
#include "TRandom.h"
#include "TSystem.h"
#include <fstream>


using namespace std;
namespace fs = std::filesystem;

TFile *fout;
TString foutName; 


TChain *classChain; 
TString MB_path = "/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/Embedding/"; 

string ZrGoodRunPath = "/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/Embedding/ZrGoodRun.list";
string RuGoodRunPath = "/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/Embedding/RuGoodRun.list";

TString SysNames[2] = {"Zr", "Ru"};
TString CentralityNames[2] = {"cent", "peri"};
TString SysName;
TString CentralityName; 

void MinMixingTest(int kSys = 0, int kCentrality = 0, int classIndex = 100, double maxMixedEventTrackCut = 5.0){
    
    vector<TClonesArray*> MB_evnet_tracks; 

    SysName = SysNames[kSys];
    CentralityName = CentralityNames[kCentrality];
    foutName = Form("mixingResults_maxtrackCut%d_%s_%s_%d.root", int(maxMixedEventTrackCut), SysName.Data(), CentralityName.Data(), classIndex);
    TClonesArray *tca_MB_tracks = new TClonesArray("TParticle", 1000000); 

    MB_path += to_string(classIndex);
    classChain = new TChain("tree"); 

    ifstream RunNumberFile; 
    string str;

    if (kSys == 0) RunNumberFile.open(ZrGoodRunPath);
    if (kSys == 1) RunNumberFile.open(RuGoodRunPath);
    
    while (std::getline(RunNumberFile, str)){
        std::istringstream iss(str);
        string nf;
        vector<string> p__;
        while (iss >> nf)
            {
                p__.push_back(nf);
            }
        if(p__.size()<=0) continue;
        cout << p__.at(0) << endl;
        TString runNumber = p__.at(0) ; 
        TString filePath = Form("%s/SepEvents_%s.root", MB_path.Data(), runNumber.Data()); 
        cout << filePath << endl;
        classChain->Add(filePath.Data());   
    }    

    classChain->SetBranchAddress("tracks", &tca_MB_tracks);    

    fout = new TFile(foutName, "RECREATE"); 
    TTree *outTree = new TTree("outTree", "outTree");
    TClonesArray *tca_ME_tracks = new TClonesArray("TParticle", 1000000);
    outTree->Branch("TParticle", &tca_ME_tracks);

    int maxNumMixedEvent  = classChain->GetEntries(); 
    

    MB_evnet_tracks.resize(maxNumMixedEvent);

    for (int i = 0; i < maxNumMixedEvent; i++){
        if (i%10000 == 0) cout << "Loading " << i << "/" << maxNumMixedEvent << endl;
        classChain->GetEntry(i);
        int nTracksIn_i_th_event = tca_MB_tracks->GetEntriesFast(); 
        MB_evnet_tracks[i] = new TClonesArray("TParticle", nTracksIn_i_th_event);
        
        for (int j = 0; j < nTracksIn_i_th_event; j++){
            TParticle *MB_track = (TParticle*)tca_MB_tracks->At(j); 
            TParticle *MB_fill_track = new ((*MB_evnet_tracks[i])[j]) TParticle();
            MB_fill_track->SetMomentum(MB_track->Px(), MB_track->Py(), MB_track->Pz(), MB_track->Energy()); 
        }
        
        //cout << i << " " << nTracksIn_i_th_event << " " << MB_evnet_tracks[i]->GetEntriesFast() << endl; 
        //tca_MB_tracks->Clear("C");
        tca_MB_tracks->Clear("C");
    }   

    TRandom3 randomNumber; 

    for (int i = 0; i < maxNumMixedEvent; i++) {
        int nTracksIn_i_th_event = MB_evnet_tracks[i]->GetEntriesFast(); 
        cout << i << "/" << maxNumMixedEvent << " " << nTracksIn_i_th_event << " " << endl; 
        int randomEventIndices[maxNumMixedEvent];
        int randomTarckIndices[nTracksIn_i_th_event];

        double testNum = randomNumber.Uniform();
        for (int f = 0; f < maxNumMixedEvent; f++){randomEventIndices[f] = randomNumber.Uniform(maxNumMixedEvent);}
        for (int f = 0; f < nTracksIn_i_th_event; f++){randomTarckIndices[f] = randomNumber.Uniform(nTracksIn_i_th_event);}

        for (int j = 0; j < nTracksIn_i_th_event; j++) {
            int randomEventIndex = randomEventIndices[i];//randomNumber.Uniform(maxNumMixedEvent);
            int randomTrackIndex = randomTarckIndices[j];//randomNumber.Uniform(MB_evnet_tracks[randomEventIndex]->GetEntriesFast());
            TParticle *ME_trackCandidate = (TParticle*)MB_evnet_tracks[randomEventIndex]->At(randomTrackIndex); 
            TParticle *ME_fill_trackCandidate =  new ((*tca_ME_tracks)[j]) TParticle();
            ME_fill_trackCandidate->SetMomentum(ME_trackCandidate->Px(), ME_trackCandidate->Py(), ME_trackCandidate->Pz(), ME_trackCandidate->Energy());
            //cout << "           pt : " << j << " " << ME_trackCandidate->Pt() << endl; 
        }

        outTree->Fill();
    }   

    cout << MB_evnet_tracks.size() << " " << maxNumMixedEvent << endl;
    
    fout->cd(); 
    fout->Write(); 
    fout->Close(); 

}   

int main(){
    MinMixingTest();
}