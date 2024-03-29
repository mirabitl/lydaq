#include "PMRInterface.hh"
#include <unistd.h>
#include <stdint.h>
using namespace Pmr;lydaq::PMRInterface::PMRInterface(Pmr::FtdiDeviceInfo* ftd) : _rd(NULL),_state("CREATED"),_dsData(NULL),_detid(150),_external(true)
{
  // Creation of data structure

  memcpy(&_ftd,ftd,sizeof(Pmr::FtdiDeviceInfo));

  _status = new Pmr::DIFStatus();
  memset(_status,0,sizeof(Pmr::DIFStatus));

  sscanf(ftd->name,"FT101%d",&(_status->id));
  _readoutStarted=false;
  _readoutCompleted=true;

}
void lydaq::PMRInterface::setExternalTrigger(bool t) {_external=t;}
void lydaq::PMRInterface::setTransport(zdaq::zmSender* p)
{
  _dsData=p;
  printf("DSDATA is %p\n", _dsData);
}
lydaq::PMRInterface::~PMRInterface()
{


  if (_dsData!=NULL)
    {
      delete _dsData;
      _dsData=NULL;
    }
  if (_rd!=NULL)
    {
      delete _rd;
      _rd=NULL;
    }
}
void lydaq::PMRInterface::initialise(zdaq::zmSender* p)
{
  uint32_t difid=_ftd.id;
  //  create services
  if (p!=NULL) this->setTransport(p);


  try
    {

      _rd = new PmrDriver(_ftd.name,_ftd.productid);
      _status->id= _rd->difId();
    }
  catch (...)
    {
      if (_rd!=NULL)
	{
	  delete _rd;
	  _rd=NULL;
	}
      LOG4CXX_FATAL(_logLdaq,"cannot create PmrDriver for  "<<difid);
      this->publishState("INIT_RD_FAILED");
      return;
    }
 
  this->publishState("INITIALISED");
}
 

void lydaq::PMRInterface::start()
{

  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "PMR   id ("<<_status->id << ") is not initialised");
      this->publishState("START_FAILED");
      return;
    }
  _rd->setAcquisitionMode(true,false,_external);
  // _rd->setAcquisitionMode(true,true,_external);
  this->publishState("STARTED");
  LOG4CXX_INFO(_logLdaq,"PMR "<<_status->id<<" is started");
  _status->bytes=0;
  _running=true;
      
  
}
void lydaq::PMRInterface::stop()
{
  _running=false;
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "PMR   id ("<<_status->id << ") is not initialised");
      this->publishState("STOP_FAILED");
      return;
    }
  _rd->setAcquisitionMode(false,false,_external);
  this->publishState("STOPPED");
  
}

void lydaq::PMRInterface::readout()
{
  LOG4CXX_INFO(_logLdaq,"Thread of dif "<<_status->id<<" is started");
  if (_rd==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, "PMR   id ("<<_status->id << ") is not initialised");
      this->publishState("READOUT_FAILED");
      _readoutStarted=false;
      return;
    }

  _rd->resetFSM();
  unsigned char cbuf[48*128*20+8];
  _readoutCompleted=false;
  while (_readoutStarted)
    {
      //printf("On rentre dans la boucle \n");fflush(stdout);
      if (!_running) {usleep((uint32_t) 100000);continue;}
      usleep((uint32_t) 100);
		
		
      //printf("Trying to read \n");fflush(stdout);
      uint32_t nread=_rd->readOneEvent(cbuf);
      //printf(" Je lis %d => %d \n",_status->id,nread);
      if (nread==0) continue;
      _rd->resetFSM();
      //printf(" Je lis %d bytes => %d %x\n",_status->id,nread,_dsData);fflush(stdout);
      //this->publishData(nread);
      
      _status->gtc=PmrGTC(cbuf);
      //unsigned long long Shift=16777216ULL;//to shift the value from the 24 first bits
      //unsigned long long LBC= ( (cbuf[PMR_ABCID_SHIFT]<<16) | (cbuf[PMR_ABCID_SHIFT+1]<<8) | (cbuf[PMR_ABCID_SHIFT+2]))*Shift+( (cbuf[PMR_ABCID_SHIFT+3]<<16) | (cbuf[PMR_ABCID_SHIFT+4]<<8) | (cbuf[PMR_ABCID_SHIFT+5]));
      uint64_t LBC=((uint64_t)cbuf[PMR_ABCID_SHIFT]<<40) |
	((uint64_t) cbuf[PMR_ABCID_SHIFT+1]<<32) |
	((uint64_t) cbuf[PMR_ABCID_SHIFT+2]<<24)|
        ((uint64_t) cbuf[PMR_ABCID_SHIFT+3]<<16 )|
	((uint64_t) cbuf[PMR_ABCID_SHIFT+4]<<8)  |
	((uint64_t) cbuf[PMR_ABCID_SHIFT+5]);

      _status->bcid=LBC;//PmrABCID(cbuf);
      fprintf(stderr,"ABCID %lx \n",LBC);
      _status->bytes+=nread;
      if (_dsData==NULL) continue;;
      memcpy((unsigned char*) _dsData->payload(),cbuf,nread);
      //printf(" Je envoie %d => %d  avec %x \n",_status->id,nread,_dsData);fflush(stdout);
      _dsData->publish(_status->bcid,_status->gtc,nread);
      //if (_status->gtc%50 ==0)
      fprintf(stderr,"ICI Je publie ABCID  %lx \n",_status->bcid);
      fprintf(stderr,"ICI Je publie  GTC %d \n",_status->gtc);
      fprintf(stderr,"ICI Je publie  NREAD %d \n",nread);

      //_rd->resetFSM();
    }
  _readoutCompleted=true;
  LOG4CXX_INFO(_logLdaq,"Thread of dif "<<_status->id<<" is stopped"<<_readoutStarted);
  _status->status=0XFFFF;
}
void lydaq::PMRInterface::destroy()
{
  if (_readoutStarted)
    {
      _readoutStarted=false;
      uint32_t ntry=0;
      while (!_readoutCompleted && ntry<100)
	{usleep((uint32_t) 200000);ntry++;}
    }
  if (_rd!=NULL)
    {
  
      delete _rd;
      _rd=NULL;
      this->publishState("CREATED");
    }
  if (_dsData!=NULL)
    {
      LOG4CXX_INFO(_logLdaq," Deleting dim services ");
      delete _dsData;
      _dsData=NULL;
    }

}

void lydaq::PMRInterface::configure(unsigned char* b,uint32_t nb)
{
  _rd->loadSLC(b,nb);
  uint32_t tdata;
  _rd->registerRead(PMR_SLC_STATUS_REG,&tdata);
  _status->slc=tdata;
  this->publishState("CONFIGURED");

  LOG4CXX_INFO(_logLdaq, "PMR   id ("<<_status->id << ") ="<<tdata);
  return;
}


