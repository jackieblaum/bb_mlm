#include "output.h"

void printRawData(hists_t hists){
  if(!(hists.dat_hist->Integral() + hists.dat_hist->Integral())) return;
  std::stringstream path;
  path << "raw_data_" << hists.outpath << ".csv";
  std::ofstream f(path.str().c_str());
  f << "MSW, di, bi, si" << std::endl;
  for(int i = 1; i <= NBIN; i++){
    f << hists.dat_hist->GetBinCenter(i) << ","
    << hists.dat_hist->GetBinContent(i) << ","
    << hists.bkg_hist->GetBinContent(i) << ","
    << hists.src_hist->GetBinContent(i) << std::endl;
  }
}

void histogram_raw_data(indices_t ins, hists_t hists){
  if(!(hists.dat_hist->Integral() + hists.bkg_hist->Integral())) return;
  std::stringstream filepath;
  filepath << "HIST_RAW_" << hists.outpath << ".png";

  //Copy the global histograms so that we can edit them
  TH1D* dathist = new TH1D(*hists.dat_hist);
  TH1D* bkghist = new TH1D(*hists.bkg_hist);

  //Set up
  TCanvas *c1 = new TCanvas("","",1200,1200);
  c1->cd();
  dathist->SetLineColor(4);
  bkghist->SetLineColor(6);

  dathist->SetStats(false);
  bkghist->SetStats(false);

  dathist->SetTitle(hists.outpath.c_str());

  bkghist->Scale(hists.dat_hist->Integral() / hists.bkg_hist->Integral());

  TLegend *legend;
  legend = new TLegend(0.12, 0.8, 0.3, 0.9);
  legend->AddEntry(dathist, "Data");
  legend->AddEntry(bkghist, "Background Template");

  dathist->SetMinimum(0);
  dathist->SetMaximum(std::max(dathist->GetMaximum(), bkghist->GetMaximum()) * 1.1);

  //Draw
  dathist->Draw();
  dathist->Draw("sameE0");
  bkghist->Draw("same");
  bkghist->Draw("sameE0");
  legend->Draw();

  //Save and clean up
  c1->SaveAs(filepath.str().c_str());
  c1->Clear();
  delete c1;
  delete dathist;
  delete bkghist;
  delete legend;
}

