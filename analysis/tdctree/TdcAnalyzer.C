#include "TdcAnalyzer.hh"
#include "jsonGeo.hh"
#include "RecoFrame.hh"
#include <TH2.h>
#include <TStyle.h>
#include <TCanvas.h>
#include <stdint.h>
#include <algorithm>
#include <iostream>
#include <sstream>
#ifdef LYON
int tdc2stripf[16]={255,1,255,2,255,3,255,255,255,255,255,255,255,255,255,255};
int tdc2stripb[16]={255,255,255,255,255,255,255,2,255,1,255,255,255,3,255,255};
#endif
int tdc2stripf[16]={255,255,1,255,2,255,255,255,3,255,255,255,255,255,255,255};
int tdc2stripb[16]={255,2,255,255,255,3,255,1,255,255,255,255,255,255,255,255};
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
lydaq::TdcAnalyzer::TdcAnalyzer(DCHistogramHandler*r ) : _rh(r),_pedestalProcessed(false),_nevt(0),_ntrigger(0),_nfound(0),_nbside(0)
{
}
void lydaq::TdcAnalyzer::setInfo(uint32_t run,uint32_t ev,uint32_t gt,uint32_t ab,uint16_t trgchan,uint32_t vth,uint32_t dac)
{_run=run;
  _event=ev;
  _gtc=gt;
  _abcid=ab;
  _triggerChannel=trgchan;
  _vthSet=vth;
  _dacSet=dac;
  
}

void lydaq::TdcAnalyzer::pedestalAnalysis(uint32_t mezId,std::vector<lydaq::TdcChannel>& vChannel)
{
  _pedestalProcessed=true;

  //if (gtc[mezId-1]
  std::cout<<"Mezzanine "<<mezId<<"Event "<<_event<<" GTC"<<_gtc<<" hits"<<vChannel.size()<<" Trigger channel "<<_triggerChannel<<std::endl;

  // Analyze
  std::stringstream sr;
  sr<<"/run"<<_run<<"/TDC"<<mezId<<"/";

  uint32_t dac =_dacSet;
  for (int ich=0;ich<_triggerChannel+1;ich++)
    {
 
      std::stringstream src;
      src<<sr.str()<<"dac"<<ich;
      TH1* hdac=_rh->GetTH1(src.str());
      if (hdac==NULL)
	{
	 
	  hdac=_rh->BookTH1(src.str(),64,0.,64.);
	}
      bool found=false;
      double lastf=0;
      for (std::vector<lydaq::TdcChannel>::iterator x=vChannel.begin();x!=vChannel.end();x++)
	{
	  if (x->channel()==ich) {
	    //printf("%d %d %f \n",x.channel(),x.bcid(),x.tdcTime());
	    double dt=x->tdcTime()-lastf;
	    lastf=x->tdcTime();
	    if (dt>25 || dt<0)
	      hdac->Fill(dac*1.);
	  }//break;}
	}
    }

}
void lydaq::TdcAnalyzer::scurveAnalysis(uint32_t mezId,std::vector<lydaq::TdcChannel>& vChannel)
{

  //if (gtc[mezId-1]
  std::cout<<"Mezzanine "<<mezId<<"Event "<<_event<<" GTC"<<_gtc<<" hits"<<vChannel.size()<<" Vth set "<<_vthSet<<" Trigger channel "<<_triggerChannel<<std::endl;

  // Analyze
  std::stringstream sr;
  sr<<"/run"<<_run<<"/TDC"<<mezId<<"/";

  uint32_t vth =_vthSet;
  for (int ich=0;ich<_triggerChannel+1;ich++)
    {
 
      std::stringstream src;
      src<<sr.str()<<"vth"<<ich;
      TH1* hvth=_rh->GetTH1(src.str());
      if (hvth==NULL)
	{
	 
	  hvth=_rh->BookTH1(src.str(),900,0.,900.);
	  printf("Booking %s \n",src.str().c_str());
	}
      bool found=false;
      double lastf=0;
      for (std::vector<lydaq::TdcChannel>::iterator x=vChannel.begin();x!=vChannel.end();x++)
	{
	  //printf("%d hit\n",x->channel());
	  if (x->channel()==ich) {
	    //printf("Scurve %s %d %d %f \n",src.str().c_str(),x->channel(),x->bcid(),x->tdcTime());
	    double dt=x->tdcTime()-lastf;
	    lastf=x->tdcTime();
	    //  if (dt>25 || dt<0)
	    hvth->Fill(vth*1.);
	    break;
	  }
	}
    }

}
void lydaq::TdcAnalyzer::normalAnalysis(uint32_t mezId,std::vector<lydaq::TdcChannel>& vChannel)
{
  this->LmAnalysis(mezId,vChannel);
}

