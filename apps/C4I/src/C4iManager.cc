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

  _msg=new lydaq::c4i::Message();
}
void lydaq::C4iManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Status CMD called ");
  response["STATUS"]="DONE";

  Json::Value jl;
  for (auto x:_mpi->boards())
    {

      Json::Value jt;
      jt["detid"]=x->data()->detectorId();
      std::stringstream sid;
      sid<<std::hex<<x->datat()->difId()<<std::dec;
      jt["sourceid"]=sid.str();
      jt["SLC"]=x->reg()->slcStatus();
      jt["gtc"]=x->data()->gtc();
      jt["abcid"]=(Json::Value::UInt64)x->data()->abcid();
      jt["event"]=x->data()->event();
      jt["triggers"]=x->data()->triggers();
      jl.append(jt);
    }
  response["C4ISTATUS"]=jl;
}


void lydaq::C4iManager::c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"RESET CMD called ");
  for (auto x:_mpi->boards())
    {
      x->reg()->writeRegister(c4i::Message::Register::ACQ_RST,1);
      ::usleep(1000);
      x->reg()->writeRegister(c4i::Message::Register::ACQ_RST,0);
    }
  response["STATUS"]="DONE"; 
}

void lydaq::C4iManager::c_readbme(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"RESET CMD called ");
  Json::Value r;
  for (auto x:_mpi->boards())
    {
      r["CAL1"]=x->reg()->readRegister(c4i::Message::Register::BME_CAL1);
      r["CAL2"]=x->reg()->readRegister(c4i::Message::Register::BME_CAL2);
      r["CAL3"]=x->reg()->readRegister(c4i::Message::Register::BME_CAL3);
      r["CAL4"]=x->reg()->readRegister(c4i::Message::Register::BME_CAL4);
      r["CAL5"]=x->reg()->readRegister(c4i::Message::Register::BME_CAL5);
      r["CAL6"]=x->reg()->readRegister(c4i::Message::Register::BME_CAL6);
      r["CAL7"]=x->reg()->readRegister(c4i::Message::Register::BME_CAL7);
      r["CAL8"]=x->reg()->readRegister(c4i::Message::Register::BME_CAL8);
      r["HUM"]=x->reg()->readRegister(c4i::Message::Register::BME_HUM);
      r["PRES"]=x->reg()->readRegister(c4i::Message::Register::BME_PRES);
      r["TEMP"]=x->reg()->readRegister(c4i::Message::Register::BME_TEMP);
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
      uint32_t value=x->reg()->readRegister(adr);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Read reg "<<x.second->hostTo()<<" "<<x.second->portTo()<<" Address "<<adr<<" Value "<<value);
      r[x.second->hostTo()]=value;
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
      x->reg()->writeRegister(adr,val);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Write reg "<<x.second->hostTo()<<" "<<x.second->portTo()<<" Address "<<adr<<" Value "<<val);
      r[x.second->hostTo()]=val;
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

      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" New C4I found in db "<<std::hex<<eip<<std::dec);
      vint.push_back(eip);
      lydaq::C4iMpi* _c4i=new lydaq::C4iMpi(eip);
      std::pair<uint32_t,lydaq::C4iMpi*> p(eip,_c4i);
      // REGISTER
      _mpi->addRegister(idif->second,lydaq::c4i::MpiInterface::PORT::REGISTER);
      _mpi->registerDataHandler(idif->second,lydaq::c4i::MpiInterface::PORT::REGISTER,boost::bind(&lydaq::C4iMpi::processBuffer, _c4i,_1,_2,_3));
       
      // DATA
      _mpi->addDataTransfer(idif->second,lydaq::c4i::MpiInterface::PORT::DATA);
      _mpi->registerDataHandler(idif->second,lydaq::c4i::MpiInterface::PORT::DATA,boost::bind(&lydaq::C4iMpi::processBuffer, _c4i,_1,_2,_3));

      //   SLC
      _mpi->addSlcTransfer(idif->second,lydaq::c4i::MpiInterface::PORT::SLC);
      _mpi->registerDataHandler(idif->second,lydaq::c4i::MpiInterface::PORT::SLC,boost::bind(&lydaq::C4iMpi::processBuffer, _c4i,_1,_2,_3));

      _vC4i.push_back(_c4i);
      _mC4i.insert(p);
       
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
  for (auto x:_vC4i)
    x->autoRegister(_context,this->configuration(),"BUILDER","collectingPort");
  //x->connect(_context,this->parameters()["publish"].asString());

  // Listen All C4i sockets
  _mpi->listen();

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Init done  "); 
}

