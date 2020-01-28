
#include "PMRManager.hh"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
//#include "ftdi.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <string.h>

//using namespace Ftdi;
using namespace lydaq;
using namespace zdaq;







void lydaq::PMRManager::prepareDevices()
{
  for ( std::map<uint32_t,pmr::FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    if (it->second!=NULL) delete it->second;
  theFtdiDeviceInfoMap_.clear();
  for ( std::map<uint32_t,PMRInterface*>::iterator it=_PMRInterfaceMap.begin();it!=_PMRInterfaceMap.end();it++)
    if (it->second!=NULL) delete it->second;
  _PMRInterfaceMap.clear();
  system("/bin/rm /var/log/pi/ftdi_devices");
  system("/opt/dhcal/bin/ListDevices.py");
  std::string line;
  std::ifstream myfile ("/var/log/pi/ftdi_devices");
  std::stringstream diflist;



  if (myfile.is_open())
    {
      while ( myfile.good() )
	{
	  getline (myfile,line);
	  pmr::FtdiDeviceInfo* difi=new pmr::FtdiDeviceInfo();
	  memset(difi,0,sizeof(pmr::FtdiDeviceInfo));
	  sscanf(line.c_str(),"%x %x %s",&difi->vendorid,&difi->productid,difi->name);
	  if (strncmp(difi->name,"FT101",5)==0)
	    {
	      sscanf(difi->name,"FT101%d",&difi->id); 
	      difi->type=0;
	      std::pair<uint32_t,pmr::FtdiDeviceInfo*> p(difi->id,difi);
	      theFtdiDeviceInfoMap_.insert(p);
	    }
	  if (strncmp(difi->name,"DCCCCC",6)==0)
	    {sscanf(difi->name,"DCCCCC%d",&difi->id);difi->type=0x10;}


	}
      myfile.close();
    }
  else 
    {
      //std::cout << "Unable to open file"<<std::endl; 
      LOG4CXX_FATAL(_logDIF,__PRETTY_FUNCTION__<<" Unable to open /var/log/pi/ftdi_devices");
    }

  for (auto it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"Device found and register "<<it->first<<" with info "<<it->second->vendorid<<" "<<it->second->productid<<" "<<it->second->name<<" "<<it->second->type);
}


void lydaq::PMRManager::scan(zdaq::fsmmessage* m) 
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  // Fill Ftdi Map
  this->prepareDevices();
  std::map<uint32_t,pmr::FtdiDeviceInfo*>& fm=this->getFtdiMap();
  std::map<uint32_t,PMRInterface*> dm=this->getPMRMap();
  //LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" CMD: SCANDEVICE clear Maps");
  for (  auto it=dm.begin();it!=dm.end();it++)
    { if (it->second!=NULL) delete it->second;}
  dm.clear();
  // _ndif=0;
  std::vector<uint32_t> vids;
  Json::Value array;
  for ( auto it=fm.begin();it!=fm.end();it++)
    {

      PMRInterface* d= new PMRInterface(it->second);
      this->getPMRMap().insert(std::make_pair(it->first,d));
      LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" CMD: SCANDEVICE created PMRInterface @ "<<std::hex<<d<<std::dec);
      Json::Value jd;
      jd["detid"]=d->detectorId();
      jd["sourceid"]=it->first;
      vids.push_back( (d->detectorId()<<16|it->first));
      array.append(jd);
    }

   
  m->setAnswer(array);
}


