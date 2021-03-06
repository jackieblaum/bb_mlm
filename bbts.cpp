#include "bbts.h"
#include "load_data.h"
#include "output.h"

std::string OUTPATH;
std::string LONGOUTPATH;

TH1D* DAT_HIST;
TH1D* BKG_HIST;
TH1D* SRC_HIST;
TH2D* DAT_2HIST;
TH2D* BKG_2HIST;

//Bin boundaries
double ZABINS[] = {0.00,14.1,25.46,32.59,37.57,42.56,47.55};
double EBINS[] = {316,630,1259,2512,5012};
int TBINS[] = {3,4};
double AZBINS[] = {0,45,90,135,180,225,270,315,360};
double OBINS[] = {0.0, 1.0, 1.4, 1.7, 2.0};


//helper function declarations
int parse_command_line(int argc, char* argv[], args_t* args);
void prepare_std_output_files(args_t args);
int optional_binning(indices_t indices, args_t args);

//Li Ma Significance Calculation
//Eq 17, Li Ma (1983)
double lima_sig(double Pb, double Ps){
  double alpha = DAT_HIST->Integral() / BKG_HIST->Integral();
  double pb = Pb * DAT_HIST->Integral() / BKG_HIST->Integral();
  double ps = Ps * DAT_HIST->Integral() / SRC_HIST->Integral();
  double N_on = ps * SRC_HIST->Integral();
  double N_off = pb * BKG_HIST->Integral();
  double S = TMath::Sqrt(2)
           * TMath::Sqrt(
              N_on * TMath::Log( ((1 + alpha) / alpha) * (N_on / (N_on + N_off)) )
            + N_off * TMath::Log( (1 + alpha) * (N_off / (N_on + N_off)) )
            );
  return S;
}

double nosrc_noBB(double Pb, bool print, TH1D* F){
  int i;
  double pb;
  double di, bi, fi;
  double dtot = 0, btot = 0, ftot = 0;
  std::ofstream f;
  double lnL = 0;
  pb = Pb * DAT_HIST->Integral() / BKG_HIST->Integral();
  if(print){
    std::stringstream path;
    path << "Bin_NoSrc_Std_" << OUTPATH << ".txt";
    f.open(path.str());
    f.precision(5);
    f << "Bin: " << LONGOUTPATH << std::endl;
    f << std::scientific;
    f << "bkgfrac    " << Pb << std::endl;
    f << "srcfrac    -----" << std::endl;
    f << PRINTSPACE << "Counts:";
    f << PRINTSPACE << "Data";
    f << PRINTSPACE << "Bkg";
    f << std::endl << std::defaultfloat;
    f << PRINTSPACE << "";
    f << PRINTSPACE << DAT_HIST->Integral();
    f << PRINTSPACE << BKG_HIST->Integral();
    f << std::endl << std::endl;
    f << PRINTSPACE << "i";
    f << PRINTSPACE << "di";
    f << PRINTSPACE << "fi";
    f << PRINTSPACE << "bi" << std::endl;
  }
  for(i = 1; i <= NBIN; i++){
    di = DAT_HIST->GetBinContent(i);
    bi = BKG_HIST->GetBinContent(i);
    fi = pb * bi;
    if(di == 0) lnL -= fi;
    else        lnL += di * TMath::Log(fi) - fi;
    dtot += di; btot += bi; ftot += fi;
    if(print){
      f << PRINTSPACE << i;
      f << PRINTSPACE << di;
      f << PRINTSPACE << fi;
      f << PRINTSPACE << bi << std::endl;
    }
    if(F){
      F->Sumw2(false);
      F->SetBinContent(i, fi);
      F->Sumw2(true);
    }
  }
  if(print){
    f << PRINTSPACE << "Sum";
    f << PRINTSPACE << dtot;
    f << PRINTSPACE << ftot;
    f << PRINTSPACE << btot << std::endl;
    f.close();
  }
  return -lnL;
}

double src_noBB(double Pb, double Ps, bool print, TH1D* F){
  int i;
  double pb, ps;
  double di, bi, si, fi;
  double dtot = 0, btot = 0, ftot = 0, stot = 0;
  std::ofstream f;
  double lnL = 0;
  pb = Pb * DAT_HIST->Integral() / BKG_HIST->Integral();
  ps = Ps * DAT_HIST->Integral() / SRC_HIST->Integral();
  if(print){
    std::stringstream path;
    path << "Bin_Src_Std_" << OUTPATH << ".txt";
    f.open(path.str());
    f.precision(5);
    f << "Bin: " << LONGOUTPATH << std::endl;
    f << std::scientific;
    f << "bkgfrac    " << Pb << std::endl;
    f << "srcfrac    " << Ps << std::endl;
    f << PRINTSPACE << "Counts:";
    f << PRINTSPACE << "Data";
    f << PRINTSPACE << "Bkg";
    f << PRINTSPACE << "Src";
    f << std::endl << std::defaultfloat;
    f << PRINTSPACE << "";
    f << PRINTSPACE << DAT_HIST->Integral();
    f << PRINTSPACE << BKG_HIST->Integral();
    f << PRINTSPACE << SRC_HIST->Integral();
    f << std::endl << std::endl;
    f << PRINTSPACE << "i";
    f << PRINTSPACE << "di";
    f << PRINTSPACE << "fi";
    f << PRINTSPACE << "bi";
    f << PRINTSPACE << "si" << std::endl;
  }
  for(i = 1; i <= NBIN; i++){
    di = DAT_HIST->GetBinContent(i);
    bi = BKG_HIST->GetBinContent(i);
    si = SRC_HIST->GetBinContent(i);
    fi = pb * bi + ps * si;
    if(di == 0) lnL -= fi;
    else        lnL += di * TMath::Log(fi) - fi;

    dtot += di; btot += bi; ftot += fi; stot += si;
    if(print){
      f << PRINTSPACE << i;
      f << PRINTSPACE << di;
      f << PRINTSPACE << fi;
      f << PRINTSPACE << bi;
      f << PRINTSPACE << si << std::endl;
    }
    if(F){
      F->Sumw2(false);
      F->SetBinContent(i, fi);
      F->Sumw2(true);
    }
  }
  if(print){
    f << PRINTSPACE << "Sum";
    f << PRINTSPACE << dtot;
    f << PRINTSPACE << ftot;
    f << PRINTSPACE << btot;
    f << PRINTSPACE << stot << std::endl;
    f.close();
  }
  return -lnL;
}

