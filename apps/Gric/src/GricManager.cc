#include "GricManager.hh"
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


lydaq::GricManager::GricManager(std::string name) : zdaq::baseApplication(name),_context(NULL),_hca(NULL),_mpi(NULL)
{
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::GricManager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::GricManager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::GricManager::configure, this,_1));
  
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::GricManager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&lydaq::GricManager::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::GricManager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&lydaq::GricManager::destroy, this,_1));
  
  
  
  //_fsm->addCommand("JOBLOG",boost::bind(&lydaq::GricManager::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS",boost::bind(&lydaq::GricManager::c_status,this,_1,_2));
  _fsm->addCommand("STARTACQ",boost::bind(&lydaq::GricManager::c_startacq,this,_1,_2));
  _fsm->addCommand("STOPACQ",boost::bind(&lydaq::GricManager::c_stopacq,this,_1,_2));
  _fsm->addCommand("RESET",boost::bind(&lydaq::GricManager::c_reset,this,_1,_2));
  _fsm->addCommand("STORESC",boost::bind(&lydaq::GricManager::c_storesc,this,_1,_2));
  _fsm->addCommand("LOADSC",boost::bind(&lydaq::GricManager::c_loadsc,this,_1,_2));
  _fsm->addCommand("READSC",boost::bind(&lydaq::GricManager::c_readsc,this,_1,_2));
  _fsm->addCommand("LASTABCID",boost::bind(&lydaq::GricManager::c_lastabcid,this,_1,_2));
  _fsm->addCommand("LASTGTC",boost::bind(&lydaq::GricManager::c_lastgtc,this,_1,_2));
  
  _fsm->addCommand("SETTHRESHOLDS",boost::bind(&lydaq::GricManager::c_setthresholds,this,_1,_2));
  _fsm->addCommand("SETPAGAIN",boost::bind(&lydaq::GricManager::c_setpagain,this,_1,_2));
  _fsm->addCommand("SETMASK",boost::bind(&lydaq::GricManager::c_setmask,this,_1,_2));
  _fsm->addCommand("SETCHANNELMASK",boost::bind(&lydaq::GricManager::c_setchannelmask,this,_1,_2));
  _fsm->addCommand("DOWNLOADDB",boost::bind(&lydaq::GricManager::c_downloadDB,this,_1,_2));
  _fsm->addCommand("CLOSE",boost::bind(&lydaq::GricManager::c_close,this,_1,_2));
  _fsm->addCommand("PULSE",boost::bind(&lydaq::GricManager::c_pulse,this,_1,_2));

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
void lydaq::GricManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Status CMD called ");
  response["STATUS"]="DONE";

  Json::Value jl;
  for (auto x:_vGric)
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
  response["GRICSTATUS"]=jl;
}

void lydaq::GricManager::c_startacq(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"STARTACQ CMD called ");

 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::STARTACQ);
    }
  response["STATUS"]="DONE";
}

void lydaq::GricManager::c_stopacq(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"STOPACQ CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::STOPACQ);
    }
  response["STATUS"]="DONE";  
}

void lydaq::GricManager::c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"RESET CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::RESET);
    }
  response["STATUS"]="DONE"; 
}

void lydaq::GricManager::c_storesc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"STORESC CMD called ");

  for (auto x:_mpi->controlSockets())
    {
      _hca->prepareSlowControl(x.second->hostTo());
      this->sendSlowControl(x.second->hostTo(),x.second->portTo(),_hca->slcBuffer());
    }
  response["STATUS"]="DONE";
}

void lydaq::GricManager::c_loadsc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"LOADSC CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::LOADSC);
    }
  response["STATUS"]="DONE";
}
void lydaq::GricManager::c_close(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"CLOSE CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::CLOSE);
    }
  response["STATUS"]="DONE";
}

void lydaq::GricManager::c_readsc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"READSC CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::READSC);
    }
  response["STATUS"]="DONE";
}

void lydaq::GricManager::c_lastabcid(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"LOADSC CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::LASTABCID);
    }
  response["STATUS"]="DONE";
}

void lydaq::GricManager::c_lastgtc(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"LOADSC CMD called ");
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::LASTGTC);
    }
  response["STATUS"]="DONE";
}

void lydaq::GricManager::c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
void lydaq::GricManager::c_pulse(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
void lydaq::GricManager::c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t gain=atol(request.get("gain","128").c_str());
  this->setGain(gain);
  response["GAIN"]=gain;

}

void lydaq::GricManager::c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
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



void lydaq::GricManager::c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response)
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

void lydaq::GricManager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"downloadDB called ");
  response["STATUS"]="DONE";


  
  std::string dbstate=request.get("state","NONE");
  uint32_t version=atol(request.get("version","0").c_str());
  Json::Value jTDC=this->parameters()["gric"];
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

