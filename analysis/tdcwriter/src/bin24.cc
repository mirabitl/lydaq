#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"
#include <stdint.h>
#include "zmBuffer.hh"
#include "zmMerger.hh"
#include "json/json.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

// This example illustrates how to make a Tree from variables or arrays
// in a C struct. Use of C structs is strongly discouraged and one should
// use classes instead. However support for C structs is important for
// legacy applications written in C or Fortran.
//    see tree2a.C for the same example using a class instead of a C-struct.
//
// In this example, we are mapping a C struct to one of the Geant3
// common blocks /gctrak/. In the real life, this common will be filled
// by Geant3 at each step and only the Tree Fill function should be called.
// The example emulates the Geant3 step routines.
//
// to run the example, do:
// .x tree2.C   to execute with the CINT interpreter
// .x tree2.C++ to execute with native compiler
//
//  Author: Rene Brun

const Int_t MAXFRAME = 48*3*128*50*2;
const Int_t MAXSTRIP = 192*128*20;

//      PARAMETER (MAXMEC=30)
//      COMMON/GCTRAK/VECT(7),GETOT,GEKIN,VOUT(7),NMEC,LMEC(MAXMEC)
//     + ,NAMEC(MAXMEC),NSTEP ,PID,DESTEP,DESTEL,SAFETY,SLENG
//     + ,STEP  ,SNEXT ,SFIELD,TOFG  ,GEKRAT,UPWGHT
typedef struct {
  Int_t run;
  Int_t event;
  Int_t gtc;
  Long_t abcid;
  Int_t    nframe;
  Long_t   frame[MAXFRAME];
  Long_t    info[MAXFRAME];
} bin24Event_t;
using namespace zdaq;

namespace branalysis
{
 


  class bin24writer : public zdaq::zmprocessor
  {
  public:
    bin24writer(std::string dire="/tmp") : _directory(dire),_run(0),_started
					   (false),_fdOut(NULL),_totalSize(0),_event(0) {
      memset(temp,0,100000);
      _tree=NULL;
    }

    
    virtual void start(uint32_t run)
    {_gEvent.run=run;      memset(temp,0,100000);

      this->createTree();
      _gEvent.event=0;}
    virtual void stop()
    {
      printf("bin2writer::stop \n");
      this->closeTree();
    }
    virtual  void processEvent(uint32_t key,std::vector<zdaq::buffer*> dss)
    {
      //return;
      _gEvent.nframe=0;
      _gEvent.event++;
      uint32_t* itemp=(uint32_t*) temp;
      uint32_t idx=28;
      for (std::vector<zdaq::buffer*>::iterator it=dss.begin();it!=dss.end();it++)
	{
	  zdaq::buffer* x=(*it);
	  //std::cout<<"Before "<<x->size()<<std::endl<<std::flush;
	  x->uncompress();
	  // std::cout<<"After "<<x->size()<<" "<<x->payloadSize()
	  // 	   <<" Ev "<<x->eventId()
	  // 	   <<" Bx "<<x->bxId()
	  // 	   <<std::hex<<" ptr "<<(int64_t) x->payload()<<std::dec
	  // 	   <<std::endl<<std::flush;
	  
	  _gEvent.gtc=x->eventId();
	  _gEvent.abcid=x->bxId();

	  if (x->payloadSize()<idx) continue;
	  if (x->payloadSize()>0x100000)
	    {
	      std::cout<<"INVALID payload "<<x->payloadSize()<<std::endl<<std::flush;
	      continue;
	    }
	  memcpy(&temp[0],(unsigned char*) x->payload(),x->payloadSize());
	  //continue;
	  uint32_t mezzanine=itemp[4];
	  uint32_t nch=itemp[6];
	  uint8_t* chans=&temp[idx];
	  uint64_t* lchans= (uint64_t*) chans;
	  //std::cout<<" channels "<<nch<<" for pls "<<x->payloadSize()<<std::endl<<std::flush;
	 
	  if (nch>(x->payloadSize()-idx)/8)
	    {
	      std::cout<<"INVALID channels "<<nch<<" for pls "<<x->payloadSize()<<std::endl<<std::flush;
	      continue;
	    }
	  for (int i=0;i<nch;i++)
	    {
	      _gEvent.frame[_gEvent.nframe]=lchans[i];
	      _gEvent.info[_gEvent.nframe]= ((uint64_t) mezzanine)<<32|(nch<<16)&0XFFFF|i;
	      _gEvent.nframe++;
	    }

	  _tree->Fill();
	}
    }
 
