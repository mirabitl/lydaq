#define evt_cxx
#include "evt.hh"
#include "jsonGeo.hh"
#include "RecoFrame.hh"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <stdint.h>
#include <algorithm>
#include "TdcAnalyzer.hh"
/**
 * \file evt.C
 * \brief Main analysis class
 * \author L.Mirabito
 * \version 0.1
 * \date 15 septembre 2017
 *
 * Simple example reading SDHCAL H2 Spetember 2017 data
 *
 */
using namespace std;

void evt::Loop()
{
  //  Create the map
  _mezMap.clear();
  for (uint32_t i=1;i<255;i++)
    { 
      _mezMap.insert(std::pair<uint32_t,std::vector<lydaq::TdcChannel> >(i,std::vector<lydaq::TdcChannel>()));}
  //by  b_branchname->GetEntry(ientry); //read only this branch
  if (fChain == 0) return;
 
  Long64_t nentries = fChain->GetEntriesFast();

  Long64_t nbytes = 0, nb = 0,nft=0,nfd[255],nfa[255][48],abcid0=0;
  float fulltime=0;
  memset(nfd,0,255*sizeof(Long64_t));
  memset(nfa,0,255*48*sizeof(Long64_t));
  lydaq::TdcAnalyzer* a= new lydaq::TdcAnalyzer(_rh);
  for (Long64_t jentry=0; jentry<nentries;jentry++) {
    Long64_t ientry = LoadTree(jentry);
    if (ientry < 0) break;
    nb = fChain->GetEntry(jentry);   nbytes += nb;
    // if (Cut(ientry) < 0) continue;
    Long64_t bcm=0;
    if (abcid==-1 && gtc==-1)
      {
	_runType=info[0];
	if (_runType==1)
	  _dacSet=info[1];
	if (_runType==2)
	  _vthSet=info[1];

	printf("Run header %d %d  %d \n",_runType,_dacSet,_vthSet);
	//getchar();
	continue;
      }
    if (abcid0==0 && abcid>0 ) abcid0=abcid;
    for (uint32_t i=1;i<255;i++)
      if (_mezMap[i].size()>0) _mezMap[i].clear();
    for (Int_t i=0;i<nframe;i++)
      {
	uint32_t mez=(info[i]>>32)&0xFFFFFFFF;
	lydaq::TdcChannel f((uint8_t*)&frame[i]);
	uint8_t* c=(uint8_t*)&frame[i];
	// for (int j=0;j<8;j++)
	//   printf("%2x ",c[j]);
	// printf("\n");
	_mezMap[mez].push_back(f);
      }
    //getchar();
    a->setInfo(run,event,gtc,abcid,TDC_TRIGGER_CHANNEL,_vthSet,_dacSet);
    for (uint32_t i=1;i<255;i++)
      if (_mezMap[i].size()>0)
	{
	  if (_runType==1) a->pedestalAnalysis(i,_mezMap[i]);
	  if (_runType==2) a->scurveAnalysis(i,_mezMap[i]);
	  if (_runType==0) a->normalAnalysis(i,_mezMap[i]);
	}

  }
  a->end();
}
static TCanvas* TCPlot=NULL;
static TCanvas* TCHits=NULL;
static TCanvas* TCShower=NULL;
static TCanvas* TCEdge=NULL;
static TCanvas* TCHT=NULL;
static TCanvas* TCCluster=NULL;
void evt::drawHits()
{
  
  TH3* hcgposi = _rh->GetTH3("InstantHitMap");
  if (hcgposi==NULL)
    {
      hcgposi =_rh->BookTH3("InstantHitMap",200,-10.,150.,120,-10.,110.,120,-10.,110.);
    }
  else
    {
      hcgposi->Reset();
    }
  
  TH2* hpix = _rh->GetTH2("ix");
  TH2* hpiy = _rh->GetTH2("iy");
  TH2* hpex = _rh->GetTH2("ex");
  TH2* hpey = _rh->GetTH2("ey");
  TH2* hpmx = _rh->GetTH2("mx");
  TH2* hpmy = _rh->GetTH2("my");
  TH2* hpax = _rh->GetTH2("ax");
  TH2* hpay = _rh->GetTH2("ay");
  
  
  if (hpix==NULL)
    {
    
      hpix =_rh->BookTH2("ix",200,-10.,150.,120,-10.,120.);
      hpiy =_rh->BookTH2("iy",200,-10.,150.,120,-10.,120.);
      hpex =_rh->BookTH2("ex",200,-10.,150.,120,-10.,120.);
      hpey =_rh->BookTH2("ey",200,-10.,150.,120,-10.,120.);
      hpmx =_rh->BookTH2("mx",200,-10.,150.,120,-10.,120.);
      hpmy =_rh->BookTH2("my",200,-10.,150.,120,-10.,120.);
      hpax =_rh->BookTH2("ax",200,-10.,150.,120,-10.,120.);
      hpay =_rh->BookTH2("ay",200,-10.,150.,120,-10.,120.);
    
    }
  else
    {
  
      hpix->Reset();
      hpiy->Reset();
      hpex->Reset();
      hpey->Reset();
      hpmx->Reset();
      hpmy->Reset();
      hpax->Reset();
      hpay->Reset();
    }
  
  if (hcgposi!=0 )
    {
      hcgposi->Reset();
      // Fill it
      
      if (TCHits==NULL)
	{
	  TCHits=new TCanvas("TCHits","tChits1",900,900);
	  TCHits->Modified();
	  TCHits->Draw();
	  TCHits->Divide(2,2);
	}
      TCHits->cd(1);
      hcgposi->SetMarkerStyle(25);
      hcgposi->SetMarkerColor(kRed);
      hcgposi->Draw("P");
   
      TCHits->cd(2);
  
      hcgposi->SetMarkerStyle(25);
      hcgposi->SetMarkerColor(kRed);
      //      hdisp->Draw("COLZ");
      TH1* hweight= _rh->GetTH1("/HitStudy/showerweight");
      //hweight->Reset();
      //for (std::vector<RecoHit>::iterator ih=_hits.begin();ih!=_hits.end();ih++)
      //	if (_pF[i].weight()>1E-2)
      //	  hweight->Fill(_pF[i].weight());
      if (hweight!=0)
	hweight->Draw();
      TCHits->Modified();
      TCHits->Draw();
    
    
      TCHits->cd(3);
      /*
       * 
       *      hdx->Draw();
       *      TCHits->cd(4);
       *      hdy->Draw();
       * 
       */
      hpix->SetLineColor(kGreen);
      hpix->Draw("BOX");
      hpex->SetLineColor(kMagenta);
      hpex->Draw("BOXSAME");
      
      hpmx->SetLineColor(kRed);
      hpmx->Draw("BOXSAME");
      hpax->SetLineColor(kBlue);
      hpax->Draw("BOXSAME");
    
    
  
      TCHits->cd(4);
    
    
      hpiy->SetLineColor(kGreen);
    
      hpiy->Draw("BOX");
      hpey->SetLineColor(kMagenta);
      hpey->Draw("BOXSAME");

      hpmy->SetLineColor(kRed);
      hpmy->Draw("BOXSAME"); 
      hpay->SetLineColor(kBlue);
      hpay->Draw("BOXSAME");
   
      
    
      TCHits->Modified();
      TCHits->Draw();
      TCHits->Update();
    }
  
}
