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
#include "TrackLevelJetArray.h"
#include <chrono>


#define LOCALTEST

using namespace std;
using namespace fastjet;

const double jetRadius = 0.4; 

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


double params_zr_cent_pos[9] = {2.269, 1.407, 0.06189, 2.259,  -2.33,    1.16,   0.02825, -2.066,   3.059};
double params_zr_cent_neg[9] = {2.942, 2.079, 0.0443,  2.373,  -2.982,   1.682,  0.2354,  -0.5651,  1.275};
double params_zr_peri_neg[9] = {4.296, 3.406, 0.02562, 0.2758, -0.06651, 0.6565, 0.102,   -0.6077,  1.084};
double params_zr_peri_pos[9] = {2.543, 1.653, 0.04662, 3.64,   0.7146,   0.7392, 3.558,    0.7376,  0.7321};

TH1F *h_prior_pt_fs;
TH1F *h_prior_pt;
TH1F *h_prior_phi;
TH1F *h_prior_eta;
TH1F *h_prior_weight;

TF1 *eff_default_zr_cent_pos;
TF1 *eff_default_zr_cent_neg;
TF1 *eff_default_zr_peri_pos;
TF1 *eff_default_zr_peri_neg;
TF1 *eff_pos;
TF1 *eff_neg;

TFile *fin_pythia_fastSim; 
TFile *fout;
TFile *f_mb_summary;

TTree *embeddedTree; 
TTree *summaryTree;

TClonesArray *tca_pltLevelJets;
TClonesArray *tca_detLevelJets; 
TClonesArray *tca_recoLevelJets;

int cumEventNumber; 
int eventIndex;
int fileIndex; 
int eventClassNumber;

double pythiaWeight = 1;
TString pythiaHatBinName = ""; 

double trigPhi;
double trigPt; 

