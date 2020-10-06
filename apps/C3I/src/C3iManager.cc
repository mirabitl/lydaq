#include "C3iManager.hh"
using namespace lytdc;
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


lydaq::C3iManager::C3iManager(std::string name) : zdaq::baseApplication(name),_context(NULL),_hca(NULL),_mpi(NULL)
{
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::C3iManager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::C3iManager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::C3iManager::configure, this,_1));
  
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::C3iManager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&lydaq::C3iManager::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::C3iManager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&lydaq::C3iManager::destroy, this,_1));
  
  
  
  //_fsm->addCommand("JOBLOG",boost::bind(&lydaq::C3iManager::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS",boost::bind(&lydaq::C3iManager::c_status,this,_1,_2));
  _fsm->addCommand("STARTACQ",boost::bind(&lydaq::C3iManager::c_startacq,this,_1,_2));
  _fsm->addCommand("STOPACQ",boost::bind(&lydaq::C3iManager::c_stopacq,this,_1,_2));
  _fsm->addCommand("RESET",boost::bind(&lydaq::C3iManager::c_reset,this,_1,_2));
  _fsm->addCommand("STORESC",boost::bind(&lydaq::C3iManager::c_storesc,this,_1,_2));
  _fsm->addCommand("LOADSC",boost::bind(&lydaq::C3iManager::c_loadsc,this,_1,_2));
  _fsm->addCommand("READSC",boost::bind(&lydaq::C3iManager::c_readsc,this,_1,_2));
  _fsm->addCommand("LASTABCID",boost::bind(&lydaq::C3iManager::c_lastabcid,this,_1,_2));
  _fsm->addCommand("LASTGTC",boost::bind(&lydaq::C3iManager::c_lastgtc,this,_1,_2));
  
  _fsm->addCommand("SETTHRESHOLDS",boost::bind(&lydaq::C3iManager::c_setthresholds,this,_1,_2));
  _fsm->addCommand("SETPAGAIN",boost::bind(&lydaq::C3iManager::c_setpagain,this,_1,_2));
  _fsm->addCommand("SETMASK",boost::bind(&lydaq::C3iManager::c_setmask,this,_1,_2));
  _fsm->addCommand("SETCHANNELMASK",boost::bind(&lydaq::C3iManager::c_setchannelmask,this,_1,_2));
  _fsm->addCommand("DOWNLOADDB",boost::bind(&lydaq::C3iManager::c_downloadDB,this,_1,_2));
  _fsm->addCommand("CLOSE",boost::bind(&lydaq::C3iManager::c_close,this,_1,_2));
  _fsm->addCommand("PULSE",boost::bind(&lydaq::C3iManager::c_pulse,this,_1,_2));

  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Service "<<name<<" is starting on "<<atoi(wp));

      
    _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink

  _msg=new lydaq::MpiMessage();
}
void lydaq::C3iManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Status CMD called ");
  response["STATUS"]="DONE";

  Json::Value jl;
  for (auto x:_vC3i)
    {

      Json::Value jt;
      jt["detid"]=x->detectorId();
      std::stringstream sid;
      sid<<std::hex<<x->difId()<<std::dec;
      jt["sourceid"]=sid.str();
      jt["SLC"]=x->slcStatus();
      jt["gtc"]=x->gtc();
      jt["abcid"]=(Json::Value::UInt64)x->abcid();
      jt["event"]=x->event();
      jt["triggers"]=x->triggers();
      jl.append(jt);
    }
  response["C3ISTATUS"]=jl;
}

void lydaq::C3iManager::c_startacq(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"STARTACQ CMD called ");

 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::STARTACQ);
    }
  response["STATUS"]="DONE";
}

void lydaq::C3iManager::c_stopacq(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"STOPACQ CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::STOPACQ);
    }
  response["STATUS"]="DONE";  
}

void lydaq::C3iManager::c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"RESET CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::RESET);
    }
  response["STATUS"]="DONE"; 
}

void lydaq::C3iManager::c_storesc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"STORESC CMD called ");

  for (auto x:_mpi->controlSockets())
    {
      _hca->prepareSlowControl(x.second->hostTo());
      this->sendSlowControl(x.second->hostTo(),x.second->portTo(),_hca->slcBuffer());
    }
  response["STATUS"]="DONE";
}

void lydaq::C3iManager::c_loadsc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"LOADSC CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::LOADSC);
    }
  response["STATUS"]="DONE";
}
void lydaq::C3iManager::c_close(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"CLOSE CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::CLOSE);
    }
  response["STATUS"]="DONE";
}

void lydaq::C3iManager::c_readsc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"READSC CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::READSC);
    }
  response["STATUS"]="DONE";
}

void lydaq::C3iManager::c_lastabcid(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"LOADSC CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::LASTABCID);
    }
  response["STATUS"]="DONE";
}

void lydaq::C3iManager::c_lastgtc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"LOADSC CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::LASTGTC);
    }
  response["STATUS"]="DONE";
}

