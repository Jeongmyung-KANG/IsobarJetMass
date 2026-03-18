#include <iostream>
#include "fastjet/ClusterSequenceArea.hh"  // use this instead of the "usual" ClusterSequence to get area support
#include "fastjet/PseudoJet.hh"
#include "fastjet/ClusterSequenceArea.hh"
#include "fastjet/Selector.hh"
#include "fastjet/tools/JetMedianBackgroundEstimator.hh"
#include "fastjet/tools/Subtractor.hh"
#include "TFile.h"
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

double params_zr_cent_pos[9] = {2.269, 1.407, 0.06189, 2.259,  -2.33,    1.16,   0.02825, -2.066,   3.059};
double params_zr_cent_neg[9] = {2.942, 2.079, 0.0443,  2.373,  -2.982,   1.682,  0.2354,  -0.5651,  1.275};
double params_zr_peri_neg[9] = {4.296, 3.406, 0.02562, 0.2758, -0.06651, 0.6565, 0.102,   -0.6077,  1.084};
double params_zr_peri_pos[9] = {2.543, 1.653, 0.04662, 3.64,   0.7146,   0.7392, 3.558,    0.7376,  0.7321};

TH1F *h_prior_pt_fs;
TH1F *h_prior_pt;
TH1F *h_prior_phi;
TH1F *h_prior_eta;

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

TClonesArray *tca_priorJets;
TClonesArray *tca_embeddedTracks; 

int cumEventNumber; 
int eventIndex;
int fileIndex; 
int eventClassNumber;

double trigPhi;
double trigPt; 

