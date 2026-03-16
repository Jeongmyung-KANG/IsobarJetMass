#define LOCALTEST

TFile *fin_pythia_fastSim; 
TFile *fout;
TFile *f_mb_summary;
TTree *summaryTree;

int cumEventNumber; 
int eventIndex;
int fileIndex; 
int eventClassNumber;

void init(int p6FileIndex){     

#ifdef LOCALTEST 
  TString p6FileName = "test.root";   
  fin_pythia_fastSim = TFile::Open(p6FileName); 
  f_mb_summary = TFile::Open(Form("/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/MB_summary.root"));
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

  //priorTree = (TTree*)fin_pythia_fastSim->Get("outtree"); 
  //if (!priorTree) {cout << "TREE IS FUNCKING EMPTY" << endl;}
} 

int MinBiasEventIndexExtraction(){
  int result = -999;
  
  int callIndex = gRandom->Uniform(0, summaryTree->GetEntries());
  summaryTree->GetEntry(callIndex-1); 
  cout << callIndex << " " << fileIndex << "  " << eventIndex << " " << cumEventNumber << " " << endl; 
  

  /*
  for (int i = 0; i < summaryTree->GetEntries(); i++){ 
    summaryTree->GetEntry(i);
    cout << i << " " << fileIndex << " " << eventIndex << " " << cumEventNumber << endl;   
  }
  */ 
  return result; 
}

void eventLoop(){ 
  TTree *priorTree = (TTree*)fin_pythia_fastSim->Get("outtree");  
  TClonesArray *tca_priorTracks = new TClonesArray("TParticle", 10000000);  
  TClonesArray *tca_MB_tracks = new TClonesArray("TParticle", 10000000); 

  priorTree->SetBranchAddress("tracks", &tca_priorTracks); 
  cout << priorTree->GetEntries() << endl; 

  for (int i = 0; i < priorTree->GetEntries(); i++) { 
    //int testMB_index = MinBiasEventIndexExtraction(); 
    priorTree->GetEntry(i); 
    int callIndex = gRandom->Uniform(0, summaryTree->GetEntries());
    summaryTree->GetEntry(callIndex-1); 
    //cout << eventClassNumber << " " << fileIndex << endl; 
    //cout << fileIndex << " " << eventIndex << endl;
    
    TString MinBiasName = Form("/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/%d/METracks_%d.root", eventClassNumber, fileIndex);
    TFile *finMinBias = TFile::Open(MinBiasName);
    TTree *MinBiasTree = (TTree*)finMinBias->Get("tree");
    MinBiasTree->SetBranchAddress("tracks", &tca_MB_tracks); 

    MinBiasTree->GetEntry(eventIndex);

    //for (int ei = 0; ei < tca_priorTracks->GetEntriesFast(); ei++) { 

    //}
    
    cout << MinBiasName << " " << fileIndex << " " << "nTracks of prior : " << tca_priorTracks->GetEntriesFast() << " nTracks of MB : " << tca_MB_tracks->GetEntries() << endl; 
    
    tca_priorTracks->Delete(); 
    tca_MB_tracks->Delete(); 
    finMinBias->Close(); 
  }
} 

void finish(){ 
  fout->cd(); 
  fout->Write(); 
  fout->Close(); 
}
void MinBiasEmbedding(){  

#ifdef LOCALTEST 
  cout << "SIBAL THIS IS FUNCKING LOCAL TEST" << endl;  
  init(-999);
#endif 
  eventLoop();
}
