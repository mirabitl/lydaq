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


lydaq::GricManager::GricManager(std::string name) : zdaq::baseApplication(name),_context(NULL)
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

  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Service "<<name<<" is starting on "<<atoi(wp));

      
    _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink
  _mpi= new lydaq::MpiInterface();
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
      jt["sourceid"]=x->difId();
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

  
  uint32_t bo=atol(request.get("B0","250").c_str());
  uint32_t b1=atol(request.get("B1","250").c_str());
  uint32_t b2=atol(request.get("B2","250").c_str());
  
  this->setThresholds(b0,b1,b2);
  response["THRESHOLD0"]=b0;
  response["THRESHOLD1"]=b1;
  response["THRESHOLD2"]=b2;
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
  sscanf(request.get("mask","0XFFFFFFFFFFFFFFFF").c_str(),"%x",&mask);
  uint32_t level=atol(request.get("level","0").c_str());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called "<<std::hex<<mask<<std::dec<<" level "<<level);
  this->setMask(level,mask);
  response["MASK"]=mask;
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
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called "<<std::hex<<mask<<std::dec<<" level "<<level);
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
  Json::Value jTDC=this->parameters()["tdc"];
   if (jTDC.isMember("db"))
     {
       Json::Value jTDCdb=jTDC["db"];
       _hca->clear();
       _hca->parseDb(dbstate,jTDCdb["mode"].asString());
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
   _mpi->initialise();

   
   Json::Value jGRIC=this->parameters()["gric"];
   //_msh =new lydaq::GricMessageHandler("/dev/shm");
   if (!jGRIC.isMember("network"))
     {
       LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No gric:network tag found ");
       return;
     }
   // Scan the network
   std::map<uint32_t,std::string> diflist=lydaq::GricMessageHandler::scanNetwork(jGRIC["network"].asString());
   // Download the configuration
   _tca=new lydaq::GricConfigAccess();
   if (jGRIC.isMember("json"))
     {
       Json::Value jGRICjson=jGRIC["json"];
       if (jGRICjson.isMember("file"))
	 {
	   _tca->parseJsonFile(jGRICjson["file"].asString());
	 }
       else
	 if (jGRICjson.isMember("url"))
	   {
	     _tca->parseJsonUrl(jGRICjson["url"].asString());
	   }
     }
    if (jGRIC.isMember("db"))
     {
       Json::Value jGRICdb=jGRIC["db"];
       _tca->parseDb(jGRICdb["state"].asString(),jGRICdb["mode"].asString());
     }
   if (_tca->asicMap().size()==0)
     {
        LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No ASIC found in the configuration ");
       return;
     }
   LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"ASIC found in the configuration "<<_tca->asicMap().size() );
   // Initialise the network
   std::vector<uint32_t> vint;
   vint.clear();
   for (auto x:_tca->asicMap())
     {
       uint32_t eip= ((x.first)>>32)&0XFFFFFFFF;
       std::map<uint32_t,std::string>::iterator idif=diflist.find(eip);
       if (idif==diflist.end()) continue;
       if ( std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;

       LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" New DIF "<<eip);
       vint.push_back(eip);
       lydaq::GricMpi* _gric=new lydaq::GricMpi(eip);
       // Slow control
       _mpi->addCommunication(idif->second,9760);
       _mpi->registerDataHandler(idif->second,9760,boost::bind(&lydaq::GricMpi::processBuffer, _gric,_1,_2,_3));
       
	 // GRIC
       _mpi->addDataTransfer(idif->second,9761);
       _mpi->registerDataHandler(idif->second,9761,boost::bind(&lydaq::GricMpi::processBuffer, _gric,_1,_2,_3));

       // Gric Sensor
       _mpi->addDataTransfer(idif->second,9762);
       _mpi->registerDataHandler(idif->second,9762,boost::bind(&lydaq::GricMpi::processBuffer, _gric,_1,_2,_3));

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
    x->connect(_context,this->parameters()["publish"].asString());

  // Listen All Gric sockets
  _mpi->listen();

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Init done  "); 
}