double nosrc_BB(double Pb, bool print, TH1D* F, TH1D* B){
  int i;
  double pb;
  double di, bi, Bi, fi;
  double Y, Z, ti;
  double dtot = 0, btot = 0, ftot = 0, Btot = 0;
  std::ofstream f;
  double lnL = 0;
  pb = Pb * DAT_HIST->Integral() / BKG_HIST->Integral();
  if(print){
    std::stringstream path;
    path << "Bin_NoSrc_BB_" << OUTPATH << ".txt";
    f.open(path.str());
    f.precision(5);
    f << "Bin: " << LONGOUTPATH << std::endl;
    f << std::scientific;
    f << "bkgfrac    " << Pb << std::endl;
    f << "srcfrac    -----" << std::endl;
    f << PRINTSPACE << "Counts:";
    f << PRINTSPACE << "Data";
    f << PRINTSPACE << "Bkg";
    f << std::endl << std::defaultfloat;
    f << PRINTSPACE << "";
    f << PRINTSPACE << DAT_HIST->Integral();
    f << PRINTSPACE << BKG_HIST->Integral();
    f << std::endl << std::endl;
    f << PRINTSPACE << "i";
    f << PRINTSPACE << "di";
    f << PRINTSPACE << "fi";
    f << PRINTSPACE << "bi";
    f << PRINTSPACE << "Bi" << std::endl;
  }
  for(i = 1; i <= NBIN; i++){
    di = DAT_HIST->GetBinContent(i);
    bi = BKG_HIST->GetBinContent(i);

    Y = pb * di + pb * bi;
    Z = di - pb * bi;
    ti = - Z / Y;
    if(di == 0) ti = 1; //because floating point
    Bi = bi / (pb * ti + 1);
    //Check for zero in background
    if(bi == 0 && (di / (1 + pb)) > 0){
      ti = - 1 / pb;
      Bi = di / (1 + pb);
    }
    fi = pb * Bi;
    if(di == 0) lnL -= fi;
    else        lnL += di * TMath::Log(fi) - fi;
    if(bi == 0) lnL -= Bi;
    else        lnL += bi * TMath::Log(Bi) - Bi;

    dtot += di; btot += bi; ftot += fi; Btot += Bi;
    if(print){
      f << PRINTSPACE << i;
      f << PRINTSPACE << di;
      f << PRINTSPACE << fi;
      f << PRINTSPACE << bi;
      f << PRINTSPACE << Bi << std::endl;
    }
    if(F){
      F->Sumw2(false);
      F->SetBinContent(i, fi);
      F->Sumw2(true);
    }
    if(B){
      B->Sumw2(false);
      B->SetBinContent(i, Bi);
      B->Sumw2(true);
    }
  }
  if(print){
    f << PRINTSPACE << "Sum";
    f << PRINTSPACE << dtot;
    f << PRINTSPACE << ftot;
    f << PRINTSPACE << btot;
    f << PRINTSPACE << Btot << std::endl;
    f.close();
  }
  return -lnL;
}