void lydaq::TdcAnalyzer::end()
{

  if (_pedestalProcessed)
    {
      for (int mez=1;mez<=255;mez++)
	{
	  std::stringstream sr;
	  
	  sr<<"/run"<<_run<<"/TDC"<<mez<<"/";

	  int ipr=0;
	  for (int ich=0;ich<32;ich++)
	    {

	      if (ich%2==0)
		ipr=ich/2;
	      else
		ipr=31-ich/2;
	      std::stringstream src;
	      src<<sr.str()<<"dac"<<ich;
	      TH1* hdac=_rh->GetTH1(src.str());
	      if (hdac==NULL) continue;
	      int ped=31;
	      if (hdac!=NULL)
		{
		  printf("Mezzanine %d Channel %d Mean %f RMS %f \n",mez,ich,hdac->GetMean(),hdac->GetRMS());
		  ped=int(hdac->GetMean());
		  if (hdac->GetRMS()>6)
		    {
		      printf("\t \t ======================>ILL %d \n",ipr);
		      ped-=int(hdac->GetRMS());
		    }
		  // if (mez==1)
		  //   _s1.set6bDac(ipr,ped&0xFF);
		  // else
		  //   _s2.set6bDac(ipr,ped&0xFF);
		}
	      // else
	      // 	if (mez==1)
	      // 	  _s1.set6bDac(ipr,31);
	      // 	else
	      // 	  _s2.set6bDac(ipr,31);

	      if (ped==0)
		{printf("\t \t ======================>DEAD %d \n",ipr);
		  ped=31;
		}
	      printf("\t %d %d \n",ipr,ped);
	    }
	}
      // _s1.toJson();
      // _s1.dumpJson();
      // _s2.toJson();
      // _s2.dumpJson();

    }
  



  std::stringstream sr;
  sr<<"/tmp/toto"<<_run<<".root";
  
  _rh->writeHistograms(sr.str());


}

