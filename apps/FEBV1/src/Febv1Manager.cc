#include "Febv1Manager.hh"
using namespace mpi;
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
#include <arpa/inet.h>
#include <boost/format.hpp>




#include "fsmwebCaller.hh"

using namespace zdaq;
using namespace lydaq;


lydaq::Febv1Manager::Febv1Manager(std::string name) : zdaq::baseApplication(name),_context(NULL),_hca(NULL),_mpi(NULL)
{
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::Febv1Manager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::Febv1Manager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::Febv1Manager::configure, this,_1));
  
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::Febv1Manager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&lydaq::Febv1Manager::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::Febv1Manager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&lydaq::Febv1Manager::destroy, this,_1));
  
  
  
  //_fsm->addCommand("JOBLOG",boost::bind(&lydaq::Febv1Manager::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS",boost::bind(&lydaq::Febv1Manager::c_status,this,_1,_2));
  _fsm->addCommand("RESET",boost::bind(&lydaq::Febv1Manager::c_reset,this,_1,_2));
  
  _fsm->addCommand("SETTHRESHOLDS",boost::bind(&lydaq::Febv1Manager::c_setthresholds,this,_1,_2));
  _fsm->addCommand("SETPAGAIN",boost::bind(&lydaq::Febv1Manager::c_setpagain,this,_1,_2));
  _fsm->addCommand("SETMASK",boost::bind(&lydaq::Febv1Manager::c_setmask,this,_1,_2));
  _fsm->addCommand("SETCHANNELMASK",boost::bind(&lydaq::Febv1Manager::c_setchannelmask,this,_1,_2));
  _fsm->addCommand("DOWNLOADDB",boost::bind(&lydaq::Febv1Manager::c_downloadDB,this,_1,_2));
  _fsm->addCommand("READREG",boost::bind(&lydaq::Febv1Manager::c_readreg,this,_1,_2));
  _fsm->addCommand("WRITEREG",boost::bind(&lydaq::Febv1Manager::c_writereg,this,_1,_2));

  _fsm->addCommand("READBME",boost::bind(&lydaq::Febv1Manager::c_readbme,this,_1,_2));
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Service "<<name<<" is starting on "<<atoi(wp));

      
      _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink


}
void lydaq::Febv1Manager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Status CMD called ");
  response["STATUS"]="DONE";

  Json::Value jl;
  for (auto x:_mpi->boards())
    {

      Json::Value jt;
      jt["detid"]=x.second->data()->detectorId();
      std::stringstream sid;
      sid<<std::hex<<x.second->data()->difId()<<std::dec;
      jt["sourceid"]=sid.str();
      jt["SLC"]=x.second->reg()->slcStatus();
      jt["gtc"]=x.second->data()->gtc();
      jt["abcid"]=(Json::Value::UInt64)x.second->data()->abcid();
      jt["event"]=x.second->data()->event();
      jt["triggers"]=x.second->data()->triggers();
      jl.append(jt);
    }
  response["C3ISTATUS"]=jl;
}


void lydaq::Febv1Manager::c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"RESET CMD called ");
  for (auto x:_mpi->boards())
    {
      x.second->reg()->writeRegister(febv1::Message::Register::ACQ_RST,1);
      ::usleep(1000);
      x.second->reg()->writeRegister(febv1::Message::Register::ACQ_RST,0);
    }
  response["STATUS"]="DONE"; 
}