double src_BB(double Pb, double Ps, bool print, TH1D* F, TH1D* B){
  int i;
  double pb, ps;
  double di, bi, si, Bi, fi;
  double dtot = 0, btot = 0, ftot = 0, stot = 0, Btot = 0;
  std::ofstream f;
  double X, Y, Z, ti;
  double lnL = 0;
  pb = Pb * DAT_HIST->Integral() / BKG_HIST->Integral();
  ps = Ps * DAT_HIST->Integral() / SRC_HIST->Integral();
  if(print){
    std::stringstream path;
    path << "Bin_Src_BB_" << OUTPATH << ".txt";
    f.open(path.str());
    f.precision(5);
    f << "Bin: " << LONGOUTPATH << std::endl;
    f << std::scientific;
    f << "bkgfrac    " << Pb << std::endl;
    f << "srcfrac    " << Ps << std::endl;
    f << PRINTSPACE << "Counts:";
    f << PRINTSPACE << "Data";
    f << PRINTSPACE << "Bkg";
    f << PRINTSPACE << "Src";
    f << std::endl << std::defaultfloat;
    f << PRINTSPACE << "";
    f << PRINTSPACE << DAT_HIST->Integral();
    f << PRINTSPACE << BKG_HIST->Integral();
    f << PRINTSPACE << SRC_HIST->Integral();
    f << std::endl << std::endl;
    f << PRINTSPACE << "i";
    f << PRINTSPACE << "di";
    f << PRINTSPACE << "fi";
    f << PRINTSPACE << "bi";
    f << PRINTSPACE << "Bi";
    f << PRINTSPACE << "si" << std::endl;
  }
  for(i = 1; i <= NBIN; i++){
    di = DAT_HIST->GetBinContent(i);
    bi = BKG_HIST->GetBinContent(i);
    si = SRC_HIST->GetBinContent(i);

    X = pb * ps * si;
    Y = pb * di + pb * bi + ps * si - pb * ps * si;
    Z = di - pb * bi - ps * si;
    ti = (-Y + TMath::Sqrt( TMath::Power(Y, 2) - 4 * X * Z)) / (2 * X);
    if(di == 0) ti = 1; //because floating point
    Bi = bi / (pb * ti + 1);
    //Check for zero in background
    if(bi == 0 && pb > ps && (di / (1 + pb) - (ps * si) / (pb - ps)) > 0){
      ti = - 1 / pb;
      Bi = di / (1 + pb) - (ps * si) / (pb - ps);
    }
    //Check for zero in source
    if(si == 0 && ps > pb && (di / (1 + ps) - (pb * bi) / (ps - pb)) > 0){
      ti = - 1 / ps;
      Bi = bi / (pb * ti + 1);
    }

    fi = pb * Bi + ps * si;
    if(di == 0) lnL -= fi;
    else        lnL += di * TMath::Log(fi) - fi;
    if(bi == 0) lnL -= Bi;
    else        lnL += bi * TMath::Log(Bi) - Bi;

    dtot += di; btot += bi; ftot += fi; stot += si; Btot += Bi;
    if(print){
      f << PRINTSPACE << i;
      f << PRINTSPACE << di;
      f << PRINTSPACE << fi;
      f << PRINTSPACE << bi;
      f << PRINTSPACE << Bi;
      f << PRINTSPACE << si << std::endl;
    }
    if(F){
      F->Sumw2(false);
      F->SetBinContent(i, fi);
      F->Sumw2(true);
    }
    if(B){
      B->Sumw2(false);
      B->SetBinContent(i, Bi);
      B->Sumw2(true);
    }
  }
  if(print){
    f << PRINTSPACE << "Sum";
    f << PRINTSPACE << dtot;
    f << PRINTSPACE << ftot;
    f << PRINTSPACE << btot;
    f << PRINTSPACE << Btot;
    f << PRINTSPACE << stot << std::endl;
    f.close();
  }
  return -lnL;
}

void wrapper_nosrc_noBB(Int_t &nDim, Double_t *gout, Double_t &result, Double_t par[], Int_t flag){ result = nosrc_noBB(par[0]); }

void wrapper_src_noBB(Int_t &nDim, Double_t *gout, Double_t &result, Double_t par[], Int_t flag){ result = src_noBB(par[0],par[1]); }

void wrapper_nosrc_BB(Int_t &nDim, Double_t *gout, Double_t &result, Double_t par[], Int_t flag){ result = nosrc_BB(par[0]); }

void wrapper_src_BB(Int_t &nDim, Double_t *gout, Double_t &result, Double_t par[], Int_t flag){ result = src_BB(par[0], par[1]); }

