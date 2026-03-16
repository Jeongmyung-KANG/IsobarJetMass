#define LOCALTEST
#include <filesystem>
TFile *fin;
TFile *fout; 
TTree *MB_tree; 
TTree *outtree;
TString MB_name; 

namespace fs = std::filesystem;

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

#ifdef LOCALTEST
    cout << "--------------WARNING-------------------" << endl;
    cout << "-                                       " << endl; 
    cout << "-       this is fucking local test please change your fucking status                                " << endl;  
    cout << "-                                       " << endl; 
    cout << "-                                       " << endl; 
    cout << "--------------WARNING-------------------" << endl;
#endif


  int inputIndex = 1; 
  int cumEventNumber = 0;  
  int eventIndex; 
  int fileIndex; 

  fout = new TFile(Form("MB_summary.root"), "RECREATE"); 
  outtree = new TTree("summaryTree", "summaryTree"); 
  outtree->Branch("cumEventNumber", &cumEventNumber); 
  outtree->Branch("eventIndex", &eventIndex);
  outtree->Branch("fileIndex", &fileIndex); 

  
  for (int i = 0; i < 640; i++){ 
    
    string path; 
    #ifdef LOCALTEST 
    path = "/Users/gangjeongmyeong/Star/IsobarMacro/Unfolding/"; 
    #endif

    TString dir = Form("%d", i);
    path += dir.Data();  
    
    if (!fs::exists(path)) {
      cout << path << " " << "dir deos not exist" << endl;
      continue;
    }

    for (const auto& entry : fs::directory_iterator(path)){  
      TString MB_rootFileName = TString(entry.path());  
      
    }

  }

  finish(); 

 }