void lydaq::TdcAnalyzer::LmAnalysis(uint32_t mezId,std::vector<lydaq::TdcChannel>& vChannel)
{
  
  if (vChannel.size()>4096) return; // Skip noise event
    double tmi=1E33,tma=-1E33;
   for (std::vector<lydaq::TdcChannel>::iterator it=vChannel.begin();it!=vChannel.end();it++)
     {
       if (it->tdcTime()<tmi) tmi=it->tdcTime();
       if (it->tdcTime()>tma) tma=it->tdcTime();
     }
   if (abs(tma-tmi)>1E4) return;

  // Analyze

  std::stringstream sr;
  std::stringstream difname;
  std::stringstream runname;
  difname<<mezId;
  runname<<_run;
  sr<<"/run"<<_run<<"/TDC"<<mezId<<"/LmAnalysis/";
  TH2* hpos=_rh->GetTH2(sr.str()+"Position");
  TH1* hdt=_rh->GetTH1(sr.str()+"DeltaT");
  TH1* hdtr0=_rh->GetTH1(sr.str()+"DeltaTr0");

  TH2* hdtr=_rh->GetTH2(sr.str()+"DeltaTr");
  TH2* hdtra=_rh->GetTH2(sr.str()+"DeltaTrAll");
  TH1* hns=_rh->GetTH1(sr.str()+"NChannel");
  TH1* hnst=_rh->GetTH1(sr.str()+"NChannelTrigger");
  TH1* hfin=_rh->GetTH1(sr.str()+"Fine");
  TH1* heff=_rh->GetTH1(sr.str()+"Efficiency");
  TH1* hstrip=_rh->GetTH1(sr.str()+"Strips");
  TH1* hstript=_rh->GetTH1(sr.str()+"Stript");
  TH1* hnstrip=_rh->GetTH1(sr.str()+"NStrips");
  TH1* hxp=_rh->GetTH1(sr.str()+"XP");
  TH1* hti=_rh->GetTH1(sr.str()+"time");
  TH1* hra=_rh->GetTH1(sr.str()+"rate");
  if (hpos==NULL)
    {
      hpos=_rh->BookTH2(sr.str()+"Position",40,0.,20.,200,-100.,100.);
      hdt=_rh->BookTH1(sr.str()+"DeltaT",500,-10.,10.);

      hdtr=_rh->BookTH2(sr.str()+"DeltaTr",32,0,32.,400,-1000.,-800.);
      hdtr0=_rh->BookTH1(sr.str()+"DeltaTr0",10000,-1000.,2500.);
      hdtra=_rh->BookTH2(sr.str()+"DeltaTrAll",32,0,32.,500,-20.,20.);
      hns=_rh->BookTH1(sr.str()+"NChannel",1024,0.,1024.);
      hnst=_rh->BookTH1(sr.str()+"NChannelTrigger",1024,0.,1024.);
      hfin=_rh->BookTH1(sr.str()+"Fine",257,0.,257.);
      heff=_rh->BookTH1(sr.str()+"Efficiency",32,0.,32.);
      hstrip=_rh->BookTH1(sr.str()+"Strips",32,0.,32.);
      hstript=_rh->BookTH1(sr.str()+"Stript",32,0.,32.);
      hnstrip=_rh->BookTH1(sr.str()+"NStrips",32,0.,32.);
      hxp=_rh->BookTH1(sr.str()+"XP",400,0.,10.);
      hti=_rh->BookTH1(sr.str()+"time",400,0.,0.02);
      hra=_rh->BookTH1(sr.str()+"rate",750,0.,200000.);

    }
  hns->Fill(vChannel.size()*1.);
  if (vChannel.size()>15) return;
  double shift1[32]={32*0};
		    
  double shift2[32]={32*0};


  _nevt++;
  heff->Fill(0.1);
  
  uint32_t ndeclenchement=0;
  
  float ti=0,tmax=0;
  uint32_t lbcid=0,bcidshift=0,bcidmax=0;
  int32_t tbcid=0;
  printf("Event %d DIF %d GTC %d ABCID %lu Size %d  \n",_event,mezId,_gtc,_abcid,vChannel.size());
  for (std::vector<lydaq::TdcChannel>::iterator it=vChannel.begin();it!=vChannel.end();it++)
    {
      hstrip->Fill(it->channel()*1.+0.5);
      if (it->bcid()>bcidmax) bcidmax=it->bcid();
      // if (it->bcid()<lbcid) {
      //   printf("lbcid %d bcid %d  Shift %d \n",lbcid,it->bcid(),b
      //   bcidshift+=65535;
      // }
      lbcid=it->bcid();
      float t=((int) it->bcid()*2E-7)+(bcidshift*2.E-7)-ti;
      if (t>tmax) tmax=t;
      // Find trigger bcid ( last one)
      if (it->channel()==_triggerChannel) {
	ndeclenchement++; tbcid=it->bcid();
	printf("Event %d DIF %d GTC %d ABCID %lu BCID trigger %d # %d \n",_event,mezId,_gtc,_abcid,tbcid,ndeclenchement);
      }
      //printf("%d %d %d %d %d  %f \n",_gtc,it->channel(),it->coarse(),it->fine(),it->bcid(),it->tdcTime());
    }
  //getchar();
  //printf("BCID max %d Bcidshift %d Tmax %f \n",bcidmax,bcidshift,tmax);
  // if (tmax==0 && vChannel.size()>0)
  //   {
  //     for (std::vector<lydaq::TdcChannel>::iterator it=vChannel.begin();it!=vChannel.end();it++)
  // 	std::cout<<(int) it->channel()<<" "<<it->coarse()*2.5E-9<<" "<<it->bcid()*2E-7<<endl;
  //     //getchar();
  //   }
  // Strore the maximum time of acquisition
  if (ndeclenchement==0) hti->Fill(tmax);
  // Calculate channel occupancy
  if (tmax>0 && ndeclenchement==0) hra->Fill(vChannel.size()/tmax);
  // Accept events with only one trigger
  if (ndeclenchement!=1) return;
  // Find the trigger

#ifdef TIMECORR
  Json::Reader reader;
  std::ifstream ifs ("run735887.json", std::ifstream::in);
  Json::Value jall;
  bool parsingSuccessful = reader.parse(ifs,jall,false);
  //std::cout<<jall<<std::endl;
#endif

  // Map of tdc channels per declenchement
  std::map<uint32_t,std::vector<lydaq::TdcChannel> > _trmap;

  _trmap.clear();
  // Fill bcid distance of hits wrt to trigger
  for (std::vector<lydaq::TdcChannel>::iterator it=vChannel.begin();it!=vChannel.end();it++)
    {
      it->setUsed(false);
      if (it->channel()!=_triggerChannel) hdtra->Fill(it->channel(),(it->bcid()-tbcid)*1.);
    }
  
  for (std::vector<lydaq::TdcChannel>::iterator it=vChannel.begin();it!=vChannel.end();it++)
    {
      // Find trigger channel
      if (it->channel()==_triggerChannel)
	{
	  it->setUsed(true);
	  std::vector<lydaq::TdcChannel> vc;
	  vc.push_back(*it);

	  // Loop on hits and find nearby channels hits
	 
	  for (std::vector<lydaq::TdcChannel>::iterator x=vChannel.begin();x!=vChannel.end();x++)
	    {
	      if (x->used()) continue;
	      if (x->channel() == _triggerChannel) continue;
	      if (x->bcid()>(it->bcid()+8) || x->bcid()<(it->bcid()-8)) continue;
	      //#ifdef TIMECORR
	      //if ((x->tdcTime()-it->tdcTime())<jall[difname.str()]["low"][x->channel()].asInt()) continue;
	      //if ((x->tdcTime()-it->tdcTime())>jall[difname.str()]["high"][x->channel()].asInt()) continue;
	      //#endif
	     
	      vc.push_back((*x));
	      x->setUsed(true);
	    }
	  // Insert bcid, vector of hits in the trigger map
	  std::pair<uint32_t,std::vector<lydaq::TdcChannel> > p(it->bcid(),vc);
	  _trmap.insert(p);
	}

    }

  // Now loop on all channel of the event
  for (std::vector<lydaq::TdcChannel>::iterator it=vChannel.begin();it!=vChannel.end();it++)
    {
      hstrip->Fill(it->channel()*1.);
    }

  if (_trmap.size()>0) printf("TDC %d  GTC %d   Number %d \n",mezId,_gtc,_trmap.size());
  bool found=false;
  bool bside=false;
  int nt=0;
  for ( std::map<uint32_t,std::vector<lydaq::TdcChannel> >::iterator t=_trmap.begin();t!=_trmap.end();t++)
    {
      //if (nt) break;
      nt++;
      printf("Trigger %d => channels %d  \n",t->first,t->second.size());
      double trigtime=0,trigbcid=0;
      bool chused[32]={32*0};
      bool sused[96]={96*0};
      for (int i=0;i<32;i++) {chused[i]=false;}
      for (int i=0;i<96;i++){sused[i]=false;}
      std::sort(t->second.begin(),t->second.end());
      double tev0=0;
      std::vector<lydaq::TdcChannel>::iterator ittrig=t->second.end();
      std::vector<lydaq::TdcChannel>::iterator itchan=t->second.end();
      for (std::vector<lydaq::TdcChannel>::iterator x=t->second.begin();x!=t->second.end();x++)
	{
	  x->setUsed(false);
	  if (x->channel()==_triggerChannel)
	    {chused[_triggerChannel]=1;trigtime=x->tdcTime();trigbcid=x->bcid()*200;x->setUsed(true);ittrig=x;
	    }
	  else
	    if (tev0==0) {tev0=x->tdcTime();itchan=x;}

	}
      uint32_t nch=0;
      for (std::vector<lydaq::TdcChannel>::iterator it=t->second.begin();it!=t->second.end();it++)

	{
	  if (abs(it->tdcTime()-tev0)>50) it->setUsed(true);
	  if (it->used()) continue;
	  //printf("%d %u %u %u %f \n",x->channel(),x->coarse(),x->fine(),x->bcid(),x->tdcTime()-tev0);
	  nch++;
	}
      //      getchar();
      if (itchan==t->second.end()) continue;
      if (ittrig==t->second.end()) continue;
      printf("Trigtime %f %u %d tev0 %f %f %u %d \n",trigtime,ittrig->coarse(),ittrig->fine(),tev0,trigtime-tev0,itchan->coarse(),itchan->fine());
      //getchar();
      hdtr0->Fill(tev0-trigtime);
      hnst->Fill(nch*1.);
      if (nch>=1)  heff->Fill(4.1);
      //printf(" Effective TDC %d  GTC %d   Number %d \n",mezId,_gtc,t->second.size());
      if (t->second.size()>2000) continue; // Use trigger with less than  20 strip

      for (std::vector<lydaq::TdcChannel>::iterator x=t->second.begin();x!=t->second.end();x++)
	{
	  //printf("Chan %d bcid %d Time %f %f \n",x->channel(),x->bcid(),x->tdcTime(),trigtime);
	  if (x->channel()==_triggerChannel) continue;
	  if (x->used()) continue;
	  int str=LEMO2STRIP[PR2LEMO[TDC2PR[x->channel()]]];
		  
	  if (str==0xFF) continue;
	  //if (x->tdcTime()-trigtime>-861) continue;
	  //if (x->tdcTime()-trigtime<-900) continue;
	  found=true;


	  
	  //if (tdc2stripb[x->channel()]!=str) continue;
	  printf("%d %u %u %u %f \n",x->channel(),x->coarse(),x->fine(),x->bcid(),x->tdcTime()-tev0);
	  std::stringstream s;
	  s<<"hdtz"<<(int) x->channel();
	  TH1* hdtz=_rh->GetTH1(sr.str()+s.str());
	  if (hdtz==NULL)
	    {
	      hdtz=_rh->BookTH1(sr.str()+s.str(),500,-30.,30.);
	    }

	  hdtz->Fill(x->tdcTime()-tev0);
	  //if (x->tdcTime()-tev0>7.5) continue;	 
	  hdtr->Fill(x->channel()+1.,x->tdcTime()-trigtime);
	  //hdtr->Fill(x->channel(),x->bcid()*200-trigbcid);

	  for (std::vector<lydaq::TdcChannel>::iterator y=t->second.begin();y!=t->second.end();y++)
	    {
	      if (y->used()) continue;
	      if (x->channel()==y->channel()) continue;
	      if (LEMO2STRIP[PR2LEMO[TDC2PR[y->channel()]]]!=str) continue;
	      //if (mezId==1 && y->channel()!=(x->channel()+1)) continue;
	      //if (mezId==2 && y->channel()!=(x->channel()-1)) continue;
	      x->setUsed(true);
	      y->setUsed(true);
	      /*
	      printf("\t First channel %d %ld %d %f \n",x->channel(),x->coarse(),x->fine(),x->tdcTime());
	      x->dump();
	      printf("\t Second channel %d %ld %d %f \n",y->channel(),y->coarse(),y->fine(),y->tdcTime());
	      y->dump();
	      getchar();
	      */
	      //if (y->channel()!=(x->channel()+1)) continue;
	      double t0=x->tdcTime();
	      double t1=y->tdcTime();
	      //if (t1<t0) continue;
	      //if (mezId%2==0 && t1>t0) continue;
	      //if (mezId%2==1 && t0>t1) continue;
	      ///if (it->coarse()!=jt->coarse()-1) continue;
	      //if (abs(t1-t0)<1) continue;
	      chused[x->channel()]=true;
	      chused[y->channel()]=true;
	      sused[str]=true;
	    
	      double tsh;//=jall["100000"][difname.str()]["shift"][str].asFloat();
	      tsh=0;
	      lydaq::TdcStrip strip(mezId,str,t0,t1,tsh);
	      _strips.push_back(strip);
	      //printf("%d %s %s %f \n",str,runname.str().c_str(),difname.str().c_str(),tsh);
	      //std::cout<<jall["100000"][difname.str()]["shift"]<<std::endl;
	      hdt->Fill((t0-t1)-tsh);
	      std::stringstream s;
	      s<<"hdts"<<str;
	      TH1* hdts=_rh->GetTH1(sr.str()+s.str());
	      if (hdts==NULL)
		{
		  hdts=_rh->BookTH1(sr.str()+s.str(),2500,-25.,25.);
		}
	      std::stringstream sx;
	      sx<<"hdtr"<<(int) x->channel();
	      TH1* hdtrx=_rh->GetTH1(sr.str()+sx.str());
	      if (hdtrx==NULL)
		{
		  hdtrx=_rh->BookTH1(sr.str()+sx.str(),400,-200.,-200.);
		}
	      hdts->Fill(t0-t1);
	      printf("%f %f => %f\n",t0,t1,t0-t1);
	      hdtrx->Fill(t0-trigtime);
	      std::stringstream sry;
	      sry<<"hdtr"<<(int) y->channel();
	      TH1* hdtry=_rh->GetTH1(sr.str()+sry.str());
	      if (hdtry==NULL)
		{
		  hdtry=_rh->BookTH1(sr.str()+sry.str(),400,-200.,-200.);
		}
	      hdtry->Fill(t1-trigtime);
	      double x=str*0.4+0.2;
	      double yp=((t0-t1)-tsh)*100./12.;
	      //std::cout<<x<<" "<<y<<std::endl;
	      bside=true;
	      hpos->Fill(x,yp);
	      break;
	    }     
	}
    }
  //if (mezId==15 ) getchar();
  //getchar();
  _ntrigger++;
  heff->Fill(1.1);
  if (!found) return;
  _nfound++;
  // update efficency
  heff->Fill(2.1);
  if (bside) {_nbside++;heff->Fill(3.1);}
  printf("%d-%d %d  #evt %d #dif %d #trig %d #found %d  #time %d \n",_run,_event,_gtc,_nevt,mezId,_ntrigger,_nfound,_nbside); 
}