void fit(indices_t ins, args_t args, double alpha, double *fracs, std::string fit_param, hists_t *hists=0){
  //Set up fitters
  TFitter* fit_nosrc_nobb = new TFitter(1);
  TFitter* fit_src_nobb   = new TFitter(2);
  TFitter* fit_nosrc_bb   = new TFitter(1);
  TFitter* fit_src_bb     = new TFitter(2);
  //Set Printout
  double p1 = args.verbosity;
  fit_nosrc_nobb->ExecuteCommand("SET PRINTOUT", &p1, 1);
  fit_src_nobb  ->ExecuteCommand("SET PRINTOUT", &p1, 1);
  fit_nosrc_bb  ->ExecuteCommand("SET PRINTOUT", &p1, 1);
  fit_src_bb    ->ExecuteCommand("SET PRINTOUT", &p1, 1);
  //Set fcn function
  fit_nosrc_nobb->SetFCN(wrapper_nosrc_noBB);
  fit_src_nobb  ->SetFCN(wrapper_src_noBB);
  fit_nosrc_bb  ->SetFCN(wrapper_nosrc_BB);
  fit_src_bb    ->SetFCN(wrapper_src_BB);
  //Set parameters
  fit_nosrc_nobb->SetParameter(0, "bkgFrac", .5, .5, 0, 1);
  fit_src_nobb  ->SetParameter(0, "bkgFrac", .5, .5, 0, 1);
  fit_nosrc_bb  ->SetParameter(0, "bkgFrac", .5, .5, 0, 1);
  fit_src_bb    ->SetParameter(0, "bkgFrac", .5, .5, 0, 1);
  fit_src_nobb  ->SetParameter(1, "srcFrac", .5, .5, 0, 1);
  fit_src_bb    ->SetParameter(1, "srcFrac", .5, .5, 0, 1);
  //Run MIGRAD
  fit_nosrc_nobb->ExecuteCommand("MIGRAD", 0, 0);
  fit_src_nobb  ->ExecuteCommand("MIGRAD", 0, 0);
  fit_nosrc_bb  ->ExecuteCommand("MIGRAD", 0, 0);
  fit_src_bb    ->ExecuteCommand("MIGRAD", 0, 0);
  //Run MINOS
  fit_nosrc_nobb->ExecuteCommand("MINOS", 0, 0);
  fit_src_nobb  ->ExecuteCommand("MINOS", 0, 0);
  fit_nosrc_bb  ->ExecuteCommand("MINOS", 0, 0);
  fit_src_bb    ->ExecuteCommand("MINOS", 0, 0);

  //Get likelihood and TS
  bool output_bins = (args.output & 1) && (DAT_HIST->Integral() + BKG_HIST->Integral());
  double lnL_nosrc_nobb = -nosrc_noBB(fit_nosrc_nobb->GetParameter(0), output_bins);
  double lnL_src_nobb = -src_noBB(fit_src_nobb->GetParameter(0), fit_src_nobb->GetParameter(1), output_bins);
  double lnL_nosrc_bb = -nosrc_BB(fit_nosrc_bb->GetParameter(0), output_bins);
  double lnL_src_bb = -src_BB(fit_src_bb->GetParameter(0), fit_src_bb->GetParameter(1), output_bins);
  double TS_nobb = 2 * (lnL_src_nobb - lnL_nosrc_nobb);
  double TS_bb = 2 * (lnL_src_bb - lnL_nosrc_bb);
  double lima_nobb = lima_sig(fit_src_nobb->GetParameter(0), fit_src_nobb->GetParameter(1));
  double lima_bb = lima_sig(fit_src_bb->GetParameter(0), fit_src_bb->GetParameter(1));

  std::stringstream std_output, bb_output, summary;
  std_output << "fitstats_" << fit_param << "_std.csv";
  bb_output << "fitstats_" << fit_param << "_bb.csv";
  summary << "summary_" << fit_param << ".csv";
  std::ofstream f(std_output.str(), std::ios::out | std::ios::app);
  if(args.bin_vars & 1) f << ins.za << ",";
  if(args.bin_vars & 2) f << ins.e << ",";
  if(args.bin_vars & 4) f << ins.tel << ",";
  if(args.bin_vars & 8) f << ins.az << ",";
  if(args.bin_vars & 16) f << ins.off << ",";
  f << std::scientific
    << fit_nosrc_nobb->GetParameter(0) << ","
    << fit_src_nobb->GetParameter(0) << ","
    << fit_src_nobb->GetParameter(1) << ","
    << std::defaultfloat
    << DAT_HIST->Integral() << ","
    << BKG_HIST->Integral() << ","
    << SRC_HIST->Integral() << ","
    << lnL_nosrc_nobb << ","
    << lnL_src_nobb << ","
    << TS_nobb << std::endl;
  f.close();

  f.open(bb_output.str(), std::ios::out | std::ios::app);
  if(args.bin_vars & 1) f << ins.za << ",";
  if(args.bin_vars & 2) f << ins.e << ",";
  if(args.bin_vars & 4) f << ins.tel << ",";
  if(args.bin_vars & 8) f << ins.az << ",";
  if(args.bin_vars & 16) f << ins.off << ",";
  f << std::scientific
    << fit_nosrc_bb->GetParameter(0) << ","
    << fit_src_bb->GetParameter(0) << ","
    << fit_src_bb->GetParameter(1) << ","
    << std::defaultfloat
    << DAT_HIST->Integral() << ","
    << BKG_HIST->Integral() << ","
    << SRC_HIST->Integral() << ","
    << lnL_nosrc_bb << ","
    << lnL_src_bb << ","
    << TS_bb << std::endl;
  f.close();

  double signal = DAT_HIST->Integral() * fit_src_bb->GetParameter(1);
  f.open(summary.str(), std::ios::out | std::ios::app);
  if(args.bin_vars & 1) f << ins.za << ",";
  if(args.bin_vars & 2) f << ins.e << ",";
  if(args.bin_vars & 4) f << ins.tel << ",";
  if(args.bin_vars & 8) f << ins.az << ",";
  if(args.bin_vars & 16) f << ins.off << ",";
  f << std::scientific
    << fit_src_bb->GetParameter(0) << ","
    << fit_src_bb->GetParError(0) << ","
    << fit_src_bb->GetParameter(1) << ","
    << fit_src_bb->GetParError(1) << ","
    << std::defaultfloat
    << DAT_HIST->Integral() << ","
    << BKG_HIST->Integral() << ","
    << SRC_HIST->Integral() << ","
    << TS_bb << ","
    << signal << ","
    << fit_src_bb->GetParError(1) << std::endl;


  fracs[0] = fit_src_nobb->GetParameter(0);
  fracs[1] = fit_src_nobb->GetParameter(1);
  fracs[2] = fit_src_bb->GetParameter(0);
  fracs[3] = fit_src_bb->GetParameter(1);
  fracs[4] = fit_nosrc_nobb->GetParameter(0);
  fracs[5] = fit_nosrc_bb->GetParameter(0);

  if(args.hist & 2 && hists) histogram_fit_data(fracs, ins, &args, hists, fit_param, fit_src_bb);
  if(args.output & 4) print_errors(fit_src_bb, fit_param, OUTPATH);
  if(args.graphics & 1) map_likelihood(fit_src_nobb->GetParameter(0), fit_src_nobb->GetParameter(1), "Std", ins, args, OUTPATH, LONGOUTPATH);
  if(args.graphics & 2) map_likelihood(fit_src_bb->GetParameter(0), fit_src_bb->GetParameter(1), "BB", ins, args, OUTPATH, LONGOUTPATH);

}

