#include "load_data.h"
#include "output.h"

std::string OUTSTR;

typedef struct SourceCut
{
  VACoordinatePair coords;
  double cut_rad;
  SourceCut(std::vector<std::string> line_in) :
  coords(std::stod(line_in[1]),std::stod(line_in[2]),VACoordinates::J2000,VACoordinates::Deg),
  cut_rad(std::stod(line_in[3]))
  {}

  bool InsideExclRadius(VACoordinatePair evt_coords) {
    double dist = coords.angularSeparation_Deg(evt_coords);
    return (dist < cut_rad);
  }
} SourceCut_t;

bool check_event(TTreeReader *reader, cuts_t *cuts, args_t *args, indices_t *ins){
  TTreeReaderValue<float> zenith(*reader, "Zenith");
  TTreeReaderValue<float> energy(*reader, "Energy_GeV");
  TTreeReaderValue<float> ntels(*reader, "NTels");
  TTreeReaderValue<float> azimuth(*reader, "Azimuth");
  TTreeReaderValue<float> offset(*reader, "Offset");
  TTreeReaderValue<float> src_excl(*reader, "Src_Excl");

  bool pass = true;

  //ZA Bin
  if(args->bin_vars & 1){
    if(*zenith >= ZABINS[ins->za] || *zenith < ZABINS[ins->za]){
      pass = false;
      cuts->za++;
    }
  }

  //Energy Bin
  if(args->bin_vars & 2){
    if(*energy >= EBINS[ins->e] || *energy < EBINS[ins->e]){
      pass = false;
      cuts->e++;
    }
  }

  //Tel Bin
  if(args->bin_vars & 4){
    if(*ntels != TBINS[ins->tel]){
      pass = false;
      cuts->tel++;
    }
  }

  //Azimuth Bin
  if(args->bin_vars & 8){
    if(*azimuth >= AZBINS[ins->az] || *azimuth < AZBINS[ins->az]){
      pass = false;
      cuts->az++;
    }
  }

  //Offset Bin
  if(args->bin_vars & 16){
    if(*offset >= OBINS[ins->off] || *offset < OBINS[ins->off]){
      pass = false;
      cuts->off++;
    }
  }
  //Offset Cut
  else if(*offset > MAX_OFFSET){
    pass = false;
    cuts->off++;
  }

  //Source Cut
  if(*src_excl){
    pass = false;
    cuts->src++;
  }

  return pass;
}

bool check_event_msw(TTreeReader *reader, cuts_t *cuts, args_t *args, indices_t *ins){
  TTreeReaderValue<float> msw(*reader, "MSW");
  TTreeReaderValue<float> msl(*reader, "MSL");
  TTreeReaderValue<float> height(*reader, "ShowerHeightMax_KM");

  bool pass = true;

  //MSL Cut
  if(*msl > MAX_MSL || *msl < MIN_MSL){
    pass = false;
    cuts->msl++;
  }

  //Shower Height Cut
  if(*height < MIN_HEIGHT){
    pass = false;
    cuts->height++;
  }

  //MSW Cut
  if(*msw > MSWHIGH || *msw < MSWLOW){
    pass = false;
    cuts->par++;
  }

  return pass;
}

bool check_event_bdt(TTreeReader *reader, cuts_t *cuts, args_t *args, indices_t *ins){
  TTreeReaderValue<float> bdt(*reader, "BDT");

  bool pass = true;

  //BDT Cut
  if(*bdt > BDTHIGH || *bdt < BDTLOW){
    pass = false;
    cuts->par++;
  }

  return pass;
}


double loadData_bbmlm(indices_t *ins, args_t *args, TH1D *hist, Fit_Par_t fit_par, std::string file_path){
  TFile *infile = TFile::Open(file_path.c_str());
  if(!infile->IsOpen()){
    std::cerr << "Cannot open " << file_path << std::endl;
    return -1;
  }
  else{
    std::cout << "Reading " << file_path << std::endl;
  }

  TTreeReader reader("ntuple", infile);
  TTreeReaderValue<float> msw(reader, "MSW");
  TTreeReaderValue<float> bdt(reader, "BDT");

  cuts_t cuts;
  timestamp;
  while(reader.Next()){
    cuts.read++;

    bool pass = false;
    if(fit_par == Fit_Par_t::msw){
      pass = check_event(&reader, &cuts, args, ins)
           & check_event_msw(&reader, &cuts, args, ins);
      if(pass) hist->Fill(*msw);
    }
    else if(fit_par == Fit_Par_t::bdt){
      pass = check_event(&reader, &cuts, args, ins)
           & check_event_bdt(&reader, &cuts, args, ins);
      if(pass) hist->Fill(*bdt);
    }

  }
  std::cout << hist->Integral() << " passed cuts." << std::endl;
  std::cout << cuts.read << " cuts read." << std::endl;
  std::cout << cuts.src << " failed src cut." << std::endl;
  std::cout << cuts.par << " failed parameter cut." << std::endl;
  std::cout << cuts.msl << " failed msl cut." << std::endl;
  std::cout << cuts.height << " failed height cut." << std::endl;
  std::cout << cuts.za << " failed za cut." << std::endl;
  std::cout << cuts.e << " failed energy cut." << std::endl;
  std::cout << cuts.tel << " failed tel cut." << std::endl;
  std::cout << cuts.az << " failed az cut." << std::endl;
  std::cout << cuts.off << " failed off cut." << std::endl;


  return 1;
}

void loadData(indices_t *ins, args_t *args, hists_t *hists, Fit_Par_t fit_par){
  OUTSTR = hists->outpath;
  loadData_bbmlm(ins, args, hists->dat, fit_par, "data.root");
  loadData_bbmlm(ins, args, hists->bkg, fit_par, "bkg.root");
  loadData_bbmlm(ins, args, hists->src, fit_par, "src.root");
  std::cout << "Histograms loaded." << std::endl;
}