void histogram_fit_data(double fracs[6], indices_t ins, args_t *args, hists_t hists){
  if(!(hists.dat_hist->Integral() + hists.bkg_hist->Integral())) return;
  std::stringstream filepath;
  filepath << "HIST_FIT_" << hists.outpath << ".png";

  //Copy the global histograms so that we can edit them
  TH1D* dathist = new TH1D(*hists.dat_hist);
  TH1D* bkghist = new TH1D(*hists.bkg_hist);
  TH1D* srchist = new TH1D(*hists.src_hist);
  bkghist->Scale(hists.dat_hist->Integral() / hists.bkg_hist->Integral());
  srchist->Scale(hists.dat_hist->Integral() / hists.src_hist->Integral());

  //Canvas set up
  TH1D* F0 = new TH1D("F0Hist", "Std Fit", NBIN, MSWLOW, MSWHIGH);
  TH1D* F1 = new TH1D("F1Hist", "BB Fit", NBIN, MSWLOW, MSWHIGH);
  TH1D* B1 = new TH1D("B1Hist", "BB Fit", NBIN, MSWLOW, MSWHIGH);
  TCanvas *c1 = new TCanvas("",hists.outpath.c_str(),1600,1600);
  c1->Divide(2,2);

  dathist->SetLineColor(4);
  bkghist->SetLineColor(6);
  srchist->SetLineColor(1);
  F0->SetLineColor(8);
  F1->SetLineColor(8);
  B1->SetLineColor(6);

  dathist->SetStats(false);
  bkghist->SetStats(false);
  srchist->SetStats(false);
  F0->SetStats(false);
  F1->SetStats(false);
  B1->SetStats(false);

  TLegend *legend0 = new TLegend(0.15, 0.8, 0.25, 0.9);
  TLegend *legend1 = new TLegend(0.15, 0.8, 0.25, 0.9);

  //Draw Std Src Fit
  bkghist->Scale(fracs[0]);
  srchist->Scale(fracs[1]);
  src_noBB(fracs[0], fracs[1], false, F0);
  c1->cd(1);
  F0->Draw("hist");
  dathist->Draw("same");
  bkghist->Draw("same hist");
  srchist->Draw("same hist");

  F0->SetMinimum(0);
  F0->SetMaximum(std::max(
    std::max(dathist->GetMaximum(), F0->GetMaximum()),
                          std::max(srchist->GetMaximum(), bkghist->GetMaximum())
  ) * 1.1);

  legend0->AddEntry(F0, "fi");
  legend0->AddEntry(bkghist, "bi");
  legend0->AddEntry(dathist, "di");
  legend0->AddEntry(srchist, "Si");
  legend0->Draw();

  srchist->Scale(1/fracs[1]);

  //Draw BB Src
  srchist->Scale(fracs[3]);
  src_BB(fracs[2], fracs[3], false, F1, B1);
  B1->Scale(hists.dat_hist->Integral() / hists.bkg_hist->Integral());
  B1->Scale(fracs[2]);
  c1->cd(2);
  F1->Draw("hist");
  dathist->Draw("same");
  B1->Draw("same hist");
  srchist->Draw("same hist");

  F1->SetMinimum(0);
  F1->SetMaximum(std::max(
    std::max(dathist->GetMaximum(), F1->GetMaximum()),
                          std::max(srchist->GetMaximum(), B1->GetMaximum())
  ) * 1.1);

  legend1->AddEntry(F1, "fi");
  legend1->AddEntry(B1, "Bi");
  legend1->AddEntry(dathist, "di");
  legend1->AddEntry(srchist, "Si");
  legend1->Draw();

  //Draw Residual
  c1->cd(3);
  TRatioPlot* rp = new TRatioPlot(F1, hists.dat_hist, "diff");
  rp->Draw();

  //Write Data
  c1->cd(4);
  TPaveText *pt = new TPaveText(0, 0, 1, 1);
  std::stringstream line;
  pt->AddText(.05, .95, "Standard:")->SetTextAlign(12);
  line << "P_b = " << fracs[0];
  pt->AddText(.05, .85, line.str().c_str())->SetTextAlign(12);
  line.str("");
  line << "P_s = " << fracs[1];
  pt->AddText(.05, .8, line.str().c_str())->SetTextAlign(12);
  line.str("");
  line << "TS = " << -2 * (src_noBB(fracs[0], fracs[1]) - nosrc_noBB(fracs[4]));
  pt->AddText(.05, .75, line.str().c_str())->SetTextAlign(12);
  pt->AddText(.55, .95, "Barlow-Beeston:")->SetTextAlign(12);
  line.str("");
  line << "P_b = " << fracs[2];
  pt->AddText(.55, .85, line.str().c_str())->SetTextAlign(12);
  line.str("");
  line << "P_s = " << fracs[3];
  pt->AddText(.55, .8, line.str().c_str())->SetTextAlign(12);
  line.str("");
  line << "TS = " << -2 * (src_BB(fracs[2], fracs[3]) - nosrc_BB(fracs[5]));
  pt->AddText(.55, .75, line.str().c_str())->SetTextAlign(12);
  pt->AddLine(0, .7, 1, .7);
  pt->SetAllWith("=", "size", .05);
  if(args->op_info.c_str()){
    std::ifstream infile(args->op_info);
    std::string readline;
    std::getline(infile, readline);
    pt->AddText(.05, .6, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt->AddText(.05, .5, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt->AddText(.05, .4, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt->AddText(.05, .3, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt->AddText(.05, .2, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt->AddText(.05, .1, readline.c_str())->SetTextAlign(12);
    infile.close();
  }
  pt->Draw();

  //Save and Clean up
  c1->SaveAs(filepath.str().c_str());
  c1->Clear();
  delete c1;
  delete dathist;
  delete bkghist;
  delete srchist;
  delete F0;
  delete F1;
  delete B1;
  delete legend0;
  delete legend1;
}

void calculate_errors(double Pb_m, double Ps_m, double sigma_Pb_m, double sigma_Ps_m, indices_t ins, double alpha, hists_t hists){
  if(!(hists.dat_hist->Integral() + hists.bkg_hist->Integral())) return;
  double N_D = hists.dat_hist->Integral();
  double N_B = hists.bkg_hist->Integral();
  double B = alpha * N_B;
  double S = N_D - B;
  double P_S = S / N_D;
  double P_B = B / N_D;

  double sigma_N_D = TMath::Sqrt(hists.dat_hist->Integral());
  double sigma_B = TMath::Sqrt(alpha) * TMath::Sqrt(B);
  double sigma_S = TMath::Sqrt( TMath::Power(sigma_N_D, 2) + TMath::Power(sigma_B, 2) );
  double sigma_P_S = TMath::Abs(P_S) * TMath::Sqrt( TMath::Power(sigma_S / S, 2) + TMath::Power(sigma_N_D / N_D, 2) );
  double sigma_P_B = TMath::Abs(P_B) * TMath::Sqrt( TMath::Power(sigma_B / B, 2) + TMath::Power(sigma_N_D / N_D, 2) );

  std::stringstream filename;
  filename << "errors" << hists.outpath << ".csv";
  std::ofstream f(filename.str().c_str());
  f << "Minuit Values:" << std::endl
  << "Ps: " << Ps_m << std::endl
  << "sigma_Ps: " << sigma_Ps_m << std::endl
  << "Pb: " << Pb_m << std::endl
  << "sigma_Pb: " << sigma_Pb_m << std::endl;
  f << std::endl;
  f << "Calculated Values:" << std::endl
  << "Ps: " << P_S << std::endl
  << "sigma_Ps: " << sigma_P_S << std::endl
  << "Pb: " << P_B << std::endl
  << "sigma_Pb: " << sigma_P_B << std::endl;
  f << std::endl;
  f << "Relative Differences:" << std::endl
  << "Ps: " << TMath::Abs(P_S - Ps_m) / Ps_m << std::endl
  << "sigma_Ps: " << TMath::Abs(sigma_Ps_m - sigma_P_S) / sigma_Ps_m << std::endl
  << "Pb: " << TMath::Abs(P_B - Pb_m) / Pb_m << std::endl
  << "sigma_Pb: " << TMath::Abs(sigma_Pb_m - sigma_P_B) / sigma_Pb_m << std::endl;
  f << std::endl;
  f << "N_D: " << N_D << std::endl
  << "sigma_N_D: " << sigma_N_D << std::endl
  << "B: " << B << std::endl
  << "sigma_B: " << sigma_B << std::endl
  << "S: " << S << std::endl
  << "sigma_S: " << sigma_S << std::endl
  << "alpha: " << alpha << std::endl;

  f.close();
}

void print_cuts(std::string action, cuts_t* cuts, std::string outpath){
  if(!action.compare("reset")){
    std::ofstream f("cuts_data.txt");
    f << PRINTSPACE << "Bin"
    << PRINTSPACE << "passed"
    << PRINTSPACE << "read"
    << PRINTSPACE << "source"
    << PRINTSPACE << "telescope"
    << PRINTSPACE << "energy"
    << PRINTSPACE << "zenith angle"
    << PRINTSPACE << "msw"
    << PRINTSPACE << "azimuth"
    << PRINTSPACE << "offset" << std::endl;
    f.close();
    f.open("cuts_bkg.txt");
    f << PRINTSPACE << "Bin"
    << PRINTSPACE << "passed"
    << PRINTSPACE << "read"
    << PRINTSPACE << "source"
    << PRINTSPACE << "telescope"
    << PRINTSPACE << "energy"
    << PRINTSPACE << "zenith angle"
    << PRINTSPACE << "msw"
    << PRINTSPACE << "azimuth"
    << PRINTSPACE << "offset" << std::endl;
    f.close();
  }
  else if(!action.compare("data") || !action.compare("bkg")){
    std::stringstream fname;
    fname << "cuts_" << action << ".txt";
    std::ofstream f(fname.str().c_str(), std::ios::out | std::ios::app);
    f << PRINTSPACE << outpath
    << PRINTSPACE << cuts->passed
    << PRINTSPACE << cuts->read
    << PRINTSPACE << cuts->src
    << PRINTSPACE << cuts->tel
    << PRINTSPACE << cuts->e
    << PRINTSPACE << cuts->za
    << PRINTSPACE << cuts->msw
    << PRINTSPACE << cuts->az
    << PRINTSPACE << cuts->off << std::endl;
    f.close();
  }
}

void map_likelihood(double Pb, double Ps, std::string title_tag, indices_t ins, args_t args, hists_t hists){
  if(!(hists.dat_hist->Integral() + hists.bkg_hist->Integral())) return;
  int xbins = 100;
  int ybins = 100;
  //Find x limits such that xmax <= 1, xmin >= 0, and xmax - xmin = .3
  double xmax = (Pb + .15 > 1.0 ? 1.0 : Pb + .15);
  double xmin = Pb - .15 < 0.0 ? 0.0 : Pb - .15;
  if(xmax - xmin < .3){
    if(xmax == 1) xmin = .7;
    else if(xmin == 0) xmax = .3;
  }
  //Find y limits similarly
  double ymax = Ps + .15 > 1.0 ? 1.0 : Ps + .15;
  double ymin = Ps - .15 < 0.0 ? 0.0 : Ps - .15;
  if(ymax - ymin < .3){
    if(ymax == 1) ymin = .7;
    else if(ymin == 0) ymax = .3;
  }

  std::stringstream title;
  title << title_tag
  << " Likelihood ";
  if(args.bin_vars & 1) title << "ZA"<< ins.za;
  if(args.bin_vars & 2) title << "E" << ins.e;
  if(args.bin_vars & 4) title << "T" << ins.tel;
  if(args.bin_vars & 8) title << "A" << ins.az;
  if(args.bin_vars & 16) title << "O" << ins.off;

  gStyle->SetOptStat(0);
  TCanvas *c1 = new TCanvas("c1","c1", 1200,800);
  TH2D *map = new TH2D("map", title.str().c_str(), xbins, xmin, xmax, ybins, ymin, ymax);
  TAxis *xax = map->GetXaxis();
  TAxis *yax = map->GetYaxis();
  xax->SetTitle("Pb");
  yax->SetTitle("Ps");
  for(int j = 1; j <= ybins; j++){
    for(int i = 1; i <= xbins; i++){
      double Pb = xax->GetBinCenter(i);
      double Ps = yax->GetBinCenter(j);
      double lnL = -src_BB(Pb,Ps);
      map->SetBinContent(i, j, lnL);
    }
  }

  gStyle->SetPalette(kBlackBody);
  map->Draw("SURF3");
  std::stringstream out_file;
  out_file << boost::replace_all_copy(title.str(), " ", "_") << ".png";
  c1->SaveAs(out_file.str().c_str());

  //clean up
  delete c1;
  delete map;
}

void plot_msw_vs_msl(hists_t hists){
  hists.dat_2hist->GetXaxis()->SetTitle("MSW");
  hists.dat_2hist->GetYaxis()->SetTitle("MSL");
  hists.bkg_2hist->GetXaxis()->SetTitle("MSW");
  hists.bkg_2hist->GetYaxis()->SetTitle("MSL");
  double dat, bkg;
  dat = hists.dat_2hist->GetMaximum();
  bkg = hists.bkg_2hist->GetMaximum();
  hists.dat_2hist->SetMaximum(dat > bkg ? dat : bkg);
  hists.bkg_2hist->SetMaximum(dat > bkg ? dat : bkg);
  dat = hists.dat_2hist->GetMinimum();
  bkg = hists.bkg_2hist->GetMinimum();
  hists.dat_2hist->SetMinimum(dat < bkg ? dat : bkg);
  hists.bkg_2hist->SetMinimum(dat < bkg ? dat : bkg);

  std::stringstream title;
  title << hists.dat_2hist->GetTitle() << " " << hists.outpath;
  hists.dat_2hist->SetTitle(title.str().c_str());
  title.str("");
  title << hists.bkg_2hist->GetTitle() << " " << hists.outpath;
  hists.bkg_2hist->SetTitle(title.str().c_str());

  gStyle->SetPalette(kBlackBody);
  gStyle->SetOptStat(0);

  TCanvas c1("c1", "c1", 2400, 1200);
  c1.Divide(2,1);
  c1.cd(1);
  hists.dat_2hist->Draw("COLZ");

  c1.cd(2);
  hists.bkg_2hist->Draw("COLZ");

  std::stringstream out_file;
  out_file << "MSWMSL_" << hists.outpath << ".png";
  c1.SaveAs(out_file.str().c_str());
}