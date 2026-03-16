TFile *fin;
TFile *fout; 
TTree *MB_tree; 
TTree *outtree;
TString MB_name; 

void init(int fileIndex){ 
  MB_name = Form("/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/461/METracks_%d.root", fileIndex); 
  cout << MB_name << endl;
  fin =  TFile::Open(MB_name);   
  if (!fin) cout << "_______________________________________________-file was not opend" << endl; 
  MB_tree = (TTree*)fin->Get("tree"); 
}

void finish(){
  fout->cd(); 
  outtree->Write();
  fout->Write(); 
  fout->Close(); 
}
 
void DataScanning(){  

  int inputIndex = 1; 
  int cumEventNumber = 0;  
  int eventIndex; 
  int fileIndex; 

  fout = new TFile(Form("MB_summary.root"), "RECREATE"); 
  outtree = new TTree("summaryTree", "summaryTree"); 
  outtree->Branch("cumEventNumber", &cumEventNumber); 
  outtree->Branch("eventIndex", &eventIndex);
  outtree->Branch("fileIndex", &fileIndex); 

  for (int i = 0; i < 10000; i++){ 
    init(i);   
    
    cout << MB_tree->GetEntries() << " " << cumEventNumber <<endl;
    
    for (int e = 0; e < MB_tree->GetEntries(); e++){   
      cout << MB_name << " " << i << " " << cumEventNumber << " " << e << endl; 
      eventIndex = e;
      fileIndex = i;
      cumEventNumber ++; 
      outtree->Fill(); 
    }

    fin->Close(); 
  }
  
  finish(); 

 }
