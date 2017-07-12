#include "TFile.h"
#include "TTree.h"
#include "TBrowser.h"
#include "TH2.h"
#include "TRandom.h"
#include "TCanvas.h"
#include "TMath.h"
#include "TROOT.h"
#include <stdint.h>
#include "SdhcalDifAccess.hh"
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
  Int_t    nstrip;
  Long_t   strip[MAXSTRIP];
} bin23Event_t;
using namespace zdaq;
using namespace sdhcal;

namespace branalysis
{
  class Frame
  {
  public:
    Frame(){_frame=0;}
    Frame(uint8_t dif,uint8_t asic,uint8_t channel,uint8_t threshold,uint32_t bc){ this->set(dif,asic,channel,threshold,bc);}
    inline uint32_t dif(){return (_frame&0xFF);}
    inline uint32_t asic( ){return ((_frame>>8)&0x3F);}
    inline uint32_t channel( ){return ((_frame>>14)&0x3F);}
    inline uint32_t threshold(){return ((_frame>>20)&0x3);}
    inline void set(uint8_t dif,uint8_t asic,uint8_t channel,uint8_t threshold,uint32_t bc)
    {
      uint64_t d=dif,a=asic,c=channel,t=threshold,b=bc;
      _frame=((b&0xFFFFFFFF)<<32)|((t&0x3)<<20)|((c&0x3f)<<14)|((a&0x3F)<<8)|
	(d&0xFF);}
    inline uint64_t value(){return _frame;}
    inline void set( uint64_t f){_frame=f;}
  private:
    uint64_t _frame;
  };



  class bin23writer : public zdaq::zmprocessor
  {
  public:
    bin23writer(std::string dire="/tmp") : _directory(dire),_run(0),_started
					   (false),_fdOut(NULL),_totalSize(0),_event(0) {}

    
    virtual void start(uint32_t run)
    {_gEvent.run=run; this->createTree();_gEvent.event=0;}
    virtual void stop()
    {
      this->closeTree();
    }
    virtual  void processEvent(uint32_t key,std::vector<zdaq::buffer*> dss)
    {_gEvent.nframe=0;
      _gEvent.event++;
      
      for (auto x:dss)
	{
	  x->uncompress();
	  _gEvent.gtc=x->eventId();
	  _gEvent.abcid=x->bxId();
	  DIFPtr d((unsigned char*) x->payload(),x->payloadSize());
	  printf("--------------------------------------------------> %d %d \n",x->size(),x->payloadSize());

	  // uint8_t* cb=(uint8_t*)x->payload();
	  // for (uint32_t i=0;i<40;i++)
	  //   printf("%02x",cb[i]);
	  // printf("\n");
	  //printf("%d %d %ld :DIF %d is found \n",_gEvent.run,_gEvent.gtc,_gEvent.abcid,d.getID());
	  
	  for (uint32_t ifra=0;ifra<d.getNumberOfFrames();ifra++)
	    {
	      uint32_t bc=d.getFrameTimeToTrigger(ifra);
	      for (uint32_t j=0;j<64;j++)
		{
		  if (!(d.getFrameLevel(ifra,j,0) || d.getFrameLevel(ifra,j,1))) continue;
		  uint8_t level=0;
		  if (d.getFrameLevel(ifra,j,0) && d.getFrameLevel(ifra,j,1)) 
		    level=2;
		  else
		    if (d.getFrameLevel(ifra,j,0)) 
		      level=1;
		  branalysis::Frame f(d.getID(),d.getFrameAsicHeader(ifra),j,level,bc);
		  _gEvent.frame[_gEvent.nframe]=f.value();
		  _gEvent.nframe++;
		}
	    }
	  
	}
      // Now fill the TTree
      _tree->Fill();
      
    }
 
    virtual  void processRunHeader(std::vector<uint32_t> header)
    {return;}
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
		  if (b->detectorId()!=100) continue;
		  if (idif==0)
		    {
		      _gEvent.abcid=b->bxId();
		      _gEvent.gtc=b->eventId();
		      /*
		      if (_t0>1E50)
			_t0=b->bxId()*2E-7;
		      double ct=b.bxId()*2E-7-_t0;
		      if ((ct-_t)>5.)
			{
			  _tspill=ct;
			  std::cout<<" New Spill====>"<<_tspill<<std::endl;
			}
		      _t=ct;
		      */
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
    bin23Event_t* event(){return &_gEvent;}
    void createTree()
    {
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
      _tree->Branch("nstrip",&_gEvent.nstrip,"nstrip/I");
      _tree->Branch("strip",_gEvent.strip,"strip[nstrip]/L");
    }
    void closeTree()
    {
      if (_fdOut!=NULL)
	{
	   if (_tree)
	     _tree->Write();
	   //_fdOut->Close();
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
    bin23Event_t _gEvent;
    bool _started;
  };
};
#undef USEMAIN
#ifdef USEMAIN
int  main() {
  branalysis::bin23writer b;
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
      return (new branalysis::bin23writer);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(zdaq::zmprocessor* obj)
    {
      delete obj;
    }
}
