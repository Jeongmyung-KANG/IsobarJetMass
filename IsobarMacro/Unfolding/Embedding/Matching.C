#include <iostream>
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
#include "../Utils/TrackLevelJetArray.h"
#include <chrono>




#define LOCALTEST


double GetDeltaR(TrackLevelJetArray *x, TrackLevelJetArray *y) { 
    double dEta = x->eta() - y->eta();
    double dPhi = TVector2::Phi_0_2pi(TVector2::Phi_0_2pi(x->phi()) - TVector2::Phi_0_2pi(y->phi()));
    return TMath::Sqrt(dEta*dEta + dPhi*dPhi); 
}

using namespace std;

const double jetRadius = 0.4; 
const double areaCut = 0.35;
const int priorJetIndex = 181818;
const int priorRecoilJetIndex = 981223;

enum SysType {kZr, kRu, nSysType};
TString SysName[nSysType] = {"Zr", "Ru"};
enum CentType {kCent, kPeri, nCentType};
TString CentralityName[nCentType] = {"cent", "peri"};
enum rawHistType {kDet, kParticle, nRawHistType};
TString rawHistName[nRawHistType] = {"detLevel", "pltLevel"}; 
enum PythiaHatBinType {k510, k1015, k1520, k2025, k2530, k3040, k40200, nPythiaHatBins};
TString HatBinNames[nPythiaHatBins] = {"510", "1015", "1520", "2025", "2530", "3040", "40200"}; 
double HatBinWeights[nPythiaHatBins] = {1.910e-01, 5.190e-03, 4.572e-04, 6.321e-05, 1.159e-05, 2.895e-06, 1.567e-07};
enum PriorJetTypes {kParticleLevel, kDetectorLevel, kRecoLevel, nPriorJetTypes};
TString PriorTreeNames[nPriorJetTypes] = {"pltLevelTree", "detLevelTree", "recoLevelTree"};

//////////////////////////////////////////////////////////////////////////////////////////
TFile *fin; 
TTree *priorTree;
TClonesArray *tca_pltLevelJets;
TClonesArray *tca_detLevelJets;
TClonesArray *tca_recoLevelJets;

TH1F *h_prior_weight;
double priorWeight;
//////////////////////////////////////////////////////////////////////////////////////////




//////////////////////////////////////////////////////////////////////////////////////////
TFile *fout;
TH2F *h_response_bkg_ptc; 
TH2F *h_response_det_pt; 
//////////////////////////////////////////////////////////////////////////////////////////

double pythiaWeight = 1;
TString pythiaHatBinName = ""; 

void init(int fileIndex, int kHatBinType, int kSys, int kCentrality) {
    TString finLocation;
    TString finName;
    TString foutLocation;
    TString foutName;
    TString priorTreeNames[nPriorJetTypes]; 
    pythiaWeight = HatBinWeights[kHatBinType];
    pythiaHatBinName = HatBinNames[kHatBinType];

    #ifdef LOCALTEST 
        finLocation = "/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/Embedding";
        finName = Form("%s_%s_%s_hybridEvent.root", SysName[kSys].Data(), CentralityName[kCentrality].Data(), pythiaHatBinName.Data()); 
        foutLocation = "/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/Embedding";
        foutName = Form("%s_%s_%s_matchingResult.root", SysName[kSys].Data(), CentralityName[kCentrality].Data(), pythiaHatBinName.Data());
    #endif  
    



    
    //------------------------------------------------------------------------------------------------------------------------------------------
    fin = TFile::Open(finName);

    h_prior_weight = (TH1F*)fin->Get("h_prior_weight"); 
    priorWeight = h_prior_weight->GetBinContent(1);
    priorTree = (TTree*)fin->Get("embeddedTree");

    tca_pltLevelJets = new TClonesArray("TrackLevelJetArray", 10000); 
    tca_detLevelJets = new TClonesArray("TrackLevelJetArray", 10000); 
    tca_recoLevelJets = new TClonesArray("TrackLevelJetArray", 10000); 

    priorTree->SetBranchAddress("pltLevelJets", &tca_pltLevelJets);
    priorTree->SetBranchAddress("detLevelJets", &tca_detLevelJets);
    priorTree->SetBranchAddress("recoLevelJets", &tca_recoLevelJets);

    //------------------------------------------------------------------------------------------------------------------------------------------





    //------------------------------------------------------------------------------------------------------------------------------------------
    fout = new TFile(foutName, "RECREATE"); 
    h_response_bkg_ptc = new TH2F("h_response_bkg_ptc", "h_response_bkg_ptc", 200, -50, 200, 200, -50, 200); 
    h_response_det_pt  = new TH2F("h_response_det_pt", "h_response_det_pt", 200, -50, 200, 200, -50, 200); 
    //------------------------------------------------------------------------------------------------------------------------------------------
}