void lydaq::GricManager::sendCommand(std::string host,uint32_t port,uint8_t command)
{
  _msg->setAddress(( (uint64_t) lydaq::GricMessageHandler::convertIP(host)<<32)|port);
  _msg->setLength(6);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[1]);
  _msg->ptr()[0]='(';
  sp[0]=htons(6);
  _msg->ptr()[4]=command;
  _msg->ptr()[5]=')';    
  _mpi->sendMessage(_msg);
}
void lydaq::GricManager::sendSlowControl(std::string host,uint32_t port,uint8_t* slc)
{
  _msg->setAddress(( (uint64_t) lydaq::GricMessageHandler::convertIP(host)<<32)|port);
  _msg->setLength(115);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[1]);
  _msg->ptr()[0]='(';
  sp[0]=htons(115);
  _msg->ptr()[4]=lydaq::MpiMessage::command::STORESC;
  mcpy(&(_msg->ptr()[5]),slc,109);
  _msg->ptr()[115]=')';    
  _mpi->sendMessage(_msg);

  // store send message
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
}
void lydaq::GricManager::configureHR2()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" COnfigure the chips ");

   // Now loop on slowcontrol socket


  for (auto x:_mpi->controlSockets())
    {

      
      _hca->prepareSlowControl(x.second->hostTo());

      this->sendSlowControl(x.second->hostTo(),x.second->portTo(),_hca->slcBuffer());
      this->sendCommand(x.second->hostTo(),x.second->portTo(),lydaq::MpiMessage::command::LOADSC);

    }

}
void lydaq::GricManager::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

   // Now loop on slowcontrol socket


  this->configureHR2();

}

void lydaq::GricManager::setThresholds(uint16_t b0,uint16_t b1,uint16_t b2)
{

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changin thresholds: "<<b0<<","<<b1<<","<<b2);
  for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
    {
      it->second.setB0(b0);
      it->second.setB1(b1);
      it->second.setB2(b2);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();
  ::sleep(1);

}
void lydaq::GricManager::setGain(uint16_t gain)
{

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing Gain: "<<gain);
  for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
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
  for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
    {
      
	it->second.setMask(level,mask);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}
void lydaq::GricManager::setChannelMask(uint16_t level,uint16_t channel,uint16_t val)
{
LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Changing Mask: "<<level<<" "<<std::hex<<mask<<std::dec);
  for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
    {
      
	it->second.setMaks(level,mask);
    }
  // Now loop on slowcontrol socket
  this->configureHR2();


  ::sleep(1);

}

void lydaq::GricManager::setVthTime(uint32_t vth)
{

    LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Debut ");
    for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
      {
      it->second.setVthTime(vth);
      // 1 seul ASIC break;
      }

  // Now loop on slowcontrol socket
    for (auto x:_gric->controlSockets())
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data send to "<<x.second->hostTo());      
      _tca->prepareSlowControl(x.second->hostTo());

      _gric->writeRamAvm(x.second,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

    }
    LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Fin ");
}

void lydaq::GricManager::setSingleVthTime(uint32_t vth,uint32_t feb,uint32_t asic)
{
  std::stringstream ip;
  ip<<"192.168.10."<<feb;

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Debut ");
  for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
    {
      uint64_t eid=(((uint64_t) lydaq::GricMessageHandler::convertIP(ip.str()))<<32) | asic;
      if (eid!=it->first) continue;
      it->second.setVthTime(vth);
    }

  // Now loop on slowcontrol socket
  for (auto x:_gric->controlSockets())
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data send to "<<x.second->hostTo());      
      _tca->prepareSlowControl(x.second->hostTo());
      
      _gric->writeRamAvm(x.second,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());
      
    }
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Fin ");
}



void lydaq::GricManager::setDelay()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Setting delays "<<_delay);
  for (auto x:_gric->controlSockets())
    {
      this->writeAddress(x.second->hostTo(),x.second->portTo(),0x222,_delay); 
    }
}
void lydaq::GricManager::setDuration()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Setting duration "<<_duration);
  for (auto x:_gric->controlSockets())
    {
      this->writeAddress(x.second->hostTo(),x.second->portTo(),0x223,_duration); 
    }
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
   
  switch (_type)
    {
    case 0:		// ilc mode	
      {
	LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Starting ILC "<<_type);
	for (auto x:_gric->controlSockets())
	  {
	    this->writeAddress(x.second->hostTo(),x.second->portTo(),0x219,0); //ILC Mode
	    this->writeAddress(x.second->hostTo(),x.second->portTo(),0x220,1); //Sart acquisition
	  }
      
      break;
      }
    case 1:		// beamtest mode
      {
	LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Starting Beamtest "<<_type);
	for (auto x:_gric->controlSockets())
	  {
	    this->writeAddress(x.second->hostTo(),x.second->portTo(),0x219,1); // Beam test Mode
	    this->writeAddress(x.second->hostTo(),x.second->portTo(),0x220,1); //Sart acquisition
	  }
      break;
      }
     
    }
}
void lydaq::GricManager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  for (auto x:_gric->controlSockets())
    {
      
      this->writeAddress(x.second->hostTo(),x.second->portTo(),0x220,0); // Stop
    }

  ::sleep(2);


}
void lydaq::GricManager::destroy(zdaq::fsmmessage* m)
{

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
 
  for (auto x:_vGric)
    delete x;
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data sockets deleted");
  _vGric.clear();

  // To be done: _gric->clear();
}