void lydaq::PMRManager::initialise(zdaq::fsmmessage* m)
{
  
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  _vDif.clear();
  Json::Value jPMR=this->parameters()["dif"];
  
  // Download the configuration
  if (_hca==NULL)
    {
      std::cout<< "Create config acccess"<<std::endl;
      _hca=new lydaq::HR2ConfigAccess();
      _hca->clear();
    }
  std::cout<< " jPMR "<<jPMR<<std::endl;
  if (jPMR.isMember("json"))
    {
      Json::Value jPMRjson=jPMR["json"];
      if (jPMRjson.isMember("file"))
	{
	  _hca->parseJsonFile(jPMRjson["file"].asString());
	}
      else
	if (jPMRjson.isMember("url"))
	  {
	    _hca->parseJsonUrl(jPMRjson["url"].asString());
	  }
    }
  if (jPMR.isMember("db"))
    {
      Json::Value jPMRdb=jPMR["db"];
      LOG4CXX_ERROR(_logDIF,__PRETTY_FUNCTION__<<"Parsing:"<<jPMRdb["state"].asString()<<jPMRdb["mode"].asString());

              
      if (jPMRdb["mode"].asString().compare("mongo")!=0)	
	_hca->parseDb(jPMRdb["state"].asString(),jPMRdb["mode"].asString());
      else
	_hca->parseMongoDb(jPMRdb["state"].asString(),jPMRdb["version"].asUInt());

      LOG4CXX_ERROR(_logDIF,__PRETTY_FUNCTION__<<"End of parseDB "<<_hca->asicMap().size());
    }
  if (_hca->asicMap().size()==0)
    {
      LOG4CXX_ERROR(_logDIF,__PRETTY_FUNCTION__<<" No ASIC found in the configuration ");
      return;
    }
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"ASIC found in the configuration "<<_hca->asicMap().size() );
  // Initialise the network
  std::map<uint32_t,PMRInterface*> dm=this->getPMRMap();
  std::vector<uint32_t> vint;
   
  vint.clear();
  for (auto x:_hca->asicMap())
    {
      // only MSB is used
      uint32_t eip= ((x.first)>>56)&0XFF;
      std::map<uint32_t,PMRInterface*>::iterator idif=dm.find(eip);
      if (idif==dm.end()) continue;
      if ( std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;

      LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" New PMR found in db "<<std::hex<<eip<<std::dec);
      vint.push_back(eip);
      
      _vDif.push_back(idif->second);
      LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Registration done for "<<eip);
    }
  //std::string network=
  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);

  if (m->content().isMember("publish"))
    {
      this->parameters()["publish"]=m->content()["publish"];
    }
  if (!this->parameters().isMember("publish"))
    {
      
      LOG4CXX_ERROR(_logDIF,__PRETTY_FUNCTION__<<" No publish tag found ");
      return;
    }
  for (auto x:_vDif)
    {
      LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Creating pusher to "<<this->parameters()["publish"].asString());
      zdaq::zmPusher* push=new zdaq::zmPusher(_context,x->detectorId(),x->status()->id);
      push->connect(this->parameters()["publish"].asString());
      x->initialise(push);

    }
  // Listen All Gric sockets






  
}







void lydaq::PMRManager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
{

  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Changin thresholds: "<<b0<<","<<b1<<","<<b2);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      if (idif!=0)
	{
	  uint32_t ip=(((it->first)>>32&0XFFFFFFFF)>>16)&0xFFFF;
	  printf("%x %x %x \n",(it->first>>32),ip,idif);
	  if (idif!=ip) continue;
	}
      it->second.setB0(b0);
      it->second.setB1(b1);
      it->second.setB2(b2);
      //it->second.setHEADER(0x56);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::sleep(1);

}
void lydaq::PMRManager::setGain(uint16_t gain)
{

  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Changing Gain: "<<gain);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	it->second.setPAGAIN(i,gain);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::sleep(1);

}

void lydaq::PMRManager::setMask(uint32_t level,uint64_t mask)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Changing Mask: "<<level<<" "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASK(level,mask);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}
void lydaq::PMRManager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Changing Mask: "<<level<<" "<<std::hex<<channel<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASKChannel(level,channel,val==1);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}

void lydaq::PMRManager::c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t b0=atol(request.get("B0","250").c_str());
  uint32_t b1=atol(request.get("B1","250").c_str());
  uint32_t b2=atol(request.get("B2","250").c_str());
  uint32_t idif=atol(request.get("PMR","0").c_str());
  
  this->setThresholds(b0,b1,b2,idif);
  response["THRESHOLD0"]=b0;
  response["THRESHOLD1"]=b1;
  response["THRESHOLD2"]=b2;
}
void lydaq::PMRManager::c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t gain=atol(request.get("gain","128").c_str());
  this->setGain(gain);
  response["GAIN"]=gain;

}

