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
    double dPhi = TVector2::Phi_mpi_pi(TVector2::Phi_mpi_pi(x->phi()) - TVector2::Phi_mpi_pi(y->phi()));
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
TString PriorHistName[nPriorJetTypes] = {"pltLevel", "detLevel", "recoLevel"};

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


//response matrix
TH2F *h_response_bkg_pt; 
TH2F *h_response_det_pt; 
TH2F *h_response_cum_pt; 


//efficiency
TH1F *h_pt[nPriorJetTypes];
TH2F *h_pt_m2[nPriorJetTypes];
TH1F *h_matched_pt[nPriorJetTypes];
TH2F *h_matched_pt_m2[nPriorJetTypes]; 
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
    h_response_bkg_pt = new TH2F("h_response_bkg_pt", "h_response_bkg_pt", 300, -50, 100, 300, -50, 100); 
    h_response_det_pt  = new TH2F("h_response_det_pt", "h_response_det_pt", 300, -50, 100, 300, -50, 100); 
    h_response_cum_pt  = new TH2F("h_response_cum_pt", "h_response_cum_pt", 300, -50, 100, 300, -50, 100);
    h_response_bkg_pt->Sumw2();
    h_response_det_pt->Sumw2(); 
    h_response_cum_pt->Sumw2();

    for (int i = 0; i < nPriorJetTypes; i++){ 
        TString ptHistName = Form("h_%s_pt", PriorHistName[i].Data());
        h_pt[i] = new TH1F(ptHistName, ptHistName, 300, -50, 100);
        TString ptm2HistName = Form("h_%s_pt_m2", PriorHistName[i].Data());
        h_pt_m2[i] = new TH2F(ptm2HistName, ptm2HistName, 300, -50, 100, 300, -20, 130);
        TString matchedPtHistName = Form("h_matched_%s_pt", PriorHistName[i].Data());
        h_matched_pt[i] = new TH1F(matchedPtHistName, matchedPtHistName, 300, -50, 100); 
        TString matchedPtM2HistName = Form("h_matched_%s_pt_m2", PriorHistName[i].Data());
        h_matched_pt_m2[i] = new TH2F(matchedPtM2HistName, matchedPtM2HistName, 300, -50, 100, 300, -20, 130); 
        
        cout << ptHistName << " " <<ptm2HistName << endl;
        
        h_pt[i]->Sumw2();
        h_pt_m2[i]->Sumw2();
        h_matched_pt[i]->Sumw2();
        h_matched_pt_m2[i]->Sumw2();
    }

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
    init(1, k510, kZr, kCent);
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

        for (int ip = 0; ip < nPltLevelJets; ip++) {
            bool isPltJetMatchedDetJet = false;
            TString matchedName[2] = {"failed", "matched"};
            TrackLevelJetArray *pltJet = (TrackLevelJetArray*)tca_pltLevelJets->At(ip);
            double pltJetPt = pltJet->pt();
            double pltJetM2 = (pltJet->m()) * (pltJet->m()); 
            double matchedPltJetPt = -999;
            double matchedDetJetPt = -999;
            double matchedRecoJetPtc = -999;
            double matchedPltJetM2 = -999; 
            double matchedDetJetM2 = -999; 
            double matchedRecoJetM2 = -999; 

            h_pt[kParticleLevel]->Fill(pltJetPt, priorWeight); 
            h_pt_m2[kParticleLevel]->Fill(pltJetPt, pltJetM2, priorWeight);

            for (int id = 0; id < nDetLevelJets; id ++) {
                TrackLevelJetArray *detJet = (TrackLevelJetArray*)tca_detLevelJets->At(id); 
                double detJetPt = detJet->pt(); 
                double detJetM2 = detJet->m() * detJet->m(); 
                int detUserIndex = detJet->userIndex(); 
                int nMatchedDetIndex = 0; 
                double dR = GetDeltaR(pltJet, detJet); 
                
                if (dR > 0.4) continue;
                double fraction = detJetPt / pltJetPt;
                if (detJetPt > pltJetPt) fraction = pltJetPt / detJetPt;
                if (fraction < 0.15) continue;

                for (int ir = 0; ir < nRecoLevelJets; ir++){ 
                    TrackLevelJetArray *recoJet = (TrackLevelJetArray*)tca_recoLevelJets->At(ir); 
                    double recoJetPt = recoJet->ptc(); 
                    double recoJetM2 = recoJet->m2c();
                    int recoUserIndex = recoJet->userIndex(); 

                    if (recoUserIndex == detUserIndex) {
                        nMatchedDetIndex ++;
                        matchedRecoJetPtc = recoJetPt;    
                        matchedRecoJetM2  = recoJetM2;
                    }
                }

                if (nMatchedDetIndex == 0) continue;
                isPltJetMatchedDetJet = true;
                matchedDetJetPt = detJetPt; 
                matchedDetJetM2 = detJetM2;
            }

            if (isPltJetMatchedDetJet) { 
                h_matched_pt[kParticleLevel]->Fill(pltJetPt, priorWeight);
                h_matched_pt[kDetectorLevel]->Fill(matchedDetJetPt, priorWeight);
                h_matched_pt[kRecoLevel]->Fill(matchedRecoJetPtc, priorWeight);
                h_matched_pt_m2[kParticleLevel]->Fill(pltJetPt, pltJetM2, priorWeight);
                cout << pltJetM2 << " " << matchedDetJetM2 << " " << matchedRecoJetM2 << endl;
                h_response_cum_pt->Fill(matchedRecoJetPtc, pltJetPt, priorWeight);
                h_response_det_pt->Fill(matchedDetJetPt, pltJetPt, priorWeight); 
                h_response_bkg_pt->Fill(matchedRecoJetPtc, matchedDetJetPt, priorWeight);
            }
        }        

        
        for (int id = 0; id < nDetLevelJets; id++){
            TrackLevelJetArray *detJet = (TrackLevelJetArray*)tca_detLevelJets->At(id); 
            double detJetPt = detJet->pt(); 
            h_pt[kDetectorLevel]->Fill(detJetPt, priorWeight); 
        }

        for (int ir = 0; ir < nRecoLevelJets; ir++) { 
            TrackLevelJetArray *recoJet = (TrackLevelJetArray*)tca_recoLevelJets->At(ir);
        }
        
        //-----------------------------------------------------------------------------
        tca_pltLevelJets->Delete();
        tca_detLevelJets->Delete();
        tca_recoLevelJets->Delete();
    }

    finish();
}