void lydaq::C3iManager::c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
void lydaq::C3iManager::c_pulse(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Pulse called ");
  response["STATUS"]="DONE";

  
  uint32_t b0=atol(request.get("value","0").c_str());
 for (auto x:_mpi->controlSockets())
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Pulse  "<<x.second->hostTo()<<" "<<x.second->portTo()<<" "<<lydaq::MpiMessage::command::PULSE<<" "<<b0);
      this->sendParameter(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::PULSE,b0);
    }
  

  response["NPULSE"]=b0&0xFF;
}
void lydaq::C3iManager::c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t gain=atol(request.get("gain","128").c_str());
  this->setGain(gain);
  response["GAIN"]=gain;

}

void lydaq::C3iManager::c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
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



void lydaq::C3iManager::c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
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

void lydaq::C3iManager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"downloadDB called ");
  response["STATUS"]="DONE";


  
  std::string dbstate=request.get("state","NONE");
  uint32_t version=atol(request.get("version","0").c_str());
  Json::Value jTDC=this->parameters()["c3i"];
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

void lydaq::C3iManager::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"****** CMD: "<<m->command());
//  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  Json::Value jtype=this->parameters()["type"];
  _type=jtype.asInt();
  printf ("_type =%d\n",_type); 

   // Need a C3I tag
   if (m->content().isMember("c3i"))
     {
     printf ("found c3i/n");
       this->parameters()["c3i"]=m->content()["c3i"];
     }
   if (!this->parameters().isMember("c3i"))
     {
       LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No c3i tag found ");
       return;
     }
   // Now create the Message handler
   if (_mpi==NULL)
     _mpi= new lydaq::c3i::MpiInterface();
   _mpi->initialise();

   
   Json::Value jC3I=this->parameters()["c3i"];
   //_msh =new lydaq::MpiMessageHandler("/dev/shm");
   if (!jC3I.isMember("network"))
     {
       LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No c3i:network tag found ");
       return;
     }
   // Scan the network
   std::map<uint32_t,std::string> diflist=mpi::MpiMessageHandler::scanNetwork(jC3I["network"].asString());
   // Download the configuration
   if (_hca==NULL)
     {
       std::cout<< "Create config acccess"<<std::endl;
       _hca=new lydaq::HR2ConfigAccess();
       _hca->clear();
     }
   std::cout<< " jC3I "<<jC3I<<std::endl;
   if (jC3I.isMember("json"))
     {
       Json::Value jC3Ijson=jC3I["json"];
       if (jC3Ijson.isMember("file"))
	 {
	   _hca->parseJsonFile(jC3Ijson["file"].asString());
	 }
       else
	 if (jC3Ijson.isMember("url"))
	   {
	     _hca->parseJsonUrl(jC3Ijson["url"].asString());
	   }
     }
    if (jC3I.isMember("db"))
     {
              Json::Value jC3Idb=jC3I["db"];
       LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<"Parsing:"<<jC3Idb["state"].asString()<<jC3Idb["mode"].asString());

              
	if (jC3Idb["mode"].asString().compare("mongo")!=0)	
	  _hca->parseDb(jC3Idb["state"].asString(),jC3Idb["mode"].asString());
	else
	  _hca->parseMongoDb(jC3Idb["state"].asString(),jC3Idb["version"].asUInt());
      
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

       LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" New C3I found in db "<<std::hex<<eip<<std::dec);
       vint.push_back(eip);
       lydaq::C3iMpi* _c3i=new lydaq::C3iMpi(eip);
       // REGISTER
       _mpi->addRegister(idif->second,lydaq::c3i::MpiInterface::PORT::REGISTER);
       _mpi->registerDataHandler(idif->second,lydaq::c3i::MpiInterface::PORT::REGISTER,boost::bind(&lydaq::C3iMpi::processBuffer, _c3i,_1,_2,_3));
       
	 // DATA
       _mpi->addDataTransfer(idif->second,lydaq::c3i::MpiInterface::PORT::DATA);
       _mpi->registerDataHandler(idif->second,lydaq::c3i::MpiInterface::PORT::DATA,boost::bind(&lydaq::C3iMpi::processBuffer, _c3i,_1,_2,_3));

       //   SLC
       _mpi->addDataTransfer(idif->second,lydaq::c3i::MpiInterface::PORT::SLC);
       _mpi->registerDataHandler(idif->second,lydaq::c3i::MpiInterface::PORT::SLC,boost::bind(&lydaq::C3iMpi::processBuffer, _c3i,_1,_2,_3));

       _vC3i.push_back(_c3i);

       
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
  for (auto x:_vC3i)
    x->autoRegister(_context,this->configuration(),"BUILDER","collectingPort");
    //x->connect(_context,this->parameters()["publish"].asString());

  // Listen All C3i sockets
  _mpi->listen();

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Init done  "); 
}