void bidirectional(args_t *args, indices_t indices, double alpha, std::string fit_param){
  double fracs_for[6];
  fit(indices, *args, alpha, fracs_for, fit_param);

  //Plot Forwards
  gStyle->SetOptStat(0);
  TCanvas *c1 = new TCanvas("",OUTPATH.c_str(),1600,1600);
  c1->Divide(3,2);
  std::stringstream title;

  //Plot Forward Fit
  c1->cd(1);
  TH1D* dat_fit_forward = new TH1D("DFit_For", "Forward", NBIN, MSWLOW, MSWHIGH);
  TH1D* bkg_fit_forward = new TH1D("BFit_For", "Forward", NBIN, MSWLOW, MSWHIGH);
  dat_fit_forward->SetLineColor(4);
  bkg_fit_forward->SetLineColor(6);
  dat_fit_forward->SetStats(false);
  bkg_fit_forward->SetStats(false);
  title << OUTPATH << " " << "Fit Forward";
  dat_fit_forward->SetTitle(title.str().c_str());
  title.str("");
  src_BB(fracs_for[2], fracs_for[3], false, dat_fit_forward, bkg_fit_forward);
  bkg_fit_forward->Scale(dat_fit_forward->Integral() / bkg_fit_forward->Integral());
  TLegend *legend2 = new TLegend(0.12, 0.6, 0.4, 0.9);
  legend2->AddEntry(dat_fit_forward, "Fit Data");
  legend2->AddEntry(bkg_fit_forward, "Fit Bkg");
  TRatioPlot_BetterError* for_fit = new TRatioPlot_BetterError(dat_fit_forward, bkg_fit_forward, "diffsig");
  for_fit->SetH1DrawOpt("E0");
  for_fit->SetH2DrawOpt("E0");
  for_fit->Draw();
  legend2->Draw();

  //store ts for later
  double ts1 = -2*(src_BB(fracs_for[2], fracs_for[3]) - nosrc_BB(fracs_for[5]));

  //Run Backward Fit
  TH1D* temp = DAT_HIST;
  DAT_HIST = BKG_HIST;
  BKG_HIST = temp;
  double fracs_back[6];
  fit(indices, *args, alpha, fracs_back, fit_param);

  //Plot Backward Fit
  c1->cd(4);
  TH1D* dat_fit_back = new TH1D("DFit_Back", "Backward", NBIN, MSWLOW, MSWHIGH);
  TH1D* bkg_fit_back = new TH1D("BFit_Back", "Backward", NBIN, MSWLOW, MSWHIGH);
  dat_fit_back->SetLineColor(6);
  bkg_fit_back->SetLineColor(4);
  dat_fit_back->SetStats(false);
  bkg_fit_back->SetStats(false);
  title << OUTPATH << " " << "Fit Backward";
  dat_fit_back->SetTitle(title.str().c_str());
  title.str("");
  src_BB(fracs_back[2], fracs_back[3], false, dat_fit_back, bkg_fit_back);
  bkg_fit_back->Scale(dat_fit_back->Integral() / bkg_fit_back->Integral());
  TLegend *legend4 = new TLegend(0.12, 0.6, 0.4, 0.9);
  legend4->AddEntry(dat_fit_back, "Fit Bkg");
  legend4->AddEntry(bkg_fit_back, "Fit Data");
  TRatioPlot_BetterError* back_fit = new TRatioPlot_BetterError(dat_fit_back, bkg_fit_back, "diffsig");
  back_fit->SetH1DrawOpt("E0");
  back_fit->SetH1DrawOpt("E0");
  back_fit->Draw();
  legend4->Draw();

  //Plot Raw Comp & Pulls
  c1->cd(2);
  TH1D* dat1 = new TH1D(*BKG_HIST);
  TH1D* dat2 = new TH1D(*DAT_HIST);
  dat1->SetLineColor(1);
  dat2->SetLineColor(2);
  dat1->SetTitle("Raw Comparison");
  dat2->Scale(dat1->Integral() / dat2->Integral());
  TRatioPlot_BetterError* rp_raw = new TRatioPlot_BetterError(dat1, dat2, "diffsig");
  rp_raw->SetH1DrawOpt("E0");
  rp_raw->SetH2DrawOpt("E0");
  rp_raw->Draw();
  TLegend *legend5 = new TLegend(0.12, 0.6, 0.4, 0.9);
  legend5->AddEntry(dat1, "Data");
  legend5->AddEntry(dat2, "Background");
  legend5->Draw();

  //Plot Fit Comp & Pulls
  /*
  c1->cd(5);
  TH1D* fit1 = new TH1D(*dat_fit_forward);
  TH1D* fit2 = new TH1D(*dat_fit_back);
  fit1->SetLineColor(1);
  fit2->SetLineColor(2);
  fit1->SetTitle("Fit Comparison");
  fit2->Scale(fit1->Integral() / fit2->Integral());
  TRatioPlot_BetterError* rp_fit = new TRatioPlot_BetterError(fit1, fit2, "diffsig");
  rp_fit->SetH1DrawOpt("E0");
  rp_fit->SetH2DrawOpt("E0");
  rp_fit->Draw();
  TLegend *legend6 = new TLegend(0.12, 0.8, 0.3, 0.9);
  legend6->AddEntry(fit1, "Sample 1");
  legend6->AddEntry(fit2, "Sample 2");
  legend6->Draw();
  */

  //Write Forward Fit Data
  c1->cd(3);
  TPaveText *pt1 = new TPaveText(0, 0, 1, 1);
  std::stringstream line;
  pt1->AddText(.5, .95, "Forward Fit Values:");
  line << "P_b = " << fracs_for[2];
  pt1->AddText(.05, .85, line.str().c_str());
  line.str("");
  line << "P_s = " << fracs_for[3];
  pt1->AddText(.05, .8, line.str().c_str());
  line.str("");
  line << "TS = " << ts1;
  pt1->AddText(.05, .75, line.str().c_str());
  line.str("");
  line << "Data Count = " << BKG_HIST->Integral();
  pt1->AddText(.05, .7, line.str().c_str());
  line.str("");
  line << "Bkg Count = " << DAT_HIST->Integral();
  pt1->AddText(.05, .65, line.str().c_str());
  line.str("");
  line << "Src Count = " << SRC_HIST->Integral();
  pt1->AddText(.05, .6, line.str().c_str());
  line.str("");
  pt1->AddLine(0, .5, 1, .5);
  pt1->AddText(.5, .4, "Bin Boundaries:");
  if(args->bin_vars & 1){
    line << "Zenith Angle: " << ZABINS[indices.za] << "-" << ZABINS[indices.za+1];
    pt1->AddText(.05, .3, line.str().c_str());
    line.str("");
  }
  if(args->bin_vars & 2){
    line << "Energy: " << EBINS[indices.e] << "-" << EBINS[indices.e+1];
    pt1->AddText(.05, .25, line.str().c_str());
    line.str("");
  }
  if(args->bin_vars & 4){
    line << "Telescope: " << TBINS[indices.tel];
    pt1->AddText(.05, .2, line.str().c_str());
    line.str("");
  }
  if(args->bin_vars & 8){
    line << "Azimuth: " << AZBINS[indices.az] << "-" << AZBINS[indices.az+1];
    pt1->AddText(.05, .15, line.str().c_str());
    line.str("");
  }
  if(args->bin_vars & 16){
    line << "Offset: " << OBINS[indices.off] << "-" << OBINS[indices.off+1];
    pt1->AddText(.05, .1, line.str().c_str());
    line.str("");
  }
  pt1->SetAllWith("=", "size", .05);
  pt1->SetAllWith("=", "align", 12);
  pt1->SetAllWith(": ", "size", .05);
  pt1->SetAllWith(": ", "align", 12);
  pt1->Draw();


  //Write Backward Fit Data
  c1->cd(6);
  double ts2 = -2*(src_BB(fracs_back[2], fracs_back[3]) - nosrc_BB(fracs_back[5]));
  TPaveText *pt2 = new TPaveText(0, 0, 1, 1);
  pt2->AddText(.5, .95, "Backward Fit Values:");
  line << "P_b = " << fracs_back[2];
  pt2->AddText(.05, .85, line.str().c_str());
  line.str("");
  line << "P_s = " << fracs_back[3];
  pt2->AddText(.05, .8, line.str().c_str());
  line.str("");
  line << "TS = " << ts2;
  pt2->AddText(.05, .75, line.str().c_str());
  line.str("");
  line << "Data Count = " << DAT_HIST->Integral();
  pt2->AddText(.05, .7, line.str().c_str());
  line.str("");
  line << "Bkg Count = " << BKG_HIST->Integral();
  pt2->AddText(.05, .65, line.str().c_str());
  line.str("");
  line << "Src Count = " << SRC_HIST->Integral();
  pt2->AddText(.05, .60, line.str().c_str());
  line.str("");
  pt2->AddLine(0, .5, 1, .5);
  if(args->op_info.c_str()){
    std::ifstream infile(args->op_info);
    std::string readline;
    std::getline(infile, readline);
    pt2->AddText(.05, .4, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt2->AddText(.05, .35, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt2->AddText(.05, .3, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt2->AddText(.05, .25, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt2->AddText(.05, .2, readline.c_str())->SetTextAlign(12);
    std::getline(infile, readline);
    pt2->AddText(.05, .15, readline.c_str())->SetTextAlign(12);
    infile.close();
  }

  pt2->SetAllWith("=", "size", .05);
  pt2->SetAllWith("=", "align", 12);
  pt2->Draw();


  //Save
  title << "Bidirectional_" << OUTPATH << ".png";
  c1->SaveAs(title.str().c_str());
  c1->Clear();
  delete c1;
  delete DAT_HIST;
  delete BKG_HIST;
  delete SRC_HIST;
  delete dat_fit_forward;
  delete bkg_fit_forward;
  delete dat_fit_back;
  delete bkg_fit_back;
  delete legend2;
  delete legend4;
  delete legend5;
//   delete legend6;
  delete dat1;
  delete dat2;
//   delete fit1;
//   delete fit2;
}

int main(int argc, char* argv[]){
  args_t* args = new args_t;
  if(parse_command_line(argc, argv, args)) return 1;
  prepare_std_output_files(*args);
  indices_t indices;
  for(indices.za = 0; indices.za < 6; indices.za++){
    for(indices.e = 0; indices.e < 4; indices.e++){
      for(indices.tel = 0; indices.tel < 2; indices.tel++){
        for(indices.az = 0; indices.az < 8; indices.az++){
          for(indices.off = 0; indices.off < 4; indices.off++){
            if(optional_binning(indices, *args)) continue;
            TH1::SetDefaultSumw2();
            //Initialize Histograms
            hists_t *hists = new hists_t;
            if(args->fit_params & 1){
              hists->msw_dat = new TH1D("MSWDataHist", "MSW Data", NBIN, MSWLOW, MSWHIGH);
              hists->msw_bkg = new TH1D("MSWBkgHist", "MSW Bkg", NBIN, MSWLOW, MSWHIGH);
              hists->msw_src = new TH1D("MSWSrcHist", "MSW Src", NBIN, MSWLOW, MSWHIGH);
            }
            if(args->fit_params & 2){
              hists->bdt_dat = new TH1D("BDTDataHist", "BDT Data", NBIN, BDTLOW, BDTHIGH);
              hists->bdt_bkg = new TH1D("BDTBkgHist", "BDT Bkg", NBIN, BDTLOW, BDTHIGH);
              hists->bdt_src = new TH1D("BDTSrcHist", "BDT Src", NBIN, BDTLOW, BDTHIGH);
            }
            if(args->graphics & 4){
              hists->msw_msl_dat = new TH2D("Data_MSWvsMSL", "Data MSWvsMSL", NBIN, MSWLOW, MSWHIGH, NBIN, MSWLOW, MSWHIGH);
              hists->msw_msl_bkg = new TH2D("Bkg_MSWvsMSL", "Bkg MSWvsMSL", NBIN, MSWLOW, MSWHIGH, NBIN, MSWLOW, MSWHIGH);
            }
            hists->outpath = OUTPATH;
            hists->longoutpath = LONGOUTPATH;
            double alpha = 1;
            loadData(indices, *args, &alpha, hists);
            if(!hists->msw_dat && !hists->bdt_dat) throw 407;
            if(!hists->msw_bkg && !hists->bdt_bkg) throw 408;
            if(!hists->msw_src && !hists->bdt_src) throw 409;
            if(args->output & 2 && args->fit_params & 1) printRawData(hists, "MSW");
            if(args->output & 2 && args->fit_params & 2) printRawData(hists, "BDT");
            if(args->hist & 1 && args->fit_params & 1) histogram_raw_data(hists, "MSW");
            if(args->hist & 1 && args->fit_params & 2) histogram_raw_data(hists, "BDT");
            if(args->graphics & 4) plot_msw_vs_msl(hists);
            if(!(hists->msw_dat->Integral() + hists->bdt_dat->Integral())) continue;
            if(!(hists->msw_bkg->Integral() + hists->bdt_bkg->Integral())) continue;
            if(!(hists->msw_src->Integral() + hists->bdt_src->Integral())) continue;

            //MSW Fit
            if(args->fit_params & 1){
              DAT_HIST = hists->msw_dat;
              BKG_HIST = hists->msw_bkg;
              SRC_HIST = hists->msw_src;
              OUTPATH = "MSW" + hists->outpath;
              LONGOUTPATH = "MSW" + hists->longoutpath;
              if(args->bidir) bidirectional(args, indices, alpha, "MSW");
              else {
                double fracs[6];
                fit(indices, *args, alpha, fracs, "MSW", hists);
              }
            }

            //BDT Fit
            if(args->fit_params & 2){
              DAT_HIST = hists->bdt_dat;
              BKG_HIST = hists->bdt_bkg;
              SRC_HIST = hists->bdt_src;
              OUTPATH = "BDT" + hists->outpath;
              LONGOUTPATH = "BDT" + hists->longoutpath;
              if(args->bidir) bidirectional(args, indices, alpha, "BDT");
              else{
                double fracs[6];
                fit(indices, *args, alpha, fracs, "BDT", hists);
              }
            }

            //Clean up hists
            delete hists->msw_dat;
            delete hists->msw_bkg;
            delete hists->msw_src;
            delete hists->bdt_dat;
            delete hists->bdt_bkg;
            delete hists->bdt_src;
            delete hists->msw_msl_dat;
            delete hists->msw_msl_bkg;
          }
        }
      }
    }
  }
  return 0;
}

//Helpers
int parse_command_line(int argc, char* argv[], args_t* args){
  for(int i = 0; i < argc; i++){
    if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")){
      const char *help_text = R"(
OPTIONS:
  -b VAR, --bin-variable VAR
    Sets which variables to bin over.
    Available: zenith, energy, telescope, azimuth, offset, all.
    Default: zenith, energy, telescope.

  --bidirectional
    Runs the fit normally, then swaps data and bkg and runs the fit again.

  -d FORMAT, --data-format FORMAT
    Format of the imput data.
    Available: vegas, sample.
    Default: vegas.

  --fit-parameter
    Which parameter(s) to run the fit on.
    Available: msw, bdt
    Default: msw

  -g GRAPHICS, --graphics GRAPHICS
    Triggers output of graphics files.
    Available: none, stdlnL, bblnL, mswmsl, all.
    Default: none.

  -h, --help
    Print help text and exit.

  -hist DATA, --histogram DATA
    Triggers output of histograms.
    Available: none, raw, fit, all.
    Default: none.

  --no-cache <Not Implemented>
    Disables input data caching

  -op, --op-info PATH
    Passes file path to be used where optional info is printed.

  -out DATA, --output DATA
    Triggers output of ascii files.
    Available: none, bins, raw, errors, cuts, all.
    Default: none.

  -v VERBOSITY, --verbosity VERBOSITY
    Controls the verbosity of MINUIT.
    Default: -1.
      )";
      std::cout << help_text << std::endl;
      return 1;
    }
    if(!strcmp(argv[i], "-d") || !strcmp(argv[i], "--data-format")){
      if(i < argc - 1 && !strcmp(argv[i+1], "vegas")){
        args->format = Format_t::Vegas;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "sample")){
        args->format = Format_t::Sample;
      }
    }
    if(!strcmp(argv[i], "-hist") || !strcmp(argv[i], "--histogram")){
      if(i < argc - 1 && !strcmp(argv[i+1], "none")){
        args->hist = 0;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "raw")){
        if(args->hist & 1) args->hist -= 1;
        else args->hist += 1;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "fit")){
        if(args->hist & 2) args->hist -= 2;
        else args->hist += 2;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "all")){
        args->hist = 3;
      }
    }
    if(!strcmp(argv[i], "-out") || !strcmp(argv[i], "--output")){
      if(i < argc - 1 && !strcmp(argv[i+1], "none")){
        args->output = 0;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "bins")){
        if(args->output & 1) args->output -= 1;
        else args->output += 1;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "raw")){
        if(args->output & 2) args->output -= 2;
        else args->output += 2;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "errors")){
        if(args->output & 4) args->output -= 4;
        else args->output += 4;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "cuts")){
        if(args->output & 8) args->output -= 8;
        else args->output += 8;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "all")){
        args->output = 15;
      }
    }
    if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--verbosity")){
      if(i < argc - 1){
        args->verbosity = atoi(argv[++i]);
      }
    }
    if(!strcmp(argv[i], "-b") || !strcmp(argv[i], "--bin-variable")){
      if(i < argc - 1 && !strcmp(argv[i+1], "zenith") && !(args->bin_vars & 1)){
        if(args->bin_vars & 1) args->bin_vars -= 1;
        else args->bin_vars += 1;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "energy") && !(args->bin_vars & 2)){
        if(args->bin_vars & 2) args->bin_vars -= 2;
        else args->bin_vars += 2;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "telescope") && !(args->bin_vars & 4)){
        if(args->bin_vars & 4) args->bin_vars -= 4;
        else args->bin_vars += 4;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "azimuth") && !(args->bin_vars & 8)){
        if(args->bin_vars & 8) args->bin_vars -= 8;
        else args->bin_vars += 8;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "offset") && !(args->bin_vars & 16)){
        if(args->bin_vars & 16) args->bin_vars -= 16;
        else args->bin_vars += 16;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "all")){
        args->bin_vars = 31;
      }
    }
    if(!strcmp(argv[i], "-g") || !strcmp(argv[i], "--graphics")){
      if(i < argc - 1 && !strcmp(argv[i+1], "none")){
        args->graphics = 0;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "stdlnL") && !(args->graphics & 1)){
        if(args->graphics & 1) args->graphics -= 1;
        else args->graphics += 1;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "bblnL") && !(args->graphics & 2)){
        if(args->graphics & 2) args->graphics -= 2;
        else args->graphics += 2;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "mswmsl") && !(args->graphics & 4)){
        if(args->graphics & 4) args->graphics -= 4;
        else args->graphics += 4;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "all")){
        args->graphics = 7;
      }
    }
    if(!strcmp(argv[i], "-op") || !strcmp(argv[i], "--op-info")){
      if(i < argc -1) args->op_info = argv[i+1];
    }
    if(!strcmp(argv[i], "--bidirectional")){
      args->bidir = true;
    }
    if(!strcmp(argv[i], "--no-cache")){
      args->cache = false;
    }
    if(!strcmp(argv[i], "--fit-parameter")){
      if(i < argc - 1 && !strcmp(argv[i+1], "msw")){
        if(args->fit_params & 1) args->fit_params -= 1;
        else args->fit_params += 1;
      }
      if(i < argc - 1 && !strcmp(argv[i+1], "bdt")){
        if(args->fit_params & 2) args->fit_params -= 2;
        else args->fit_params += 2;
      }
    }
  }
  return 0;
}