void lydaq::C4iManager::processReply(uint32_t adr,uint32_t tr,uint8_t command,uint32_t* reply)
{
  uint8_t b[0x4000];
  for (auto x:_vC4i)
    {
      if (x->address()!=adr) continue;

      uint8_t* rep=x->answer(tr%255);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<adr<<" Trame "<<tr<<" Command "<<command<<" "<<rep[0]<<" C "<<rep[4]);
      int cnt=0;
      while (rep[C4I_FMT_CMD]!=lydaq::c4i::MpiMessage::ACKNOWLEDGE )
	{
	  usleep(1000);
	  cnt++;
	  if (cnt>1000)
	    {
	      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" no return after "<<cnt);
	      break;
	    }
	}

      // Dump returned buffer
      memcpy(b,rep,0x4000);
      uint16_t* _sBuf= (uint16_t*) &b[C4I_FMT_LEN];
      uint16_t length=ntohs(_sBuf[0]); // Header
      uint8_t trame=b[C4I_FMT_TRANS];
      uint8_t command=b[C4I_FMT_CMD];
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" REPLY command ="<<(int) command<<" length="<<length<<" trame id="<<(int) trame);
      
      fprintf(stderr,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
      
      for (int i=C4I_FMT_PAYLOAD;i<length-1;i++)
	{
	  fprintf(stderr,"%.2x ",(b[i]));
	  
	  if ((i-4)%16==15)
	    {
	      fprintf(stderr,"\n");
	    }
	}
      fprintf(stderr,"\n<<<<<<<<<<<<<<<<<<<<<<<<<<\n");

        if (reply!=0) //special case for read register
    {
      memcpy(reply,&rep[C4I_FMT_PAYLOAD+2],sizeof(uint32_t));
      return;
    }

      break;
    }

}
void lydaq::C4iManager::writeRegister(std::string host,uint32_t port,uint16_t address,uint32_t value)
{
  uint16_t len=16;
  uint32_t adr= mpi::MpiMessageHandler::convertIP(host);
  _msg->setAddress(( (uint64_t) adr<<32)|port);
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[C4I_FMT_LEN]);
  _msg->ptr()[C4I_FMT_HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[C4I_FMT_CMD]=lydaq::c4i::MpiMessage::command::WRITEREG;
  uint16_t radr=htons(address);uint32_t rval=htonl(value);
  memcpy(&(_msg->ptr()[C4I_FMT_PAYLOAD]),&radr,2);
  memcpy(&(_msg->ptr()[C4I_FMT_PAYLOAD+2]),&rval,4);
  
  _msg->ptr()[len-1]=')';    
  uint32_t tr=_mpi->sendMessage(_msg);
  this->processReply(adr,0,lydaq::c4i::MpiMessage::command::WRITEREG,0);
}
uint32_t lydaq::C4iManager::readRegister(std::string host,uint32_t port,uint16_t address)
{
  uint16_t len=16;
  uint32_t adr= mpi::MpiMessageHandler::convertIP(host);
  _msg->setAddress(( (uint64_t) adr<<32)|port);
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[C4I_FMT_LEN]);
  _msg->ptr()[C4I_FMT_HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[C4I_FMT_CMD]=lydaq::c4i::MpiMessage::command::READREG;
  uint16_t radr=htons(address);
  memcpy(&(_msg->ptr()[C4I_FMT_PAYLOAD]),&radr,2);
  _msg->ptr()[len-1]=')';    
  uint32_t tr=_mpi->sendMessage(_msg);
  uint32_t rep=0;
  this->processReply(adr,0,lydaq::c4i::MpiMessage::command::READREG,&rep);
  return ntohl(rep);
}
void lydaq::C4iManager::sendSlowControl(std::string host,uint32_t port,uint8_t* slc)
{
  uint16_t len=116;
  uint32_t adr= mpi::MpiMessageHandler::convertIP(host);
  _msg->setAddress(( (uint64_t) adr<<32)|port);
  
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[C4I_FMT_LEN]);
  _msg->ptr()[C4I_FMT_HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[C4I_FMT_CMD]=lydaq::c4i::MpiMessage::command::SLC;
  memcpy(&(_msg->ptr()[C4I_FMT_PAYLOAD]),slc,109);
  _msg->ptr()[len-1]=')';
  //fprintf(stderr,"Sending slow control %s %d  %x \n",host.c_str(),port,slc);
  uint32_t tr=_mpi->sendSlcMessage(_msg);
  //fprintf(stderr,"processing reply slow control \n");
  this->processReply(adr,0,(uint8_t) lydaq::c4i::MpiMessage::command::SLC,0);
}
void lydaq::C4iManager::configureHR2()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" COnfigure the chips ");

  // Turn On SLC Mode
    for (auto x:_mpi->controlSockets())
      this->writeRegister(x.second->hostTo(),x.second->portTo(),lydaq::c4i::MpiMessage::Register::SLC_CTRL,1);

  // Now loop on slowcontrol socket
    fprintf(stderr,"Loop on socket for Sending slow control \n");
  for (auto x:_mpi->slcSockets())
    {
      _hca->prepareSlowControl(x.second->hostTo());
      this->sendSlowControl(x.second->hostTo(),x.second->portTo(),_hca->slcBuffer());
    }
  
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Maintenant on charge ");
  for (auto x:_mpi->controlSockets())
      this->writeRegister(x.second->hostTo(),x.second->portTo(),lydaq::c4i::MpiMessage::Register::SLC_CTRL,2);

  // Turn off SLC

  for (auto x:_mpi->controlSockets())
    this->writeRegister(x.second->hostTo(),x.second->portTo(),lydaq::c4i::MpiMessage::Register::SLC_CTRL,0);

  // Read SLC status

  for (auto x:_mpi->controlSockets())
    {
      uint32_t status=x->reg()->readRegister(c4i::Message::Register::SLC_STATUS);
      uint32_t adr= mpi::MpiMessageHandler::convertIP(x.second->hostTo());
      if (auto ic3=_mC4i.find(adr)!=_mC4i.end())
	_mC4i[adr]->setSlcStatus(status);
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
  for (auto x:_vC4i)
    {
      x->clear();
    }

  // Turn run type on
  for (auto x:_mpi->controlSockets())
    {
      // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
      this->writeRegister(x.second->hostTo(),x.second->portTo(),lydaq::c4i::MpiMessage::Register::ACQ_CTRL,1);
    }
}
void lydaq::C4iManager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  for (auto x:_mpi->controlSockets())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      this->writeRegister(x.second->hostTo(),x.second->portTo(),lydaq::c4i::MpiMessage::Register::ACQ_CTRL,0);

    }
  ::sleep(2);


}
void lydaq::C4iManager::destroy(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"CLOSE called ");
  _mpi->close();
  delete _mpi;
  _mpi=0;
  for (auto x:_vC4i)
    delete x;
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data sockets deleted");
  _vC4i.clear();
  _mC4i.clear();

  // To be done: _c4i->clear();
}