void init(int p6FileIndex, int kHatBinType, int kSys, int kCentrality){     
    gRandom->SetSeed(p6FileIndex); 
    pythiaWeight = HatBinWeights[kHatBinType];
    pythiaHatBinName = HatBinNames[kHatBinType];

    eff_default_zr_cent_pos = new TF1("eff_default_zr_cent_pos", "[0] - [1]*exp([2] / x) + [3] * exp(-(x-[4])*(x-[4]) / (2*[5]*[5])) - [6] * exp(-(x-[7])*(x-[7]) / (2*[8]*[8]))", 0, 20);
    eff_default_zr_cent_pos->SetParameters(params_zr_cent_pos[0], params_zr_cent_pos[1], params_zr_cent_pos[2], params_zr_cent_pos[3], params_zr_cent_pos[4], params_zr_cent_pos[5], params_zr_cent_pos[6], params_zr_cent_pos[7], params_zr_cent_pos[8]);
    eff_default_zr_cent_neg = new TF1("eff_default_zr_cent_neg", "[0] - [1]*exp([2] / x) + [3] * exp(-(x-[4])*(x-[4]) / (2*[5]*[5])) - [6] * exp(-(x-[7])*(x-[7]) / (2*[8]*[8]))", 0, 20);
    eff_default_zr_cent_neg->SetParameters(params_zr_cent_neg[0], params_zr_cent_neg[1], params_zr_cent_neg[2], params_zr_cent_neg[3], params_zr_cent_neg[4], params_zr_cent_neg[5], params_zr_cent_neg[6], params_zr_cent_neg[7], params_zr_cent_neg[8]);
    eff_default_zr_peri_pos = new TF1("eff_default_zr_peri_pos", "[0] - [1]*exp([2] / x) + [3] * exp(-(x-[4])*(x-[4]) / (2*[5]*[5])) - [6] * exp(-(x-[7])*(x-[7]) / (2*[8]*[8]))", 0, 20);
    eff_default_zr_peri_pos->SetParameters(params_zr_peri_pos[0], params_zr_peri_pos[1], params_zr_peri_pos[2], params_zr_peri_pos[3], params_zr_peri_pos[4], params_zr_peri_pos[5], params_zr_peri_pos[6], params_zr_peri_pos[7], params_zr_peri_pos[8]);
    eff_default_zr_peri_neg = new TF1("eff_default_zr_peri_neg", "[0] - [1]*exp([2] / x) + [3] * exp(-(x-[4])*(x-[4]) / (2*[5]*[5])) - [6] * exp(-(x-[7])*(x-[7]) / (2*[8]*[8]))", 0, 20);
    eff_default_zr_peri_neg->SetParameters(params_zr_peri_neg[0], params_zr_peri_neg[1], params_zr_peri_neg[2], params_zr_peri_neg[3], params_zr_peri_neg[4], params_zr_peri_neg[5], params_zr_peri_neg[6], params_zr_peri_neg[7], params_zr_peri_neg[8]);

    if (kSys == kZr) {
        if (kCentrality == kCent) {
            eff_pos = (TF1*)eff_default_zr_cent_pos->Clone("eff_pos"); 
            eff_neg = (TF1*)eff_default_zr_cent_neg->Clone("eff_neg"); 
        }

        if (kCentrality == kPeri) { 
            eff_pos = (TF1*)eff_default_zr_peri_pos->Clone("eff_pos"); 
            eff_neg = (TF1*)eff_default_zr_peri_neg->Clone("eff_neg"); 
        }
         
    }

#ifdef LOCALTEST 
    TString p6FileName = "../genMc/mc_res_40200_1.root";   
    fin_pythia_fastSim = TFile::Open(p6FileName); 
    f_mb_summary = TFile::Open(Form("/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/Embedding/MB_summary.root"));
    summaryTree = (TTree*)f_mb_summary->Get("summaryTree");

  #else 
  TString p6FileName = Form("p6out_%d.root", p6FileIndex);    
  TString embeddedEventName = Form("embedded_%d.root", p6FileIndex);
  fin_pythia_fastSim = TFile::Open(p6FileName); 
  fout = new TFile(embeddedEventName, "RECREATE"); 
#endif  

    summaryTree->SetBranchAddress("cumEventNumber", &cumEventNumber);
    summaryTree->SetBranchAddress("eventIndex", &eventIndex);
    summaryTree->SetBranchAddress("eventClassNumber", &eventClassNumber); 
    summaryTree->SetBranchAddress("fileIndex", &fileIndex); 
    
    

    fout = new TFile(Form("%s_%s_%s_hybridEvent.root", SysName[kSys].Data(), CentralityName[kCentrality].Data(), pythiaHatBinName.Data()), "RECREATE");
    embeddedTree = new TTree("embeddedTree", "embeddedTree"); 
    tca_pltLevelJets = new TClonesArray("TrackLevelJetArray", 10000);
    tca_detLevelJets = new TClonesArray("TrackLevelJetArray", 10000); 
    tca_recoLevelJets = new TClonesArray("TrackLevelJetArray", 10000);
    embeddedTree->Branch("pltLevelJets", &tca_pltLevelJets);
    embeddedTree->Branch("detLevelJets", &tca_detLevelJets);
    embeddedTree->Branch("recoLevelJets", &tca_recoLevelJets); 
    embeddedTree->Branch("triggerPt", &trigPt);
    embeddedTree->Branch("triggerPhi", &trigPhi);
    
    //priorTree = (TTree*)fin_pythia_fastSim->Get("outtree"); 
    //if (!priorTree) {cout << "TREE IS FUNCKING EMPTY" << endl;}
    h_prior_pt_fs = new TH1F("h_prior_pt_fs", "h_prior_pt_fs", 1000, 0, 20);
    h_prior_pt    = new TH1F("h_prior_pt",    "h_prior_pt"   , 1000, 0, 20); 
    h_prior_phi    = new TH1F("h_prior_phi",    "h_prior_phi"   , 1000, -10, 10); 
    h_prior_eta    = new TH1F("h_prior_eta",    "h_prior_eta"   , 1000, -5, 5); 
    h_prior_weight = new TH1F("h_prior_weight", "h_prior_weight", 1, 0, 1);
    h_prior_pt_fs->Sumw2(); 
    h_prior_pt->Sumw2(); 
    h_prior_phi->Sumw2(); 
    h_prior_eta->Sumw2();
    h_prior_weight->Sumw2();
    h_prior_weight->Fill(0.5, pythiaWeight);
} 

