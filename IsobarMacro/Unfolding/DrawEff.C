void DrawEff() {
    TFile *fin = TFile::Open("Zr_cent_embeddedTree.root");
    TH1F *fpt = (TH1F*)fin->Get("h_prior_pt");
    TH1F *hpt_fs = (TH1F*)fin->Get("h_prior_pt_fs");
    fpt->Rebin(4); 
    hpt_fs->Rebin(4);
    TH1F *eff = (TH1F*)hpt_fs->Clone("eff");
    eff->Divide(fpt);

    TCanvas *c = new TCanvas("c", "c", 500, 500); 
    eff->Draw();
    c->SaveAs("eff.pdf");
    
}