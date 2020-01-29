#include "PMRInterface.hh"
#include <unistd.h>
#include <stdint.h>
lydaq::PMRInterface::PMRInterface(pmr::FtdiDeviceInfo* ftd) : _rd(NULL),_state("CREATED"),_dsData(NULL),_detid(100)
{
  // Creation of data structure
  memcpy(&_ftd,ftd,sizeof(pmr::FtdiDeviceInfo));
  memset(_status,0,sizeof(pmr::DIFStatus));
  _readoutStarted=false;
  _readoutCompleted=true;
}
void lydaq::PMRInterface::setTransport(zdaq::zmPusher* p)
{
  _dsData=p;
  printf("DSDATA is %x\n",_dsData);
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
void lydaq::PMRInterface::initialise(zdaq::zmPusher* p)
{
  uint32_t difid=_ftd.id;
  //  create services
  if (p!=NULL) this->setTransport(p);


  try
    {

      _rd = new PmrDriver(_ftd.name,_ftd.productid);
      
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
  _rd->setAcquisitionMode();
  this->publishState("STARTED");
  LOG4CXX_INFO(_logLdaq,"PMR "<<_status->id<<" is started");
  _status->bytes=0;
  _running=true;
      
  
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


  unsigned char cbuf[48*128*20+8];
  _readoutCompleted=false;
  while (_readoutStarted)
    {
      if (!_running) {usleep((uint32_t) 100000);continue;}
      usleep((uint32_t) 100);
		
		
	  //printf("Trying to read \n");fflush(stdout);
      uint32_t nread=_rd->readOneEvent(cbuf);
      //printf(" Je lis %d => %d \n",_status->id,nread);
      if (nread==0) continue;
      //printf(" Je lis %d bytes => %d %x\n",_status->id,nread,_dsData);fflush(stdout);
      //this->publishData(nread);
      
      _status->gtc=PmrGTC(cbuf);
      _status->bcid=PmrABCID(cbuf);
      _status->bytes+=nread;
      if (_dsData==NULL) continue;;
      memcpy((unsigned char*) _dsData->payload(),cbuf,nread);
      //printf(" Je envoie %d => %d  avec %x \n",_status->id,nread,_dsData);fflush(stdout);
      _dsData->publish(_status->bcid,_status->gtc,nread);

		
    }
  _readoutCompleted=true;
  LOG4CXX_INFO(_logLdaq,"Thread of dif "<<_status->id<<" is stopped"<<_readoutStarted);
  _status->status=0XFFFF;
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
  _rd->setAcquisitionMode(false);
  this->publishState("STOPPED");
  
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
  
  LOG4CXX_INFO(_logLdaq, "PMR   id ("<<_status->id << ") ="<<tdata);
  return;
}


