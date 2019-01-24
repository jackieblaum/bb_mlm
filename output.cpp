#include "output.h"

void init_output_root_file(){
  TFile *fout = new TFile("output.root", "NEW");
  if(!fout->IsOpen()){
    std::cerr << "Failed to init output file." << std::endl;
  }
  TDirectory *dir_src_excl = fout->mkdir("Source Excl");
  dir_src_excl->mkdir("MSW");
  dir_src_excl->mkdir("BDT");
  TDirectory *dir_all = fout->mkdir("All");
  dir_all->mkdir("MSW");
  dir_all->mkdir("BDT");
  fout->Write();
  delete fout;
}


/**
  Outputs a bin's data to the output root file.
  @param args Current arguments struct.
  @param hists Current hists struct.
  @param fracs Pointer to start of array of
  background and source fractions. The order
  should be:
    bkgfrac_src_nobb
    srcfrac_src_nobb
    bkgfrac_src_bb
    srcfrac_src_bb
  @param fit_param Which parameter is being fit on.
  Should have the value "MSW" or "BDT".
  @param srcexcl Whether sources were excluded.
*/
void write_to_root_file(args_t *args, hists_t *hists, double *fracs, std::string fit_param, bool srcexcl){
  TFile *fout = new TFile("output.root", "UPDATE");
  if (!fout->IsOpen()){
    std::cerr << "Failed to open output file." << std::endl;
  }
  TDirectory *tld;
  if(srcexcl) tld = (TDirectory*)fout->Get("Source Excl");
  else tld = (TDirectory*)fout->Get("All");
  TDirectory *paramdir = (TDirectory*)tld->Get(fit_param.c_str());
  TDirectory *maindir = paramdir->mkdir(hists->longoutpath.c_str());

  maindir->cd();
  //Write input histograms
  double parlow, parhigh;
  if(fit_param == "MSW"){
    hists->msw_dat->Write("Data");
    hists->msw_bkg->Write("Background");
    hists->msw_src->Write("Source");
    parlow = MSWLOW; parhigh = MSWHIGH;
  }
  else if(fit_param == "BDT"){
    hists->bdt_dat->Write("Data");
    hists->bdt_bkg->Write("Background");
    hists->bdt_src->Write("Source");
    parlow = BDTLOW; parhigh = BDTHIGH;
  }
  TH1D* nobb_F = new TH1D("nobb_F", "Std Fit", NBIN, parlow, parhigh);
  TH1D* bb_F = new TH1D("bb_F", "BB Fit Data", NBIN, parlow, parhigh);
  TH1D* bb_B = new TH1D("bb_B", "BB Fit Bkg", NBIN, parlow, parhigh);

  src_noBB(fracs[0], fracs[1], nobb_F);
  src_BB(fracs[2], fracs[3], bb_F, bb_B);

  nobb_F->Write();
  bb_F->Write();
  bb_B->Write();

  delete fout;
}