void finish(){
    fout->cd();
    fout->Write();
    fout->Close();
}
void Matching(){

    #ifdef LOCALTEST 
    R__LOAD_LIBRARY(/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/Utils/libTools.so);
    init(1, k40200, kZr, kCent);
    #endif

    //cout << priorTree->GetEntries() << " " << tca_detLevelJets->GetEntries() << " " << tca_recoLevelJets->GetEntries() << endl;
    //cout << h_prior_weight->GetBinContent(1) << endl;

    int nEvents = priorTree->GetEntries();

    for (int e=0; e<nEvents; e++){ 
        priorTree->GetEntry(e); 

        int nPltLevelJets = tca_pltLevelJets->GetEntriesFast();
        int nDetLevelJets = tca_detLevelJets->GetEntriesFast();
        int nRecoLevelJets = tca_recoLevelJets->GetEntriesFast(); 

        //cout << nPltLevelJets << " " << nDetLevelJets << " " << nRecoLevelJets << endl; 

        //det ---> reco level matching---------------------------------------------------------------------------------------------
        vector<int> matchedDetRecoIndices;

        for (int id = 0; id < nDetLevelJets; id++){
            TrackLevelJetArray *detJet = (TrackLevelJetArray*)tca_detLevelJets->At(id);
            bool isRecoDetMatched = false; 
            double detJetPt = detJet->pt();
            int priorIndexInDetJet = detJet->userIndex();
            if (priorIndexInDetJet < 0) continue;
            //cout << priorIndexInDetJet << endl;
            for (int ir = 0; ir < nRecoLevelJets; ir++){
                TrackLevelJetArray *recoJet = (TrackLevelJetArray*)tca_recoLevelJets->At(ir);
                int priorIndexInRecoJet = recoJet->userIndex();
                double recoJetPtc = recoJet->ptc();
                if (priorIndexInRecoJet < 0) continue;
                if (priorIndexInDetJet == priorIndexInRecoJet) {
                    h_response_bkg_ptc->Fill(recoJetPtc, detJetPt, priorWeight); 
                    isRecoDetMatched = true; 
                    //cout << recoJetPtc << " " << detJetPt << endl;
                    matchedDetRecoIndices.push_back(id);
                }
            }
        }   
        //end reco level matching---------------------------------------------------------------------------------------------
        
        if (matchedDetRecoIndices.size() == 0) continue;

        //plt ---> det level matchig -----------------------------------------------------------------------------
        for (int ip = 0; ip < nPltLevelJets; ip++) {
            TrackLevelJetArray *pltJet = (TrackLevelJetArray*)tca_pltLevelJets->At(ip);
            double pltJetPt = pltJet->pt();
            double pltJetPhi = pltJet->phi(); 
            double pltJetEta = pltJet->eta();

            for (int id = 0; id < matchedDetRecoIndices.size(); id++){
                TrackLevelJetArray *detJet = (TrackLevelJetArray*)tca_detLevelJets->At(matchedDetRecoIndices[id]);
                double detJetPt = detJet->pt();
                double detJetPhi = detJet->phi(); 
                double detJetEta = detJet->eta();
                double dR = GetDeltaR(pltJet, detJet);
                double nom;
                double denom;

                if (pltJetPt >= detJetPt) {
                    nom = pltJetPt;
                    denom = detJetPt;
                }
                if (pltJetPt <= detJetPt) {
                    nom = detJetPt;
                    denom = pltJetPt;
                }
               

                double fraction = denom / nom;

                if (dR < 0.4 && fraction > 0.75) {
                    cout << e << "  "<< fraction << " " << pltJetPt << " " << detJetPt << " " << detJetPt / pltJetPt << endl;
                    h_response_det_pt->Fill(detJetPt, pltJetPt);
                    }
            }
        }        

        //-----------------------------------------------------------------------------
        

        tca_detLevelJets->Delete();
        tca_recoLevelJets->Delete();
    }

    finish();
}