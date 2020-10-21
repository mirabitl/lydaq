#include "C4iManager.hh"
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


lydaq::C4iManager::C4iManager(std::string name) : zdaq::baseApplication(name),_context(NULL),_hca(NULL),_mpi(NULL)
{
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::C4iManager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::C4iManager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::C4iManager::configure, this,_1));
  
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::C4iManager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&lydaq::C4iManager::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::C4iManager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&lydaq::C4iManager::destroy, this,_1));
  
  
  
  //_fsm->addCommand("JOBLOG",boost::bind(&lydaq::C4iManager::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS",boost::bind(&lydaq::C4iManager::c_status,this,_1,_2));
  _fsm->addCommand("RESET",boost::bind(&lydaq::C4iManager::c_reset,this,_1,_2));
  
  _fsm->addCommand("SETTHRESHOLDS",boost::bind(&lydaq::C4iManager::c_setthresholds,this,_1,_2));
  _fsm->addCommand("SETPAGAIN",boost::bind(&lydaq::C4iManager::c_setpagain,this,_1,_2));
  _fsm->addCommand("SETMASK",boost::bind(&lydaq::C4iManager::c_setmask,this,_1,_2));
  _fsm->addCommand("SETCHANNELMASK",boost::bind(&lydaq::C4iManager::c_setchannelmask,this,_1,_2));
  _fsm->addCommand("DOWNLOADDB",boost::bind(&lydaq::C4iManager::c_downloadDB,this,_1,_2));
  _fsm->addCommand("READREG",boost::bind(&lydaq::C4iManager::c_readreg,this,_1,_2));
  _fsm->addCommand("WRITEREG",boost::bind(&lydaq::C4iManager::c_writereg,this,_1,_2));

  _fsm->addCommand("READBME",boost::bind(&lydaq::C4iManager::c_readbme,this,_1,_2));
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Service "<<name<<" is starting on "<<atoi(wp));

      
      _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink


}
void lydaq::C4iManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
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


void lydaq::C4iManager::c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"RESET CMD called ");
  for (auto x:_mpi->boards())
    {
      x.second->reg()->writeRegister(c4i::Message::Register::ACQ_RST,1);
      ::usleep(1000);
      x.second->reg()->writeRegister(c4i::Message::Register::ACQ_RST,0);
    }
  response["STATUS"]="DONE"; 
}

void lydaq::C4iManager::c_readbme(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"RESET CMD called ");
  Json::Value r;
  for (auto x:_mpi->boards())
    {
      r["CAL1"]=x.second->reg()->readRegister(c4i::Message::Register::BME_CAL1);
      r["CAL2"]=x.second->reg()->readRegister(c4i::Message::Register::BME_CAL2);
      r["CAL3"]=x.second->reg()->readRegister(c4i::Message::Register::BME_CAL3);
      r["CAL4"]=x.second->reg()->readRegister(c4i::Message::Register::BME_CAL4);
      r["CAL5"]=x.second->reg()->readRegister(c4i::Message::Register::BME_CAL5);
      r["CAL6"]=x.second->reg()->readRegister(c4i::Message::Register::BME_CAL6);
      r["CAL7"]=x.second->reg()->readRegister(c4i::Message::Register::BME_CAL7);
      r["CAL8"]=x.second->reg()->readRegister(c4i::Message::Register::BME_CAL8);
      r["HUM"]=x.second->reg()->readRegister(c4i::Message::Register::BME_HUM);
      r["PRES"]=x.second->reg()->readRegister(c4i::Message::Register::BME_PRES);
      r["TEMP"]=x.second->reg()->readRegister(c4i::Message::Register::BME_TEMP);
    }
  response["STATUS"]=r; 
}


void lydaq::C4iManager::c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
void lydaq::C4iManager::c_readreg(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
void lydaq::C4iManager::c_writereg(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
void lydaq::C4iManager::c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t gain=atol(request.get("gain","128").c_str());
  this->setGain(gain);
  response["GAIN"]=gain;

}

void lydaq::C4iManager::c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
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



void lydaq::C4iManager::c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
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

void lydaq::C4iManager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"downloadDB called ");
  response["STATUS"]="DONE";


  
  std::string dbstate=request.get("state","NONE");
  uint32_t version=atol(request.get("version","0").c_str());
  Json::Value jTDC=this->parameters()["c4i"];
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