void lydaq::PMRManager::c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"SetMask called ");
  response["STATUS"]="DONE";

  
  //uint32_t nc=atol(request.get("value","4294967295").c_str());
  uint64_t mask;
  sscanf(request.get("mask","0XFFFFFFFFFFFFFFFF").c_str(),"%lx",&mask);
  uint32_t level=atol(request.get("level","0").c_str());
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"SetMask called "<<std::hex<<mask<<std::dec<<" level "<<level);
  this->setMask(level,mask);
  response["MASK"]=(Json::UInt64) mask;
  response["LEVEL"]=level;
}



void lydaq::PMRManager::c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"SetMask called ");
  response["STATUS"]="DONE";

  
  //uint32_t nc=atol(request.get("value","4294967295").c_str());
  uint32_t level=atol(request.get("level","0").c_str());
  uint32_t channel=atol(request.get("channel","0").c_str());
  bool on=atol(request.get("value","1").c_str())==1;
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"SetMaskChannel called "<<channel<<std::dec<<" level "<<level);
  this->setChannelMask(level,channel,on);
  response["CHANNEL"]=channel;
  response["LEVEL"]=level;
  response["ON"]=on;
}
void lydaq::PMRManager::c_ctrlreg(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"CTRLREG called "<<request.get("value","0").c_str());

  uint32_t  ctrlreg=0;
  sscanf(request.get("value","0").c_str(),"%u",&ctrlreg);
   
  if (ctrlreg!=0)
    this->parameters()["ctrlreg"]=ctrlreg;

  fprintf(stderr,"CTRLREG %s %lx %d\n",request.get("value","0").c_str(),ctrlreg,this->parameters()["ctrlreg"].asUInt());
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"CTRLREG called "<<std::hex<<ctrlreg<<std::dec);
  response["STATUS"]="DONE";
  response["CTRLREG"]= ctrlreg;
  
}
void lydaq::PMRManager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<"downloadDB called ");
  response["STATUS"]="DONE";


  
  std::string dbstate=request.get("state","NONE");
  uint32_t version=atol(request.get("version","0").c_str());
  Json::Value jTDC=this->parameters()["dif"];
  if (jTDC.isMember("db"))
    {
      Json::Value jTDCdb=jTDC["db"];
      _hca->clear();

      if (jTDCdb["mode"].asString().compare("mongo")!=0)
	_hca->parseDb(dbstate,jTDCdb["mode"].asString());
      else
	_hca->parseMongoDb(dbstate,version);

	 
    }
  response["DBSTATE"]=dbstate;
}


void lydaq::PMRManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  
  int32_t rc=1;
  std::map<uint32_t,PMRInterface*> dm=this->getPMRMap();
  Json::Value array_slc;
 
  for ( std::map<uint32_t,PMRInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      
      Json::Value ds;
      ds["detid"]=it->second->detectorId();
      ds["state"]=it->second->state();
      ds["id"]=it->second->status()->id;
      ds["status"]=it->second->status()->status;
      ds["slc"]=it->second->status()->slc;
      ds["gtc"]=it->second->status()->gtc;
      ds["bcid"]=(Json::Value::UInt64) it->second->status()->bcid;
      ds["bytes"]=(Json::Value::UInt64)it->second->status()->bytes;
      ds["host"]=it->second->status()->host;
      array_slc.append(ds);



    }
  response["STATUS"]="DONE";
  response["PMRLIST"]=array_slc;


  return;
  
}

Json::Value lydaq::PMRManager::configureHR2()
{
  /// A reecrire
  uint32_t ctrlreg=this->parameters()["ctrlreg"].asUInt();
  printf("CTRLREG %lx \n",ctrlreg);
  int32_t rc=1;
  std::map<uint32_t,PMRInterface*> dm=this->getPMRMap();
  Json::Value array_slc=Json::Value::null;

  for ( std::map<uint32_t,PMRInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      std::stringstream ips;
      // Dummy IP address for PMRs
      ips<<"0.0.0."<<it->first;
      _hca->prepareSlowControl(ips.str(),true);
     
      it->second->configure(_hca->slcBuffer(),_hca->slcBytes());
      Json::Value ds;
      ds["id"]=it->first;
      ds["slc"]=it->second->status()->slc;
      array_slc.append(ds);
      

    }
  return array_slc;
}