void lydaq::Febv1Manager::c_readbme(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"RESET CMD called ");
  Json::Value r;
  for (auto x:_mpi->boards())
    {
      r["CAL1"]=x.second->reg()->readRegister(febv1::Message::Register::BME_CAL1);
      r["CAL2"]=x.second->reg()->readRegister(febv1::Message::Register::BME_CAL2);
      r["CAL3"]=x.second->reg()->readRegister(febv1::Message::Register::BME_CAL3);
      r["CAL4"]=x.second->reg()->readRegister(febv1::Message::Register::BME_CAL4);
      r["CAL5"]=x.second->reg()->readRegister(febv1::Message::Register::BME_CAL5);
      r["CAL6"]=x.second->reg()->readRegister(febv1::Message::Register::BME_CAL6);
      r["CAL7"]=x.second->reg()->readRegister(febv1::Message::Register::BME_CAL7);
      r["CAL8"]=x.second->reg()->readRegister(febv1::Message::Register::BME_CAL8);
      r["HUM"]=x.second->reg()->readRegister(febv1::Message::Register::BME_HUM);
      r["PRES"]=x.second->reg()->readRegister(febv1::Message::Register::BME_PRES);
      r["TEMP"]=x.second->reg()->readRegister(febv1::Message::Register::BME_TEMP);
    }
  response["STATUS"]=r; 
}


void lydaq::Febv1Manager::c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t b0=atol(request.get("B0","250").c_str());
  uint32_t b1=atol(request.get("B1","250").c_str());
  uint32_t b2=atol(request.get("B2","250").c_str());
  uint32_t idif=atol(request.get("DIF","0").c_str());
  
  this->setThresholds(b0,b1,b2,idif);
  response["THRESHOLD0"]=b0;
  response["THRESHOLD1"]=b1;
  response["THRESHOLD2"]=b2;
}
void lydaq::Febv1Manager::c_readreg(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Pulse called ");
  response["STATUS"]="DONE";

  
  uint32_t adr=atol(request.get("adr","0").c_str());

  Json::Value r;
  for (auto x:_mpi->boards())
    {    
      uint32_t value=x.second->reg()->readRegister(adr);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Read reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<value);
      r[x.second->ipAddress()]=value;
    }
  

  response["READREG"]=r;
}
void lydaq::Febv1Manager::c_writereg(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Pulse called ");
  response["STATUS"]="DONE";

  
  uint32_t adr=atol(request.get("adr","0").c_str());
  uint32_t val=atol(request.get("val","0").c_str());

  Json::Value r;
  for (auto x:_mpi->boards())
    {    
      x.second->reg()->writeRegister(adr,val);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Write reg "<<x.second->ipAddress()<<" Address "<<adr<<" Value "<<val);
      r[x.second->ipAddress()]=val;
    }
  

  response["WRITEREG"]=r;
}
void lydaq::Febv1Manager::c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t gain=atol(request.get("gain","128").c_str());
  this->setGain(gain);
  response["GAIN"]=gain;

}

void lydaq::Febv1Manager::c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called ");
  response["STATUS"]="DONE";

  
  //uint32_t nc=atol(request.get("value","4294967295").c_str());
  uint64_t mask;
  sscanf(request.get("mask","0XFFFFFFFFFFFFFFFF").c_str(),"%lx",&mask);
  uint32_t level=atol(request.get("level","0").c_str());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called "<<std::hex<<mask<<std::dec<<" level "<<level);
  this->setMask(level,mask);
  response["MASK"]=(Json::UInt64) mask;
  response["LEVEL"]=level;
}



void lydaq::Febv1Manager::c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called ");
  response["STATUS"]="DONE";

  
  //uint32_t nc=atol(request.get("value","4294967295").c_str());
  uint32_t level=atol(request.get("level","0").c_str());
  uint32_t channel=atol(request.get("channel","0").c_str());
  bool on=atol(request.get("value","1").c_str())==1;
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMaskChannel called "<<channel<<std::dec<<" level "<<level);
  this->setChannelMask(level,channel,on);
  response["CHANNEL"]=channel;
  response["LEVEL"]=level;
  response["ON"]=on;
}

void lydaq::Febv1Manager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"downloadDB called ");
  response["STATUS"]="DONE";


  
  std::string dbstate=request.get("state","NONE");
  uint32_t version=atol(request.get("version","0").c_str());
  Json::Value jTDC=this->parameters()["febv1"];
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

