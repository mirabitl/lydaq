//////////////////////////////////////////////////////////
// This class has been automatically generated on
// Wed Jul 12 17:30:04 2017 by ROOT version 5.34/26
// from TTree evt/a Tree with SDHCAL frame storage
// found on file: /data/srv02/RAID6/Dome0916/SMM_120717_170551_736300.root
//////////////////////////////////////////////////////////

#ifndef evt_h
#define evt_h

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TdcChannel.hh"
#include "DCHistogramHandler.hh"
#include <vector>
#include <map>
#include<stdint.h>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <map>
#include <bitset>
#include <dirent.h>
#include <fnmatch.h>

#define TDC_TRIGGER_CHANNEL 16
// Header file for the classes stored in the TTree if any.

// Fixed size dimensions of array or collections stored in the TTree if any.
class TdcStrip
{
public:
  TdcStrip() :_dif(0), _str(0),_t0(0),_t1(0),_shift(0){;}
  TdcStrip(uint16_t dif,uint16_t st,double t0,double t1,double shift=0) :_dif(dif), _str(st),_t0(t0),_t1(t1),_shift(shift){;}
  inline uint8_t strip() const {return _str;}
  inline uint8_t dif() const {return _dif;}
  inline double t0() const {return _t0;}
  inline double t1() const {return _t1;}
  inline double ypos() const {return (_t0-_t1-_shift)/0.125;}
  inline double xpos() const {
    if (_dif%2==1) return -1*(_str*0.4+0.2);
    else return +1*(_str*0.4+0.2);
  }
private:
  uint16_t _dif,_str;
  double _t0,_t1,_shift;

};
class evt {
public :
   TTree          *fChain;   //!pointer to the analyzed TTree or TChain
   Int_t           fCurrent; //!current Tree number in a TChain

   // Declaration of leaf types
   Int_t           run;
   Int_t           event;
   Int_t           gtc;
   Long64_t        abcid;
   Int_t           nframe;
   Long64_t        frame[1000000];   //[nframe]
   Long64_t        info[1000000];   //[nstrip]

   // List of branches
   TBranch        *b_run;   //!
   TBranch        *b_event;   //!
   TBranch        *b_gtc;   //!
   TBranch        *b_abcid;   //!
   TBranch        *b_nframe;   //!
   TBranch        *b_frame;   //!
   TBranch        *b_info;   //!

   evt(TTree *tree=0);
   evt(std::string fn);
  evt(Int_t nr);
   virtual ~evt();
   virtual Int_t    Cut(Long64_t entry);
   virtual Int_t    GetEntry(Long64_t entry);
   virtual Long64_t LoadTree(Long64_t entry);
   virtual void     Init(TTree *tree);
   virtual void     Loop();
   virtual Bool_t   Notify();
   virtual void     Show(Long64_t entry = -1);
   DCHistogramHandler* _rh;
   TH1* GetTH1(std::string name){return _rh->GetTH1(name);}
   TH2* GetTH2(std::string name){return _rh->GetTH2(name);}
  void writeHistograms(std::string n){_rh->writeHistograms(n);}

  void drawHits();
  
 private:
  std::map<uint32_t,std::vector<lydaq::TdcChannel> > _mezMap;
  uint32_t _runType,_dacSet,_vthSet,_nevt,_ntrigger,_nfound,_nbside;
  std::vector<lydaq::TdcChannel>::iterator _trigger;
  std::vector<TdcStrip> _strips;
};

#endif