void lydaq::PMRManager::configure(zdaq::fsmmessage* m)
{

  LOG4CXX_DEBUG(_logDIF,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  if (m->content().isMember("ctrlreg"))
    {
      this->parameters()["ctrlreg"]=m->content()["ctrlreg"].asUInt();
    }
  uint32_t ctrlreg=this->parameters()["ctrlreg"].asUInt();
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Configuring with  ctr "<<ctrlreg<<" cont "<<m->content());
  int32_t rc=1;

  Json::Value array_slc=this->configureHR2();

  m->setAnswer(array_slc);
  return;
    
}

void lydaq::PMRManager::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Starting ");
 
  int32_t rc=1;
  std::map<uint32_t,PMRInterface*> dm=this->getPMRMap();
  for ( std::map<uint32_t,PMRInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      this->startReadoutThread(it->second);
      it->second->start();
    }

  return;
  
}
void lydaq::PMRManager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Stopping ");
 
  int32_t rc=1;
  std::map<uint32_t,PMRInterface*> dm=this->getPMRMap();
 
  for ( std::map<uint32_t,PMRInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Stopping thread of PMR"<<it->first);
      it->second->stop();
    }
  
  return;
  
}
void lydaq::PMRManager::destroy(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Destroying ");
  
  int32_t rc=1;
  std::map<uint32_t,PMRInterface*> dm=this->getPMRMap();

  bool running=false;
  for ( std::map<uint32_t,PMRInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      running=running ||it->second->readoutStarted();
    }
  if (running)
    {
      for ( std::map<uint32_t,PMRInterface*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->setReadoutStarted(false);
	}
      
      this->joinThreads();
    }
  for ( std::map<uint32_t,PMRInterface*>::iterator it=dm.begin();it!=dm.end();it++)
    {
      it->second->destroy();
    }


  return;
  
}






lydaq::PMRManager::PMRManager(std::string name)  : zdaq::baseApplication(name)
{

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();
  // Zmq transport
  _context = new zmq::context_t (1);
  // Register state
  _fsm->addState("SCANNED");
  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  _fsm->addState("STOPPED");
  _fsm->addTransition("SCAN","CREATED","SCANNED",boost::bind(&lydaq::PMRManager::scan, this,_1));
  _fsm->addTransition("INITIALISE","SCANNED","INITIALISED",boost::bind(&lydaq::PMRManager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::PMRManager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::PMRManager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","STOPPED","CONFIGURED",boost::bind(&lydaq::PMRManager::configure, this,_1));
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::PMRManager::start, this,_1));
  _fsm->addTransition("START","STOPPED","RUNNING",boost::bind(&lydaq::PMRManager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","STOPPED",boost::bind(&lydaq::PMRManager::stop, this,_1));
  _fsm->addTransition("DESTROY","STOPPED","CREATED",boost::bind(&lydaq::PMRManager::destroy, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::PMRManager::destroy, this,_1));


  _fsm->addCommand("STATUS",boost::bind(&lydaq::PMRManager::c_status,this,_1,_2));
  _fsm->addCommand("SETTHRESHOLDS",boost::bind(&lydaq::PMRManager::c_setthresholds,this,_1,_2));
  _fsm->addCommand("SETPAGAIN",boost::bind(&lydaq::PMRManager::c_setpagain,this,_1,_2));
  _fsm->addCommand("SETMASK",boost::bind(&lydaq::PMRManager::c_setmask,this,_1,_2));
  _fsm->addCommand("SETCHANNELMASK",boost::bind(&lydaq::PMRManager::c_setchannelmask,this,_1,_2));
  _fsm->addCommand("DOWNLOADDB",boost::bind(&lydaq::PMRManager::c_downloadDB,this,_1,_2));
  _fsm->addCommand("CTRLREG",boost::bind(&lydaq::PMRManager::c_ctrlreg,this,_1,_2));

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logDIF,__PRETTY_FUNCTION__<<" Service "<<name<<" started on port "<<atoi(wp));
      _fsm->start(atoi(wp));
    }
  _hca=NULL;
  // Initialise delays for 
}




void lydaq::PMRManager::startReadoutThread(PMRInterface* d)
{
  if (d->readoutStarted()) return;
  d->setReadoutStarted(true);	

  g_d.create_thread(boost::bind(&PMRInterface::readout,d));
  
}