void init(int p6FileIndex, int kSys, int kCentrality){     

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
    TString p6FileName = "test.root";   
    fin_pythia_fastSim = TFile::Open(p6FileName); 
    f_mb_summary = TFile::Open(Form("/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/Embedding/MB_summary.root"));
    summaryTree = (TTree*)f_mb_summary->Get("summaryTree");
    fout = new TFile(Form("%s_%s_embeddedTree.root", SysName[kSys].Data(), CentralityName[kCentrality].Data()), "RECREATE");

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
    embeddedTree = new TTree("embeddedTree", "embeddedTree"); 
    tca_embeddedTracks = new TClonesArray("TParticle", 1000000); 
    tca_priorJets = new TClonesArray("TParticle", 1000000); 
    //embeddedTree->Branch("tracks", "TClonesArray", &tca_embeddedTracks);
    embeddedTree->Branch("priorJets", "TClonesArray", &tca_priorJets); 
    embeddedTree->Branch("triggerPt", &trigPt);
    embeddedTree->Branch("triggerPhi", &trigPhi);

    //priorTree = (TTree*)fin_pythia_fastSim->Get("outtree"); 
    //if (!priorTree) {cout << "TREE IS FUNCKING EMPTY" << endl;}
    h_prior_pt_fs = new TH1F("h_prior_pt_fs", "h_prior_pt_fs", 1000, 0, 20);
    h_prior_pt    = new TH1F("h_prior_pt",    "h_prior_pt"   , 1000, 0, 20); 
    h_prior_phi    = new TH1F("h_prior_phi",    "h_prior_phi"   , 1000, -10, 10); 
    h_prior_eta    = new TH1F("h_prior_eta",    "h_prior_eta"   , 1000, -5, 5); 

    h_prior_pt_fs->Sumw2(); 
    h_prior_pt->Sumw2(); 
    h_prior_phi->Sumw2(); 
    h_prior_eta->Sumw2();
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
  TTree *priorTree = (TTree*)fin_pythia_fastSim->Get("outtree");  
  TClonesArray *tca_priorTracks = new TClonesArray("TParticle", 10000000);  
  TClonesArray *tca_MB_tracks = new TClonesArray("TParticle", 10000000); 

  priorTree->SetBranchAddress("tracks", &tca_priorTracks); 
  //cout << priorTree->GetEntries() << endl; 
  
  

  //int nPriorEvents = priorTree->GetEntries();
  int nPriorEvents = 5000;
  cout << "tot event : " << priorTree->GetEntries() << endl;
  for (int i = 0; i < nPriorEvents; i++) { 
    if (i % 1000 ==0) cout << "event : " << i << endl; 
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
    
    int fillIndex = 0;

    vector<PseudoJet> vec_priorTracks;
    vector<PseudoJet> vec_embeddedJets;

    vector<double> vec_trigPhi;
    vector<double> vec_trigPt;

    if (tca_priorTracks->GetEntriesFast() == 0) continue;

    for (int ei = 0; ei < tca_priorTracks->GetEntriesFast(); ei++) { 
      TParticle *priorTrack = (TParticle*)tca_priorTracks->At(ei);
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

      if (!isPassFastSim) continue;

      h_prior_pt_fs->Fill(priorTrack->Pt());
      PseudoJet track = PseudoJet(priorTrack->Px(), priorTrack->Py(), priorTrack->Pz(), priorTrack->Energy());
      vec_priorTracks.push_back(track); 
      //cout << fillIndex << "/" << tca_priorTracks->GetEntriesFast() +  tca_MB_tracks->GetEntries() << " prior tracks : " << priorTrack->Pt() << endl;  
    }
    
    if (vec_trigPt.size() == 0) continue;
    int triggerTrackIndex = gRandom->Uniform(vec_trigPt.size());
    trigPt = vec_trigPt[triggerTrackIndex]; 
    trigPhi = vec_trigPhi[triggerTrackIndex];
    //cout << trigPt << " " << trigPhi << " " << triggerTrackIndex << " " << vec_trigPt.size() << endl;
    JetDefinition   jet_def(antikt_algorithm, jetRadius);
    GhostedAreaSpec  area_spec(1.0);
    AreaDefinition  area_def(active_area, GhostedAreaSpec(1.0, 1, 0.01));

    ClusterSequenceArea csa_priorJets(vec_priorTracks, jet_def, area_def); 
    vector<PseudoJet> priorJets = sorted_by_pt(csa_priorJets.inclusive_jets());

    for (int ei = 0; ei < priorJets.size(); ei++) {
      PseudoJet priorJet = priorJets[ei];
      double jet_eta = priorJet.eta(); 
      TParticle *priorJet_as_track = new((*tca_priorJets)[ei]) TParticle();
      priorJet_as_track->SetMomentum(priorJet.px(), priorJet.py(), priorJet.pz(), priorJet.e());

      if (TMath::Abs(jet_eta) > 1.0 - jetRadius) continue;

      priorJet.set_user_index(981223); 
      vec_embeddedJets.push_back(priorJet); 
      TParticle *jet_as_track = new((*tca_embeddedTracks)[fillIndex]) TParticle();
      jet_as_track->SetMomentum(priorJet.px(), priorJet.py(), priorJet.pz(), priorJet.e());
      jet_as_track->SetFirstMother(981223);
      jet_as_track->SetPdgCode(981223);

      fillIndex++;

    }


    for (int ei = 0; ei < tca_MB_tracks->GetEntriesFast(); ei++) {
      TParticle *MbTracks = (TParticle*)tca_MB_tracks->At(ei);
      TParticle *e_mb_track = new((*tca_embeddedTracks)[fillIndex]) TParticle();
      e_mb_track->SetMomentum(MbTracks->Px(), MbTracks->Py(), MbTracks->Pz(), MbTracks->Energy());
      //vec_embeddedJets.push_back(MbTracks->Px(), MbTracks->Py(), MbTracks->Pz(), MbTracks.Energy()); 
      //cout << fillIndex << "/" << tca_priorTracks->GetEntriesFast() +  tca_MB_tracks->GetEntries() << " mb tracks : " << MbTracks->Pt() << endl; 
      fillIndex++; 
    }
    
    //cout << priorJets.size() << " " << tca_MB_tracks->GetEntriesFast() << " " << vec_embeddedJets.size() << endl; 
    //cout << MinBiasName << " " << fileIndex << " " << "nTracks of prior : " << tca_priorTracks->GetEntriesFast() << " nTracks of MB : " << tca_MB_tracks->GetEntries() << endl; 

    embeddedTree->Fill(); 
    tca_priorJets->Clear();
    tca_embeddedTracks->Clear(); 
    tca_priorTracks->Clear(); 
    tca_MB_tracks->Clear(); 
    finMinBias->Close(); 
  }
} 

void finish(){ 
  fout->cd(); 
  embeddedTree->Write();
  //fout->Write(); 
  fout->Close(); 
}

void MinBiasEmbedding(){  

#ifdef LOCALTEST 
  cout << "SIBAL THIS IS FUNCKING LOCAL TEST" << endl;  
  init(-999, 0, 0);
#endif 
  eventLoop();

  finish();

}


int main(){ 
  MinBiasEmbedding();
  return 0;
}