void lydaq::Febv1Manager::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"****** CMD: "<<m->command());
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  Json::Value jtype=this->parameters()["type"];
  _type=jtype.asInt();
  printf ("_type =%d\n",_type); 

  // Need a FEBV1 tag
  if (m->content().isMember("febv1"))
    {
      printf ("found febv1/n");
      this->parameters()["febv1"]=m->content()["febv1"];
    }
  if (!this->parameters().isMember("febv1"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No febv1 tag found ");
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new lydaq::febv1::Interface();
  _mpi->initialise();

   
  Json::Value jFEBV1=this->parameters()["febv1"];
  //_msh =new lydaq::MpiMessageHandler("/dev/shm");
  if (!jFEBV1.isMember("network"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No febv1:network tag found ");
      return;
    }
  // Scan the network
  std::map<uint32_t,std::string> diflist=mpi::MpiMessageHandler::scanNetwork(jFEBV1["network"].asString());
  // Download the configuration
  if (_hca==NULL)
    {
      std::cout<< "Create config acccess"<<std::endl;
      _hca=new lydaq::HR2ConfigAccess();
      _hca->clear();
    }
  std::cout<< " jFEBV1 "<<jFEBV1<<std::endl;
  if (jFEBV1.isMember("json"))
    {
      Json::Value jFEBV1json=jFEBV1["json"];
      if (jFEBV1json.isMember("file"))
	{
	  _hca->parseJsonFile(jFEBV1json["file"].asString());
	}
      else
	if (jFEBV1json.isMember("url"))
	  {
	    _hca->parseJsonUrl(jFEBV1json["url"].asString());
	  }
    }
  if (jFEBV1.isMember("db"))
    {
      Json::Value jFEBV1db=jFEBV1["db"];
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<"Parsing:"<<jFEBV1db["state"].asString()<<jFEBV1db["mode"].asString());

              
      if (jFEBV1db["mode"].asString().compare("mongo")!=0)	
	_hca->parseDb(jFEBV1db["state"].asString(),jFEBV1db["mode"].asString());
      else
	_hca->parseMongoDb(jFEBV1db["state"].asString(),jFEBV1db["version"].asUInt());
      
    }
  if (_hca->asicMap().size()==0)
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No ASIC found in the configuration ");
      return;
    }
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"ASIC found in the configuration "<<_hca->asicMap().size() );
  // Initialise the network
  std::vector<uint32_t> vint;
  vint.clear();
  for (auto x:_hca->asicMap())
    {
      uint32_t eip= ((x.first)>>32)&0XFFFFFFFF;
      std::map<uint32_t,std::string>::iterator idif=diflist.find(eip);
      if (idif==diflist.end()) continue;
      if ( std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;
      
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" New FEBV1 found in db "<<std::hex<<eip<<std::dec<<" IP address "<<idif->second);
      vint.push_back(eip);
      _mpi->addDevice(idif->second);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Registration done for "<<eip);
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
      
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No publish tag found ");
      return;
    }
  for (auto x:_mpi->boards())
    x.second->data()->autoRegister(_context,this->configuration(),"BUILDER","collectingPort");
  //x->connect(_context,this->parameters()["publish"].asString());

  // Listen All Febv1 sockets
  _mpi->listen();

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Init done  "); 
}

void lydaq::Febv1Manager::configurePR2()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" COnfigure the chips ");

  
  fprintf(stderr,"Loop on socket for Sending slow control \n");
  for (auto x:_mpi->boards())
    {
      _hca->prepareSlowControl(x.second->ipAddress());
      x.second->reg()->writeRam(_tca->slcAddr(), _tca->slcBuffer(), _tca->slcBytes());
    }
}
void lydaq::Febv1Manager::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

  // Now loop on slowcontrol socket


  this->configurePR2();

}