    virtual  void processRunHeader(std::vector<uint32_t> header)
    {
      _gEvent.abcid=-1;
      _gEvent.gtc=-1;
      _gEvent.nframe=header.size();
      for (int i=0;i<_gEvent.nframe;i++)
	{_gEvent.info[i]=header[i];_gEvent.frame[i]=0;}
      _tree->Fill();

      return;}
    virtual void loadParameters(Json::Value params)
    {
      _directory=params["directory"].asString();
    }
    void readBinaryFile(std::string filename)
    {
      int _fdIn= ::open(filename.c_str(), O_RDONLY | O_NONBLOCK,S_IRWXU);
      if (_fdIn<0)
	{
	  perror("No way to store to file :");
	  //std::cout<<" No way to store to file"<<std::endl;
	  return;
	}  
      _gEvent.event=0;
      _started=true;

      std::vector<zdaq::buffer*> vbuf;
      vbuf.clear();
        

      while (_started)
	{
	  if (!_started) return;
	  uint32_t theNumberOfDIF=0;
	  // To be implemented
	  if (_fdIn>0)
	    {
 
	      int ier=::read(_fdIn,&_gEvent.event,sizeof(uint32_t));
	      if (ier<=0)
		{
		  printf("Cannot read anymore %d \n ",ier);break;
		}
	      else
		printf("Event read %d \n",_gEvent.event);
      
	      ier=::read(_fdIn,&theNumberOfDIF,sizeof(uint32_t));
	      if (ier<=0)
		{
		  printf("Cannot read anymore number of DIF %d \n ",ier);break;
		}
	      else
		printf("Number of DIF found %d \n",theNumberOfDIF);

	      for (uint32_t idif=0;idif<theNumberOfDIF;idif++) 
		{
		  zdaq::buffer* b=NULL;
		  if (vbuf.size()<idif+1)
		    {b=new  zdaq::buffer(0x100000);vbuf.push_back(b);}
		  b=vbuf[idif];
		  //printf("\t writing %d bytes",idata[SHM_BUFFER_SIZE]);
		  //(*iv)->compress();
		  uint32_t bsize=0;
		  // _totalSize+=bsize;
		  ier=::read(_fdIn,&bsize,sizeof(uint32_t));
		  if (ier<=0)
		    {
		      printf("Cannot read anymore  DIF Size %d \n ",ier);return;
		    }
		  //else
		  //  printf("\t DIF size %d \n",bsize);
	      
		  ier=::read(_fdIn,b->ptr(),bsize);
		  if (ier<=0)
		    {
		      printf("Cannot read anymore Read data %d \n ",ier);return;
		    }
		  b->setPayloadSize(bsize-(3*sizeof(uint32_t)+sizeof(uint64_t)));
		  b->uncompress();
		  //memcpy(&_buf[_idx], b.payload(),b.payloadSize());
		  //printf("\t \t %d %d %d %x %d %d %d\n",b->detectorId(),b->dataSourceId(),b->eventId(),b->bxId(),b->payloadSize(),bsize,idif);
		  if (b->detectorId()!=120) continue;
		  if (idif==0)
		    {
		      _gEvent.abcid=b->bxId();
		      _gEvent.gtc=b->eventId();
		    }
		 
		} // End of loop on DIF

	      this->processEvent(_gEvent.abcid,vbuf);
	      printf("\t Summary %d %ld  has %d frames \n",_gEvent.gtc,_gEvent.abcid,_gEvent.nframe);
	      
	    } //end of test on filedescriptor
      
	} //End of loop
      // delete zdaq::buffer
      for (auto x:vbuf)
	delete x;

      
      _started=false;
      ::sleep(1);
      if (_fdIn>0)
	{
	  ::close(_fdIn);
	  _fdIn=-1;
	}

    }
    uint32_t totalSize(){return _totalSize;}
    uint32_t eventNumber(){return _event;}
    uint32_t runNumber(){return _run;}
    bin24Event_t* event(){return &_gEvent;}
    void createTree()
    {
      std::cout<<" On rentre dasn createTree \n"<<std::flush;
      if (_fdOut!=NULL)
	{
	  if (_tree)
	    _tree->Write();
	  _fdOut->Close();
	  printf("On detruit fdOut \n");fflush(stdout);
	  delete _fdOut;
	  _fdOut=NULL;
	}
      if (_tree!=NULL)
	{
	  printf("On detruit le ttree \n");fflush(stdout);
	  //delete _tree;
	  _tree=NULL;
	}
      std::stringstream filename("");    
      char dateStr [64];
            
      time_t tm= time(NULL);
      strftime(dateStr,20,"SMM_%d%m%y_%H%M%S",localtime(&tm));
      filename<<_directory<<"/"<<dateStr<<"_"<<_gEvent.run<<".root";

      _fdOut = new TFile(filename.str().c_str(),"recreate");
      _tree= new TTree("evt","a Tree with SDHCAL frame storage");
      _tree->SetAutoSave(0);
      _tree->Branch("run",&_gEvent.run,"run/I");
      _tree->Branch("event",&_gEvent.event,"event/I");
      _tree->Branch("gtc",&_gEvent.gtc,"gtc/I");
      _tree->Branch("abcid",&_gEvent.abcid,"abcid/L");
      _tree->Branch("nframe",&_gEvent.nframe,"nframe/I");
      _tree->Branch("frame",_gEvent.frame,"frame[nframe]/L");
      _tree->Branch("info",_gEvent.info,"info[nframe]/L");
      std::cout<<" On sort de createTree \n"<<std::flush;
	    
    }
    void closeTree()
    {
      printf("bin2writer::closeTree \n");
      if (_fdOut!=NULL)
	{
	  printf("bin2writer Tree header is writen \n");
	   if (_tree)
	     _tree->Write();
	   printf("bin2writer File is closed \n");
	   _fdOut->Close();
	   //delete _fdOut;
	  _fdOut=NULL;
	}
      if (_tree!=NULL)
	{
	  //delete _tree;
	  _tree=NULL;
	}
    }
  private:
    std::string _directory;
    uint32_t _run,_event,_totalSize;
    TFile* _fdOut;
    TTree* _tree;
    bin24Event_t _gEvent;
    bool _started;
    uint8_t temp[0x100000];

  };
};
#undef USEMAIN
#ifdef USEMAIN
int  main() {
  branalysis::bin24writer b;
  b.start(736286);
  b.readBinaryFile("/data/srv02/RAID6/Dome0916/SMM_070717_162812_736286.dat");
  b.stop();

}
#endif

extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  zdaq::zmprocessor* loadProcessor(void)
    {
      return (new branalysis::bin24writer);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(zdaq::zmprocessor* obj)
    {
      delete obj;
    }
}