bool doDiceRoll(double charge, double pt) {
    bool result = false;
    double dice = gRandom->Uniform(0,1);
    double ref_dice;
    if (charge > 0) { 
        ref_dice = eff_pos->Eval(pt); 
    }
    //cout << pt << " " << dice << " " << ref_dice << endl; 
    if (dice < ref_dice) result = true;
    return result; 
}

void eventLoop(){ 
  TTree *priorTree = (TTree*)fin_pythia_fastSim->Get("tree");  
  TClonesArray *tca_priorTracks = new TClonesArray("TParticle", 10000000);  
  TClonesArray *tca_MB_tracks = new TClonesArray("TParticle", 10000000); 

  priorTree->SetBranchAddress("tracks", &tca_priorTracks); 
  //cout << priorTree->GetEntries() << endl; 
  
  

  int nPriorEvents = priorTree->GetEntries();
  cout << "tot event : " << priorTree->GetEntries() << endl;
  for (int i = 0; i < nPriorEvents; i++) { 
    if (i % 100 ==0) cout << "event : " << i << endl; 
    priorTree->GetEntry(i); 
    int callIndex = gRandom->Uniform(1, summaryTree->GetEntries());
    summaryTree->GetEntry(callIndex-1); 
    //cout << eventClassNumber << " " << fileIndex << endl; 
    //cout << fileIndex << " " << eventIndex << endl;
    
    TString MinBiasName = Form("/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/%d/METracks_%d.root", eventClassNumber, fileIndex);
    TFile *finMinBias = TFile::Open(MinBiasName);
    TTree *MinBiasTree = (TTree*)finMinBias->Get("tree");
    MinBiasTree->SetBranchAddress("tracks", &tca_MB_tracks); 

    MinBiasTree->GetEntry(eventIndex);
  
    vector<double> vec_trigPhi;
    vector<double> vec_trigPt;

    if (tca_priorTracks->GetEntriesFast() == 0) continue;

    vector<PseudoJet> pltLevelTracks;
    vector<PseudoJet> pltLevelJets;

    vector<PseudoJet> detLevelTracks;
    vector<PseudoJet> detLevelJets;

    vector<PseudoJet> recoLevelTracks;
    vector<PseudoJet> recoLevelJets;

    for (int ei = 0; ei < tca_priorTracks->GetEntriesFast(); ei++) { 
      TParticle *priorTrack = (TParticle*)tca_priorTracks->At(ei);
      if (priorTrack->Pt() < 0.2) continue;
      if (TMath::Abs(priorTrack->Eta()) > 1.0) continue;
      bool isPassFastSim = doDiceRoll(1, priorTrack->Pt());
      h_prior_pt->Fill(priorTrack->Pt());
      h_prior_phi->Fill(priorTrack->Phi());
      h_prior_eta->Fill(priorTrack->Eta());

      double priorTrackPt = priorTrack->Pt(); 
      double priorTrackPhi = priorTrack->Phi();

      if (priorTrackPt > 7 && priorTrackPt < 25){ 
        vec_trigPt.push_back(priorTrackPt); 
        vec_trigPhi.push_back(priorTrackPhi); 
      }

      PseudoJet track = PseudoJet(priorTrack->Px(), priorTrack->Py(), priorTrack->Pz(), priorTrack->Energy()); 
      pltLevelTracks.push_back(track); 

      if (!isPassFastSim) continue;

      detLevelTracks.push_back(track);
      h_prior_pt_fs->Fill(priorTrack->Pt());
    }

    if (vec_trigPt.size() == 0) continue;
    int triggerTrackIndex = gRandom->Uniform(vec_trigPt.size());
    trigPt = vec_trigPt[triggerTrackIndex]; 
    trigPhi = TVector2::Phi_0_2pi(vec_trigPhi[triggerTrackIndex]);


    JetDefinition   jet_def(antikt_algorithm, jetRadius);
    GhostedAreaSpec  area_spec(1.0);
    AreaDefinition  area_def(active_area, GhostedAreaSpec(1.0, 1, 0.01));

    ClusterSequenceArea csa_pltLevelJets(pltLevelTracks, jet_def, area_def); 
    ClusterSequenceArea csa_detLevelJets(detLevelTracks, jet_def, area_def); 
    pltLevelJets = sorted_by_pt(csa_pltLevelJets.inclusive_jets()); 
    detLevelJets = sorted_by_pt(csa_detLevelJets.inclusive_jets());

    int fillIndex = 0; 
    for (int ji=0; ji<pltLevelJets.size(); ji++){
      PseudoJet jet = pltLevelJets[ji];
      double jet_phi = TVector2::Phi_0_2pi(jet.phi()); 
      double jet_eta = jet.eta(); 
      double dphi = TVector2::Phi_0_2pi(trigPhi - jet_phi);

      if (!(dphi > 3*TMath::Pi()/4 && dphi < 5*TMath::Pi()/4)) continue;
      if (TMath::Abs(jet_eta) > 1.0 - jetRadius) continue;
      TrackLevelJetArray *pltJet = new((*tca_pltLevelJets)[fillIndex]) TrackLevelJetArray();
      pltJet->SetPxPyPzE(jet.px(), jet.py(), jet.pz(), jet.e());
      pltJet->SetPhiEta(jet_phi, jet_eta); 
      pltJet->SetArea(jet.area()); 
      fillIndex++;
    }
    fillIndex = 0;

    for (int ji=0; ji<detLevelJets.size(); ji++){
      PseudoJet jet = detLevelJets[ji]; 
      double jet_phi = jet.phi(); 
      double jet_eta = jet.eta(); 
      double dphi = TVector2::Phi_0_2pi(trigPhi - jet_phi);
      bool isRecoil = (dphi > 3*TMath::Pi()/4 && dphi < 5*TMath::Pi()/4);
      if (TMath::Abs(jet_eta) > 1.0 - jetRadius) continue;

      if (isRecoil) {
        jet.set_user_index(45600 + ji); 
        TrackLevelJetArray *detJet = new ((*tca_detLevelJets)[fillIndex]) TrackLevelJetArray();
        detJet->SetPxPyPzE(jet.px(), jet.py(), jet.pz(), jet.e());
        detJet->SetPhiEta(jet_phi, jet_eta); 
        detJet->SetArea(jet.area()); 
        recoLevelTracks.push_back(jet);
        fillIndex++;
      } 
      
      if (!isRecoil) {
        jet.set_user_index(12300 + ji); 
        recoLevelTracks.push_back(jet);

      }
      //cout << "   " << jet.user_index() << " " << detLevelJets.size() << endl;
    }
    fillIndex=0;

    //recoLevelTracks = JetEmbedding(tca_MB_tracks, detLevelJets);
    for (int jei = 0; jei < tca_MB_tracks->GetEntriesFast(); jei++){
      TParticle *mb_track = (TParticle*)tca_MB_tracks->At(jei);
      double track_eta = mb_track->Eta(); 
      double track_pt  = mb_track->Pt(); 
      if (TMath::Abs(track_eta) > 1.0) continue;
      if (track_pt < 0.2) continue;
      recoLevelTracks.push_back(PseudoJet(mb_track->Px(), mb_track->Py(), mb_track->Pz(), mb_track->Energy()));
    }

    ClusterSequenceArea csa_recoLevelJets(recoLevelTracks, jet_def, area_def);
    recoLevelJets = sorted_by_pt(csa_recoLevelJets.inclusive_jets()); 
    //cout << pltLevelJets.size() << " " << detLevelJets.size() << " " << tca_MB_tracks->GetEntriesFast() << " " << recoLevelTracks.size() << endl;
    //cout << "   " << detLevelJets.size() << " " << pltLevelJets.size() << " " << recoLevelJets.size() << endl;
    JetDefinition jet_def_bkgd(kt_algorithm, 0.4); 
    AreaDefinition area_def_bkgd(active_area_explicit_ghosts, GhostedAreaSpec(1.0, 1, 0.01)); 
    Selector selector = SelectorAbsEtaMax(1.0) * !SelectorNHardest(2) * SelectorPtMin(0.001); 
    JetMedianBackgroundEstimator bkgd_estimator(selector, jet_def_bkgd, area_def_bkgd); 
    bkgd_estimator.set_particles(recoLevelTracks);
    double rho = bkgd_estimator.rho(); 
    double rhoM = bkgd_estimator.rho_m(); 

    for (int ji=0; ji<recoLevelJets.size(); ji++){ 
      PseudoJet jet = recoLevelJets[ji];
      double jet_phi = jet.phi(); 
      double jet_eta = jet.eta(); 
      double pt = jet.pt(); 
      double area = jet.area(); 
      double ptc = pt - rho * area;
      double dphi = TVector2::Phi_0_2pi(trigPhi - jet_phi);
      vector<PseudoJet> consties = jet.constituents();
      int tmpIndex = -1;
      vector<int> priorIndexVector;
      vector<double> priorPtVector;
      for (int c = 0; c < consties.size(); c++) { 
        PseudoJet consti = consties[c];
        int index = consti.user_index(); 
        if (index > 0) {
          tmpIndex = index; 
          priorIndexVector.push_back(index); 
          priorPtVector.push_back(consti.pt());
          }
      }
      if(priorIndexVector.size() > 1) {
        cout << priorIndexVector.size() << endl;
        for (int ct = 0; ct < priorIndexVector.size(); ct++) {
            cout << " " << priorIndexVector[ct] << " " << priorPtVector[ct] << endl; 
            if (priorIndexVector[ct] > 12300 && priorIndexVector[ct] < 45600) {
              cout << "    crossCheck : " << detLevelJets[priorIndexVector[ct]- 12300].pt() << endl;
              }
          }
        }
      jet.set_user_index(tmpIndex); 
      if (!(dphi > 3*TMath::Pi()/4 && dphi < 5*TMath::Pi()/4)) continue;
      if (TMath::Abs(jet_eta) > 1.0 - jetRadius) continue;
      TrackLevelJetArray *recoJet = new ((*tca_recoLevelJets)[fillIndex]) TrackLevelJetArray();
      recoJet->SetPxPyPzE(jet.px(), jet.py(), jet.pz(), jet.e());
      recoJet->SetPhiEta(jet_phi, jet_eta); 
      recoJet->SetArea(jet.area()); 
      recoJet->SetPtc(ptc);
      recoJet->SetUserIndex(tmpIndex);
      //cout << "  input index : " << jet.user_index() << " result index : " << recoJet->userIndex()<< endl; 
      fillIndex++;
    }
    fillIndex=0;

    embeddedTree->Fill(); 
    tca_MB_tracks->Delete(); 
    tca_priorTracks->Delete(); 
    tca_pltLevelJets->Delete();
    tca_detLevelJets->Delete();
    tca_pltLevelJets->Delete();
    finMinBias->Close(); 
  }
} 

void finish(){ 
  fout->cd(); 
  //embeddedTree->Write();
  fout->Write(); 
  fout->Close(); 
}

void MinBiasEmbedding(){  

#ifdef LOCALTEST 
  cout << "SIBAL THIS IS FUNCKING LOCAL TEST" << endl;  
  init(-999, k510, kZr, kCent);
#endif 
  eventLoop();

  finish();

}


int main(){ 
  MinBiasEmbedding();
  return 0;
}