void lydaq::WiznetManager::set6bDac(uint8_t dac)
{

  //::sleep(1);

  // Modify ASIC SLC
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    for (int i = 0; i < 32; i++)
    {
      it->second.set6bDac(i, dac);
    }
  }
  // Now loop on slowcontrol socket and send packet
  this->configurePR2();
}
void lydaq::WiznetManager::cal6bDac(uint32_t mask, int32_t dacShift)
{
  LOG4CXX_INFO(_logFeb, "CAL6BDAC called " << mask << " Shift " << dacShift);
  //::usleep(50000);
  std::map<uint64_t, uint16_t *> ascopy;
  // Modify ASIC SLC
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {

    if (ascopy.find(it->first) == ascopy.end())
    {
      uint16_t *b = new uint16_t[32];
      std::pair<uint64_t, uint16_t *> p(it->first, b);
      ascopy.insert(p);
    }
    auto ic = ascopy.find(it->first);
    for (int i = 0; i < 32; i++)
    {
      ic->second[i] = it->second.get6bDac(i);
      if ((mask >> i) & 1)
      {
        uint32_t dac = it->second.get6bDac(i);
        int32_t ndac = dac + dacShift;
        if (ndac < 0)
          ndac = 0;
        if (ndac > 63)
          ndac = 63;
        std::cout << "channel " << i << " DAC " << dac << " shifted to " << ndac << std::endl;

        it->second.set6bDac(i, ndac);
      }
    }
  }
  // Now loop on slowcontrol socket and send packet
  this->configurePR2();

  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {

    auto ic = ascopy.find(it->first);
    for (int i = 0; i < 32; i++)

      it->second.set6bDac(i, ic->second[i]);
  }
  for (auto it = ascopy.begin(); it != ascopy.end(); it++)
    delete it->second;

 
}
void lydaq::WiznetManager::c_asics(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";
  Json::Value jlist;
  ;
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    Json::Value jasic;
    uint32_t iasic = it->first & 0xFF;
    jasic["num"] = iasic;
    uint32_t difid = ((it->first) >> 32 & 0xFFFFFFFF);
    jasic["dif"] = difid;
    it->second.toJson();
    jasic["slc"] = it->second.getJson();
    jlist.append(jasic);
  }
  response["asics"] = jlist;
}

void lydaq::WiznetManager::setMask(uint32_t mask, uint8_t asic)
{

  //::sleep(1);
  // Change all Asics VthTime
  uint32_t umask;
  uint32_t asica = asic;
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    uint32_t iasic = it->first & 0xFF;
    fprintf(stderr, "ASIC in map %d ASIC asked %d \n", iasic, asica);
    if ((iasic & asica) == 0)
    {
      fprintf(stderr, "Skipping asic %d by masking all channels\n", iasic);
      umask = 0;
    }
    else
      umask = mask;

    for (int i = 0; i < 32; i++)
    {
      if ((umask >> i) & 1)
      {
        it->second.setMaskDiscriTime(i, 0);
      }
      else
      {
        it->second.setMaskDiscriTime(i, 1);
      }
    }
    //std::cout << "ASIC " << (int)iasic << "==========================" << std::endl;
    //it->second.Print();
  }

  // Now loop on slowcontrol socket
  this->configurePR2();
}

void lydaq::WiznetManager::setVthTime(uint32_t vth)
{

  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Debut ");
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    int iasic = it->first & 0xFF;

    it->second.setVthTime(vth);
    //it->second.Print();
    // 1 seul ASIC break;
  }
  this->configurePR2();
 
  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Fin ");
}

void lydaq::WiznetManager::setSingleVthTime(uint32_t vth, uint32_t feb, uint32_t asic)
{
  // Encode IP
  std::stringstream ip;
  ip << "192.168.10." << feb;

  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Debut ");
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    //  Change VTH time only on specified ASIC
    uint64_t eid = (((uint64_t)lydaq::WiznetMessageHandler::convertIP(ip.str())) << 32) | asic;
    if (eid != it->first)
      continue;
    it->second.setVthTime(vth);
  }
  this->configurePR2();

  
  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Fin ");
}