void prepare_std_output_files(args_t args){
  std::vector<std::string> outfiles;
  std::vector<std::string> summary_files;
  if(args.fit_params & 1){
    outfiles.push_back("fitstats_MSW_std.csv");
    outfiles.push_back("fitstats_MSW_bb.csv");
    summary_files.push_back("summary_MSW.csv");
  }
  if(args.fit_params & 2){
    outfiles.push_back("fitstats_BDT_std.csv");
    outfiles.push_back("fitstats_BDT_bb.csv");
    summary_files.push_back("summary_BDT.csv");
  }
  for(std::string str : outfiles){
    std::ofstream f(str);
    if(args.bin_vars & 1)  f << "ZA,";
    if(args.bin_vars & 2)  f << "E,";
    if(args.bin_vars & 4)  f << "T,";
    if(args.bin_vars & 8)  f << "A,";
    if(args.bin_vars & 16) f << "O,";
    f << "bkgfrac_nosrc, bkgfrac, srcfrac, dataCt, bkgCt, srcCt, lnL_nosrc, lnL_src, TS" << std::endl;
    f.close();
  }
  for(std::string str : summary_files){
    std::ofstream f(str);
    if(args.bin_vars & 1)  f << "ZA,";
    if(args.bin_vars & 2)  f << "E,";
    if(args.bin_vars & 4)  f << "T,";
    if(args.bin_vars & 8)  f << "A,";
    if(args.bin_vars & 16) f << "O,";
    f << "bkgfrac,e_bkgfrac,srcfrac,e_srcfrac,dataCt,bkgCt,srcCt,TS,Signal,e_Signal" << std::endl;
    f.close();
  }

}