void lydaq::GricManager::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"****** CMD: "<<m->command());
//  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  Json::Value jtype=this->parameters()["type"];
  _type=jtype.asInt();
  printf ("_type =%d\n",_type); 

   // Need a GRIC tag
   if (m->content().isMember("gric"))
     {
     printf ("found gric/n");
       this->parameters()["gric"]=m->content()["gric"];
     }
   if (!this->parameters().isMember("gric"))
     {
       LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No gric tag found ");
       return;
     }
   // Now create the Message handler
   if (_mpi==NULL)
     _mpi= new lydaq::MpiInterface();
   _mpi->initialise();

   
   Json::Value jGRIC=this->parameters()["gric"];
   //_msh =new lydaq::MpiMessageHandler("/dev/shm");
   if (!jGRIC.isMember("network"))
     {
       LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No gric:network tag found ");
       return;
     }
   // Scan the network
   std::map<uint32_t,std::string> diflist=lydaq::MpiMessageHandler::scanNetwork(jGRIC["network"].asString());
   // Download the configuration
   if (_hca==NULL)
     {
       std::cout<< "Create config acccess"<<std::endl;
       _hca=new lydaq::HR2ConfigAccess();
       _hca->clear();
     }
   std::cout<< " jGRIC "<<jGRIC<<std::endl;
   if (jGRIC.isMember("json"))
     {
       Json::Value jGRICjson=jGRIC["json"];
       if (jGRICjson.isMember("file"))
	 {
	   _hca->parseJsonFile(jGRICjson["file"].asString());
	 }
       else
	 if (jGRICjson.isMember("url"))
	   {
	     _hca->parseJsonUrl(jGRICjson["url"].asString());
	   }
     }
    if (jGRIC.isMember("db"))
     {
              Json::Value jGRICdb=jGRIC["db"];
       LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<"Parsing:"<<jGRICdb["state"].asString()<<jGRICdb["mode"].asString());

              
	if (jGRICdb["mode"].asString().compare("mongo")!=0)	
	  _hca->parseDb(jGRICdb["state"].asString(),jGRICdb["mode"].asString());
	else
	  _hca->parseMongoDb(jGRICdb["state"].asString(),jGRICdb["version"].asUInt());
      
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

       LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" New GRIC found in db "<<std::hex<<eip<<std::dec);
       vint.push_back(eip);
       lydaq::GricMpi* _gric=new lydaq::GricMpi(eip);
       // Slow control
       _mpi->addCommunication(idif->second,lydaq::MpiInterface::PORT::CTRL);
       _mpi->registerDataHandler(idif->second,lydaq::MpiInterface::PORT::CTRL,boost::bind(&lydaq::GricMpi::processBuffer, _gric,_1,_2,_3));
       
	 // GRIC
       _mpi->addDataTransfer(idif->second,lydaq::MpiInterface::PORT::DATA);
       _mpi->registerDataHandler(idif->second,lydaq::MpiInterface::PORT::DATA,boost::bind(&lydaq::GricMpi::processBuffer, _gric,_1,_2,_3));

       // Gric Sensor
       _mpi->addDataTransfer(idif->second,lydaq::MpiInterface::PORT::SENSOR);
       _mpi->registerDataHandler(idif->second,lydaq::MpiInterface::PORT::SENSOR,boost::bind(&lydaq::GricMpi::processBuffer, _gric,_1,_2,_3));

       _vGric.push_back(_gric);

       
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
  for (auto x:_vGric)
    x->autoRegister(_context,this->configuration(),"BUILDER","collectingPort");
    //x->connect(_context,this->parameters()["publish"].asString());

  // Listen All Gric sockets
  _mpi->listen();

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Init done  "); 
}

void lydaq::GricManager::processReply(uint32_t adr,uint32_t tr,uint8_t command,bool slc)
{
  uint8_t b[0x4000];
  for (auto x:_vGric)
    {
      if (x->address()!=adr) continue;

      uint8_t* rep=x->answer(tr%255);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<adr<<" Trame "<<tr<<" Command "<<command<<" "<<rep[0]<<" C "<<rep[4]);
      int cnt=0;
      while (rep[4]!=command && rep[4]!=8 && rep[4]!=9)
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
void lydaq::GricManager::sendCommand(std::string host,uint32_t port,uint8_t command,bool slc)
{
  uint16_t len=6;
  uint32_t adr= lydaq::MpiMessageHandler::convertIP(host);
  _msg->setAddress(( (uint64_t) adr<<32)|port);
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[1]);
  _msg->ptr()[0]='(';
  sp[0]=htons(len);
  _msg->ptr()[4]=command;
  _msg->ptr()[len-1]=')';    
  uint32_t tr=_mpi->sendMessage(_msg);
  this->processReply(adr,tr,command,slc);
}
void lydaq::GricManager::sendParameter(std::string host,uint32_t port,uint8_t command,uint8_t par)
{
  uint16_t len=7;
  uint32_t adr= lydaq::MpiMessageHandler::convertIP(host);
  _msg->setAddress(( (uint64_t) adr<<32)|port);
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[1]);
  _msg->ptr()[0]='(';
  sp[0]=htons(len);
  _msg->ptr()[4]=command;
  _msg->ptr()[5]=par;
  _msg->ptr()[len-1]=')';

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" SENDING ="<<(int) command<<" length="<<len<<" parameter="<<(int) par<<" address="<<host <<" port="<<port);
  uint32_t tr=_mpi->sendMessage(_msg);
  this->processReply(adr,tr,command);
}
void lydaq::GricManager::sendSlowControl(std::string host,uint32_t port,uint8_t* slc)
{
  uint16_t len=115;
  uint32_t adr= lydaq::MpiMessageHandler::convertIP(host);
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
void lydaq::GricManager::configureHR2()
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
void lydaq::GricManager::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

   // Now loop on slowcontrol socket


  this->configureHR2();

}

void lydaq::GricManager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif)
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
void lydaq::GricManager::setGain(uint16_t gain)
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

void lydaq::GricManager::setMask(uint32_t level,uint64_t mask)
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
void lydaq::GricManager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
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

void lydaq::GricManager::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  // Create run file
  Json::Value jc=m->content();
  _run=jc["run"].asInt();

  // Clear buffers
  for (auto x:_vGric)
     {
       x->clear();
     }

  // Turn run type on
    for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::STARTACQ);
    }
}
void lydaq::GricManager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
 for (auto x:_mpi->controlSockets())
    {
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::STOPACQ);
    }
  ::sleep(2);


}
void lydaq::GricManager::destroy(zdaq::fsmmessage* m)
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
  for (auto x:_vGric)
    delete x;
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data sockets deleted");
  _vGric.clear();

  // To be done: _gric->clear();
}