#ifdef evt_cxx
evt::evt(TTree *tree) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
   if (tree == 0) {
      TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject("/data/srv02/RAID6/Dome0916/SMM_130717_141221_736307.root");
      if (!f || !f->IsOpen()) {
         f = new TFile("/data/srv02/RAID6/Dome0916/SMM_130717_141221_736307.root");
      }
      f->GetObject("evt",tree);

   }
   Init(tree);
   _rh=DCHistogramHandler::instance();

}
evt::evt(std::string fn) : fChain(0) 
{
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
  TTree* tree=NULL;
   if (tree == 0) {
     TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(fn.c_str());
      if (!f || !f->IsOpen()) {
         f = new TFile(fn.c_str());
      }
      f->GetObject("evt",tree);

   }
   Init(tree);
   _rh=DCHistogramHandler::instance();
}
evt::evt(Int_t nr) : fChain(0) 
{

 std::stringstream spat;
 // int runask=atol(argv[1]);
 std::string fn="";
  spat<<"SMM*"<<nr<<"*.root";
  struct dirent **namelist;
  int n;
  std::cout<<"Pattern "<<spat.str()<<std::endl;
  std::string dirp="/data/srv02/RAID6/FE1PR";
  n = scandir(dirp.c_str(), &namelist, NULL, alphasort);
  if (n < 0)
    perror("scandir");
  else {
    while (n--) {

      if (fnmatch(spat.str().c_str(), namelist[n]->d_name, 0)==0)
	{
	  printf("%s %d \n", namelist[n]->d_name,fnmatch(spat.str().c_str(), namelist[n]->d_name, 0));
	  printf("found\n");
	  std::stringstream sf;
	  sf<<dirp<<"/"<< namelist[n]->d_name;
	  fn=sf.str();
	  break;
	}
      free(namelist[n]);
    }
    free(namelist);
  }
  
// if parameter tree is not specified (or zero), connect the file
// used to generate this class and read the Tree.
  TTree* tree=NULL;
   if (tree == 0) {
     TFile *f = (TFile*)gROOT->GetListOfFiles()->FindObject(fn.c_str());
      if (!f || !f->IsOpen()) {
         f = new TFile(fn.c_str());
      }
      f->GetObject("evt",tree);

   }
   Init(tree);
   _rh=DCHistogramHandler::instance();
}

evt::~evt()
{
   if (!fChain) return;
   delete fChain->GetCurrentFile();
}

Int_t evt::GetEntry(Long64_t entry)
{
// Read contents of entry.
   if (!fChain) return 0;
   return fChain->GetEntry(entry);
}
Long64_t evt::LoadTree(Long64_t entry)
{
// Set the environment to read one entry
   if (!fChain) return -5;
   Long64_t centry = fChain->LoadTree(entry);
   if (centry < 0) return centry;
   if (fChain->GetTreeNumber() != fCurrent) {
      fCurrent = fChain->GetTreeNumber();
      Notify();
   }
   return centry;
}

void evt::Init(TTree *tree)
{
   // The Init() function is called when the selector needs to initialize
   // a new tree or chain. Typically here the branch addresses and branch
   // pointers of the tree will be set.
   // It is normally not necessary to make changes to the generated
   // code, but the routine can be extended by the user if needed.
   // Init() will be called many times when running on PROOF
   // (once per file to be processed).

   // Set branch addresses and branch pointers
   if (!tree) return;
   fChain = tree;
   fCurrent = -1;
   fChain->SetMakeClass(1);

   fChain->SetBranchAddress("run", &run, &b_run);
   fChain->SetBranchAddress("event", &event, &b_event);
   fChain->SetBranchAddress("gtc", &gtc, &b_gtc);
   fChain->SetBranchAddress("abcid", &abcid, &b_abcid);
   fChain->SetBranchAddress("nframe", &nframe, &b_nframe);
   fChain->SetBranchAddress("frame", frame, &b_frame);
   fChain->SetBranchAddress("info", &info, &b_info);
   Notify();
}

Bool_t evt::Notify()
{
   // The Notify() function is called when a new file is opened. This
   // can be either for a new TTree in a TChain or when when a new TTree
   // is started when using PROOF. It is normally not necessary to make changes
   // to the generated code, but the routine can be extended by the
   // user if needed. The return value is currently not used.

   return kTRUE;
}

void evt::Show(Long64_t entry)
{
// Print contents of entry.
// If entry is not specified, print current entry
   if (!fChain) return;
   fChain->Show(entry);
}
Int_t evt::Cut(Long64_t entry)
{
// This function may be called from Loop.
// returns  1 if entry is accepted.
// returns -1 otherwise.
   return 1;
}
#endif // #ifdef evt_cxx