void lydaq::C3iManager::processReply(uint32_t adr,uint32_t tr,uint8_t command)
{
  uint8_t b[0x4000];
  for (auto x:_vC3i)
    {
      if (x->address()!=adr) continue;

      uint8_t* rep=x->answer(tr%255);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<adr<<" Trame "<<tr<<" Command "<<command<<" "<<rep[0]<<" C "<<rep[4]);
      int cnt=0;
      while (rep[4]!=command )
	{
	  usleep(1000);
	  cnt++;
	  if (cnt>1000)
	    {
	      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" no return after "<<cnt);
	      return;
	    }
	}
      memcpy(b,rep,0x4000);
      uint16_t* _sBuf= (uint16_t*) &b[1];
      uint16_t length=ntohs(_sBuf[0]); // Header
      uint8_t trame=b[3];
      uint8_t command=b[4];
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" REPLY command ="<<(int) command<<" length="<<length<<" trame id="<<(int) trame);
      
      fprintf(stderr,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
      if (slc) x->setSlcStatus(command);
      for (int i=4;i<length-1;i++)
	{
	  fprintf(stderr,"%.2x ",(b[i]));
	  
	  if ((i-4)%16==15)
	    {
	      fprintf(stderr,"\n");
	    }
	}
      fprintf(stderr,"\n<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
      break;
    }
}
void lydaq::C3iManager::writeRegister(std::string host,uint32_t port,uint16_t& address,uin32_t& value)
{
  uint16_t len=16;
  uint32_t adr= mpi::MpiMessageHandler::convertIP(host);
  _msg->setAddress(( (uint64_t) adr<<32)|port);
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[1]);
  _msg->ptr()[0]='(';
  sp[0]=htons(len);
  _msg->ptr()[4]=WRITEREG;
  memcpy(&(_msg->ptr()[5]),&address,2);
  memcpy(&(_msg->ptr()[7]),&value,4);
  
  _msg->ptr()[len-1]=')';    
  uint32_t tr=_mpi->sendMessage(_msg);
  this->processReply(adr,tr,WRITEREG);
}
void lydaq::C3iManager::readRegister(std::string host,uint32_t port,uint16_t address)
{
  uint16_t len=16;
  uint32_t adr= mpi::MpiMessageHandler::convertIP(host);
  _msg->setAddress(( (uint64_t) adr<<32)|port);
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[1]);
  _msg->ptr()[0]='(';
  sp[0]=htons(len);
  _msg->ptr()[4]=READREG;
  memcpy(&(_msg->ptr()[5]),&address,2);
  _msg->ptr()[len-1]=')';    
  uint32_t tr=_mpi->sendMessage(_msg);
  this->processReply(adr,tr,READREG);
}
void lydaq::C3iManager::sendSlowControl(std::string host,uint32_t port,uint8_t* slc)
{
  uint16_t len=115;
  uint32_t adr= mpi::MpiMessageHandler::convertIP(host);
  _msg->setAddress(( (uint64_t) adr<<32)|port);
  
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[1]);
  _msg->ptr()[0]='(';
  sp[0]=htons(115);
  _msg->ptr()[4]=lydaq::MpiMessage::command::STORESC;
  memcpy(&(_msg->ptr()[5]),slc,109);
  _msg->ptr()[len-1]=')';    
  uint32_t tr=_mpi->sendMessage(_msg);

  this->processReply(adr,tr,(uint8_t) lydaq::MpiMessage::command::STORESC);

  // store send message
#undef DUMPSC
#ifdef DUMPSC
  fprintf(stderr,"\n Slow Control \n==> ");
  for (int i=0;i<109;i++)
    {
      fprintf(stderr,"%.2x ",(slc[i]));
      
      if (i%16==15)
	{
	  fprintf(stderr,"\n==> ");
	}
    }
  fprintf(stderr,"\n");
#endif
}
void lydaq::C3iManager::configureHR2()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" COnfigure the chips ");

   // Now loop on slowcontrol socket


  for (auto x:_mpi->controlSockets())
    {

      
      _hca->prepareSlowControl(x.second->hostTo());

      this->sendSlowControl(x.second->hostTo(),x.second->portTo(),_hca->slcBuffer());
       LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Maintenant on charge ");
       this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::LOADSC,true);

    }

}
void lydaq::C3iManager::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

   // Now loop on slowcontrol socket


  this->configureHR2();

}

void lydaq::C3iManager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
{

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changin thresholds: "<<b0<<","<<b1<<","<<b2);
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
void lydaq::C3iManager::setGain(uint16_t gain)
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

void lydaq::C3iManager::setMask(uint32_t level,uint64_t mask)
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
void lydaq::C3iManager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
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

void lydaq::C3iManager::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  // Create run file
  Json::Value jc=m->content();
  _run=jc["run"].asInt();

  // Clear buffers
  for (auto x:_vC3i)
     {
       x->clear();
     }

  // Turn run type on
    for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::STARTACQ);
    }
}
void lydaq::C3iManager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::STOPACQ);
    }
  ::sleep(2);


}
void lydaq::C3iManager::destroy(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"CLOSE called ");
  for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::CLOSE);
    }

  _mpi->close();
  delete _mpi;
  _mpi=0;
  for (auto x:_vC3i)
    delete x;
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data sockets deleted");
  _vC3i.clear();

  // To be done: _c3i->clear();
}