int optional_binning(indices_t indices, args_t args){
  std::stringstream path;
  std::stringstream longpath;
  if(!(args.bin_vars & 1) && indices.za != 0) return 1;
  else if(args.bin_vars & 1){
    path << "ZA" << indices.za;
    longpath << "ZA" << ZABINS[indices.za] << "-" << ZABINS[indices.za+1];
  }
  if(!(args.bin_vars & 2) && indices.e != 0) return 1;
  else if(args.bin_vars & 2){
    path << "E" << indices.e;
    longpath << "_E" << EBINS[indices.e] << "-" << EBINS[indices.e+1];
  }
  if(!(args.bin_vars & 4) && indices.tel != 0) return 1;
  else if(args.bin_vars & 4){
    path << "T" << TBINS[indices.tel];
    longpath << "_T" << TBINS[indices.tel];
  }
  if(!(args.bin_vars & 8) && indices.az != 0) return 1;
  else if(args.bin_vars & 8){
    path << "A" << indices.az;
    longpath << "_A" << AZBINS[indices.az] << "-" << AZBINS[indices.az+1];
  }
  if(!(args.bin_vars & 16) && indices.off != 0) return 1;
  else if(args.bin_vars & 16){
    path << "O" << indices.off;
    longpath << "_O" << OBINS[indices.off] << "-" << OBINS[indices.off+1];
  }
  std::cout << path.str() << std::endl;
  std::cout << longpath.str() << std::endl;
  OUTPATH = path.str();
  LONGOUTPATH = longpath.str();
  return 0;
}
