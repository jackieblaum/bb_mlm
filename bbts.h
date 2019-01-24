#ifndef BBTS_H
#define BBTS_H

//ROOT Includes
#include "TH1D.h"
#include "TFile.h"
#include "Riostream.h"
#include "TFractionFitter.h"
#include "TTreeReader.h"
#include "TLegend.h"
#include "TMath.h"
#include "TFitter.h"
#include "TCanvas.h"
#include "TTreeReader.h"
#include "TCut.h"
#include "TChain.h"
#include "TPaveText.h"
#include "TRatioPlot.h"
//VEGAS Includes
#include "VASkyMap.h"
#include "VSOptions.hpp"
#include "VSDataConverter.hpp"
#include "VATime.h"
#include "VASimpleCuts.h"
//CPP Includes
#include <boost/algorithm/string.hpp>
#include <iostream>
#include <sstream>
#include <iomanip>
#include <string>
//C Includes
#include <ctime>
#include <cstring>
#include <cstdlib>
//Project Includes
#include "TRatioPlot_BetterError.h"

#define timestamp                                           \
{                                                           \
  Long_t __timestamp = std::time(0);                        \
  std::cout << std::asctime(std::localtime(&__timestamp));  \
}

#define MSWLOW 0.7875
#define MSWHIGH 1.3125
#define BDTLOW 0.589
#define BDTHIGH 1.011
#define NBIN 20 //TODO make bins parameter dependent
#define PW 15

extern double ZABINS[];
extern double EBINS[];
extern int TBINS[];
extern double AZBINS[];
extern double OBINS[];

enum class Format_t{Vegas, Bbmlm};
struct args_t {
  args_t() : format(Format_t::Vegas),
  verbosity(-1),
  bin_vars(7),
  fit_params(1) {}
  Format_t format;
  int verbosity;
  int bin_vars;
  int fit_params;
};
struct indices_t{
  int za;
  int e;
  int tel;
  int az;
  int off;
};
struct cuts_t{
  cuts_t() : za(0), e(0), tel(0), az(0), off(0),
  src(0), passed(0), read(0) {}
  int za;
  int e;
  int tel;
  int az;
  int off;
  int par;
  int src;
  int passed;
  int read;
};
struct hists_t{
  TH1D* msw_dat;
  TH1D* msw_bkg;
  TH1D* msw_src;
  TH1D* bdt_dat;
  TH1D* bdt_bkg;
  TH1D* bdt_src;
  std::string outpath;
  std::string longoutpath;
};

double nosrc_noBB(double Pb, TH1D* F = 0);
double src_noBB(double Pb, double Ps, TH1D* F = 0);
double nosrc_BB(double Pb, TH1D* F = 0, TH1D* B = 0);
double src_BB(double Pb, double Ps, TH1D* F = 0, TH1D* B = 0);

#endif