void lydaq::C4iManager::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"****** CMD: "<<m->command());
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  Json::Value jtype=this->parameters()["type"];
  _type=jtype.asInt();
  printf ("_type =%d\n",_type); 

  // Need a C4I tag
  if (m->content().isMember("c4i"))
    {
      printf ("found c4i/n");
      this->parameters()["c4i"]=m->content()["c4i"];
    }
  if (!this->parameters().isMember("c4i"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No c4i tag found ");
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new lydaq::c4i::Interface();
  _mpi->initialise();

   
  Json::Value jC4I=this->parameters()["c4i"];
  //_msh =new lydaq::MpiMessageHandler("/dev/shm");
  if (!jC4I.isMember("network"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No c4i:network tag found ");
      return;
    }
  // Scan the network
  std::map<uint32_t,std::string> diflist=mpi::MpiMessageHandler::scanNetwork(jC4I["network"].asString());
  // Download the configuration
  if (_hca==NULL)
    {
      std::cout<< "Create config acccess"<<std::endl;
      _hca=new lydaq::HR2ConfigAccess();
      _hca->clear();
    }
  std::cout<< " jC4I "<<jC4I<<std::endl;
  if (jC4I.isMember("json"))
    {
      Json::Value jC4Ijson=jC4I["json"];
      if (jC4Ijson.isMember("file"))
	{
	  _hca->parseJsonFile(jC4Ijson["file"].asString());
	}
      else
	if (jC4Ijson.isMember("url"))
	  {
	    _hca->parseJsonUrl(jC4Ijson["url"].asString());
	  }
    }
  if (jC4I.isMember("db"))
    {
      Json::Value jC4Idb=jC4I["db"];
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<"Parsing:"<<jC4Idb["state"].asString()<<jC4Idb["mode"].asString());

              
      if (jC4Idb["mode"].asString().compare("mongo")!=0)	
	_hca->parseDb(jC4Idb["state"].asString(),jC4Idb["mode"].asString());
      else
	_hca->parseMongoDb(jC4Idb["state"].asString(),jC4Idb["version"].asUInt());
      
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
      
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" New C4I found in db "<<std::hex<<eip<<std::dec<<" IP address "<<idif->second);
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

  // Listen All C4i sockets
  _mpi->listen();

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Init done  "); 
}

void lydaq::C4iManager::configureHR2()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" COnfigure the chips ");

  // Turn Off/ On SLC Mode
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(lydaq::c4i::Message::Register::SLC_CTRL,0);
  usleep(100);
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(lydaq::c4i::Message::Register::SLC_CTRL,1);

  // Now loop on slowcontrol socket
  fprintf(stderr,"Loop on socket for Sending slow control \n");
  for (auto x:_mpi->boards())
    {
      _hca->prepareSlowControl(x.second->ipAddress());
      x.second->slc()->sendSlowControl(_hca->slcBuffer());
    }
  
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Maintenant on charge ");
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(lydaq::c4i::Message::Register::SLC_SIZE,109);
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(lydaq::c4i::Message::Register::SLC_CTRL,2);

  usleep(100000);
  // Turn off SLC
  for (auto x:_mpi->boards())
    x.second->reg()->writeRegister(lydaq::c4i::Message::Register::SLC_CTRL,0);

  // Read SLC status
  for (auto x:_mpi->boards())
    {
      uint32_t status=0,cnt=0;
      while(!(status&1))
	{status=x.second->reg()->readRegister(lydaq::c4i::Message::Register::SLC_STATUS);
	  fprintf(stderr,"::::::::::::::: %x \n",status);
	  usleep(1000);
	  cnt++;
	  if (cnt>1000)
	    {
	      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" DIFSTATUS NULL after 1 s ");
	    break;
	    }

	}
      x.second->reg()->setSlcStatus(status);
    }

}
void lydaq::C4iManager::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

  // Now loop on slowcontrol socket


  this->configureHR2();

}

void lydaq::C4iManager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
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
void lydaq::C4iManager::setGain(uint16_t gain)
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

void lydaq::C4iManager::setMask(uint32_t level,uint64_t mask)
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
void lydaq::C4iManager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
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

void lydaq::C4iManager::start(zdaq::fsmmessage* m)
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
      x.second->reg()->writeRegister(lydaq::c4i::Message::Register::ACQ_CTRL,1);
    }
}
void lydaq::C4iManager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      x.second->reg()->writeRegister(lydaq::c4i::Message::Register::ACQ_CTRL,0);
    }
  ::sleep(2);


}
void lydaq::C4iManager::destroy(zdaq::fsmmessage* m)
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



  // To be done: _c4i->clear();
}