void lydaq::WiznetManager::setDelay()
{
  LOG4CXX_INFO(_logFeb, "Setting active time " << (int) _delay);
  for (auto x : _mpi->boards())
    x.second->reg()->writeAddress(0x222, _delay);
 
}
void lydaq::WiznetManager::setDuration()
{
  LOG4CXX_INFO(_logFeb, " Setting Dead time duration " << (int) _duration);
  for (auto x : _mpi->boards())
    x.second->reg()->writeAddress(0x223, _duration);
 
}
void lydaq::WiznetManager::getLUT(int chan)
{
  LOG4CXX_INFO(_logFeb, " get LUT for " << chan << " on all FEBS");
  for (auto x : _mpi->boards())
    x.second->reg()->writeAddress(0x224,chan);
}
void lydaq::WiznetManager::getCalibrationStatus()
{
  LOG4CXX_INFO(_logFeb, " get Calibration Status for on all FEBS");
  for (auto x : _wiznet->controlSockets())
  {
    this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x225, 0);
    ::usleep(100000);
    this->readShm(x.second->hostTo(), x.second->portTo());
  }
}
void lydaq::WiznetManager::setCalibrationMask(uint64_t mask)
{
  LOG4CXX_INFO(_logFeb, " setCalibrationMask " << std::hex << mask << std::dec << " on all FEBS");
  for (auto x : _wiznet->controlSockets())
  {
    this->writeLongWord(x.second->hostTo(), x.second->portTo(), 0x226, mask);
  }
}
void lydaq::WiznetManager::setMeasurementMask(uint64_t mask,uint32_t feb)
{
  LOG4CXX_INFO(_logFeb, " setMeasurementMask " << std::hex << mask << std::dec << " on  FEBS"<<feb);
  

  
  for (auto x : _wiznet->controlSockets())
  {
    if (feb!=255)
      {
	std::stringstream ip;
	ip << "192.168.10." << feb;
	if (ip.str().compare(x.second->hostTo())!=0)
	  {
	    LOG4CXX_INFO(_logFeb, " setMeasurementMask " << std::hex << mask << std::dec << " skipping"<<feb);
	    continue;
	  }
      }
    this->writeLongWord(x.second->hostTo(), x.second->portTo(), 0x230, mask);
  }
}


/////////////////////////////////////////////////////////
void lydaq::Febv1Manager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
{

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changin thresholds: "<<b0<<","<<b1<<","<<b2);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      if (idif!=0)
	{
	  uint32_t ip=(((it->first)>>32&0XFFFFFFFF)>>16)&0xFFFF;
	  printf("%lx %x %x \n",(it->first>>32),ip,idif);
	  if (idif!=ip) continue;
	}
      it->second.setB0(b0);
      it->second.setB1(b1);
      it->second.setB2(b2);
      //it->second.setHEADER(0x56);
      it->second.dumpBinary();
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::sleep(1);

}
void lydaq::Febv1Manager::setGain(uint16_t gain)
{

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing Gain: "<<gain);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      for (int i=0;i<64;i++)
	it->second.setPAGAIN(i,gain);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::sleep(1);

}

void lydaq::Febv1Manager::setMask(uint32_t level,uint64_t mask)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing Mask: "<<level<<" "<<std::hex<<mask<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASK(level,mask);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}
void lydaq::Febv1Manager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing Mask: "<<level<<" "<<std::hex<<channel<<std::dec);
  for (auto it=_hca->asicMap().begin();it!=_hca->asicMap().end();it++)
    {
      
      it->second.setMASKChannel(level,channel,val==1);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}

void lydaq::Febv1Manager::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  // Create run file
  Json::Value jc=m->content();
  _run=jc["run"].asInt();

  // Clear buffers
  for (auto x:_mpi->boards())
    {
      x.second->data()->clear();
    }

  // Turn run type on
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
      x.second->reg()->writeRegister(lydaq::febv1::Message::Register::ACQ_CTRL,1);
    }
}
void lydaq::Febv1Manager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      x.second->reg()->writeRegister(lydaq::febv1::Message::Register::ACQ_CTRL,0);
    }
  ::sleep(2);


}
void lydaq::Febv1Manager::destroy(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"CLOSE called ");
  
  _mpi->close();
  for (auto x:_mpi->boards())
    delete x.second;
  _mpi->boards().clear();
  delete _mpi;
  _mpi=0;

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data sockets deleted");



  // To be done: _febv1->clear();
}

