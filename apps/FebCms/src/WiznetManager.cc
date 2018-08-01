#include "WiznetManager.hh"
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


lydaq::WiznetManager::WiznetManager(std::string name) : zdaq::baseApplication(name),_context(NULL)
{
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::WiznetManager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::WiznetManager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::WiznetManager::configure, this,_1));
  
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::WiznetManager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&lydaq::WiznetManager::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::WiznetManager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&lydaq::WiznetManager::destroy, this,_1));
  
  
  
  //_fsm->addCommand("JOBLOG",boost::bind(&lydaq::WiznetManager::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS",boost::bind(&lydaq::WiznetManager::c_status,this,_1,_2));
  _fsm->addCommand("DIFLIST",boost::bind(&lydaq::WiznetManager::c_diflist,this,_1,_2));
  _fsm->addCommand("SET6BDAC",boost::bind(&lydaq::WiznetManager::c_set6bdac,this,_1,_2));
  _fsm->addCommand("SETVTHTIME",boost::bind(&lydaq::WiznetManager::c_setvthtime,this,_1,_2));
  _fsm->addCommand("SETMASK",boost::bind(&lydaq::WiznetManager::c_setMask,this,_1,_2));
  _fsm->addCommand("DOWNLOADDB",boost::bind(&lydaq::WiznetManager::c_downloadDB,this,_1,_2));

  _fsm->addCommand("SETMODE",boost::bind(&lydaq::WiznetManager::c_setMode,this,_1,_2));
  _fsm->addCommand("SETDELAY",boost::bind(&lydaq::WiznetManager::c_setDelay,this,_1,_2));
  _fsm->addCommand("SETDURATION",boost::bind(&lydaq::WiznetManager::c_setDuration,this,_1,_2));

  
  
 
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Service "<<name<<" is starting on "<<atoi(wp));

      
    _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink
  _wiznet= new lydaq::WiznetInterface();
  _msg=new lydaq::WiznetMessage();
}
void lydaq::WiznetManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Status CMD called ");
  response["STATUS"]="DONE";

  Json::Value jl;
  for (auto x:_vTdc)
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
  response["TDCSTATUS"]=jl;
}
void lydaq::WiznetManager::c_diflist(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Diflist CMD called ");
  response["STATUS"]="DONE";
  response["DIFLIST"]="EMPTY";

  Json::Value jl;
  for (auto x:_vTdc)
    {
      if (x==NULL) continue;
      Json::Value jt;
      jt["detid"]=x->detectorId();
      jt["sourceid"]=x->difId();
      jl.append(jt);
    }
  response["DIFLIST"]=jl;
}

void lydaq::WiznetManager::c_set6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Set6bdac called ");
  response["STATUS"]="DONE";

  
  uint32_t nc=atol(request.get("value","31").c_str());
  
  this->set6bDac(nc&0xFF);
  response["6BDAC"]=nc;
}
void lydaq::WiznetManager::c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"set VThTime called ");
  response["STATUS"]="DONE";

  
  uint32_t nc=atol(request.get("value","380").c_str());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Value set "<<nc);
  this->setVthTime(nc);
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Completed "<<nc);
  response["VTHTIME"]=nc;
}
void lydaq::WiznetManager::c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called ");
  response["STATUS"]="DONE";

  
  //uint32_t nc=atol(request.get("value","4294967295").c_str());
uint32_t nc;
sscanf(request.get("value","4294967295").c_str(),"%u",&nc);
  
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMask called "<<std::hex<<nc<<std::dec<<" parameter "<<request.get("value","4294967295"));
  this->setMask(nc);
  response["MASK"]=nc;
}
void lydaq::WiznetManager::c_setDelay(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMode called ");
  response["STATUS"]="DONE";


   uint8_t delay=atol(request.get("value","255").c_str());
   _delay=delay;
   this->setDelay();
   LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetDelay called with"<<delay<<" "<<_delay );
  response["MODE"]=_delay;
}
void lydaq::WiznetManager::c_setDuration(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMode called ");
  response["STATUS"]="DONE";


   uint8_t duration=atol(request.get("value","255").c_str());
   _duration=duration;
   this->setDuration();
   LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Setduration called with"<<duration<<" "<<_duration );
  response["MODE"]=_duration;
}

void lydaq::WiznetManager::c_setMode(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMode called ");
  response["STATUS"]="DONE";


   uint32_t mode=atol(request.get("value","2").c_str());
   if (mode!=2)
     _type=mode;
   LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"SetMode called with"<<mode<<" "<<_type );
  response["MODE"]=_type;
}
void lydaq::WiznetManager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"downloadDB called ");
  response["STATUS"]="DONE";


  
  std::string dbstate=request.get("state","NONE");
  Json::Value jTDC=this->parameters()["tdc"];
   if (jTDC.isMember("db"))
     {
       Json::Value jTDCdb=jTDC["db"];
       _tca->clear();
       _tca->parseDb(dbstate,jTDCdb["mode"].asString());
     }
  response["DBSTATE"]=dbstate;
}

void lydaq::WiznetManager::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"****** CMD: "<<m->command());
//  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  Json::Value jtype=this->parameters()["type"];
  _type=jtype.asInt();
  printf ("_type =%d\n",_type); 

   // Need a TDC tag
   if (m->content().isMember("tdc"))
     {
     printf ("found tdc/n");
       this->parameters()["tdc"]=m->content()["tdc"];
     }
   if (!this->parameters().isMember("tdc"))
     {
       LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No tdc tag found ");
       return;
     }
   // Now create the Message handler
   _wiznet->initialise();

   
   Json::Value jTDC=this->parameters()["tdc"];
   //_msh =new lydaq::TdcMessageHandler("/dev/shm");
   if (!jTDC.isMember("network"))
     {
       LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No tdc:network tag found ");
       return;
     }
   // Scan the network
   std::map<uint32_t,std::string> diflist=lydaq::WiznetMessageHandler::scanNetwork(jTDC["network"].asString());
   // Download the configuration
   _tca=new lydaq::TdcConfigAccess();
   if (jTDC.isMember("json"))
     {
       Json::Value jTDCjson=jTDC["json"];
       if (jTDCjson.isMember("file"))
	 {
	   _tca->parseJsonFile(jTDCjson["file"].asString());
	 }
       else
	 if (jTDCjson.isMember("url"))
	   {
	     _tca->parseJsonUrl(jTDCjson["url"].asString());
	   }
     }
    if (jTDC.isMember("db"))
     {
       Json::Value jTDCdb=jTDC["db"];
       _tca->parseDb(jTDCdb["state"].asString(),jTDCdb["mode"].asString());
     }
   if (_tca->asicMap().size()==0)
     {
        LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No ASIC found in the configuration ");
       return;
     }
   // Initialise the network
   for (auto x:_tca->asicMap())
     {
       uint32_t eip= ((x.first)>>32)&0XFFFFFFFF;
       std::map<uint32_t,std::string>::iterator idif=diflist.find(eip);
       if (idif==diflist.end()) continue;

       lydaq::TdcWiznet* _tdc=new lydaq::TdcWiznet(eip);
       // Slow control
       _wiznet->addCommunication(idif->second,10001);
       _wiznet->registerDataHandler(idif->second,10001,boost::bind(&lydaq::TdcWiznet::processSlc, _tdc,_1,_2,_3));

       
   
	 // TDC
       _wiznet->addCommunication(idif->second,10002);
       _wiznet->registerDataHandler(idif->second,10002,boost::bind(&lydaq::TdcWiznet::processBuffer, _tdc,_1,_2,_3));

       _vTdc.push_back(_tdc);
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
  for (auto x:_vTdc)
    x->connect(_context,this->parameters()["publish"].asString());

  // Listen All Wiznet sockets
  _wiznet->listen();

  
}

void lydaq::WiznetManager::writeAddress(std::string host,uint32_t port,uint16_t addr,uint16_t val)
{
  _msg->_address=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(host)<<32)|port;
  _msg->_length =4;
  _msg->_buf[0]=htons(0xFF00);
  _msg->_buf[1]=htons(1);
  _msg->_buf[2]=htons(addr);
  _msg->_buf[3]=htons(val);
  _wiznet->sendMessage(_msg);
}
void lydaq::WiznetManager::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

   // Now loop on slowcontrol socket


  for (auto x:_wiznet->controlSockets())
    {
      this->writeAddress(x.second->hostTo(),x.second->portTo(),0x220,0); //Stop acquisition
      
      _tca->prepareSlowControl(x.second->hostTo());

      _wiznet->writeRamAvm(x.second,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

    }

}

void lydaq::WiznetManager::set6bDac(uint8_t dac)
{

  ::sleep(1);

 
  for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
    {
      for (int i=0;i<32;i++)
	{
	  it->second.set6bDac(i,dac);
	}      
    }
  // Now loop on slowcontrol socket
    for (auto x:_wiznet->controlSockets())
    {
      
      _tca->prepareSlowControl(x.second->hostTo());

      _wiznet->writeRamAvm(x.second,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

    }

  ::sleep(1);

}
void lydaq::WiznetManager::setMask(uint32_t mask)
{

  ::sleep(1);
    // Change all Asics VthTime
  for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
    {
      for (int i=0;i<32;i++)
	{
	  if ((mask>>i)&1)
	    {
	      it->second.setMaskDiscriTime(i,0);
	    }
	  else
	    {
	      it->second.setMaskDiscriTime(i,1);
	    }
	}
      

    }

  // Now loop on slowcontrol socket
    for (auto x:_wiznet->controlSockets())
    {
      
      _tca->prepareSlowControl(x.second->hostTo());

      _wiznet->writeRamAvm(x.second,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

    }

  ::sleep(1);

}

void lydaq::WiznetManager::setVthTime(uint32_t vth)
{

    LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Debut ");
    for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
      it->second.setVthTime(vth);


  // Now loop on slowcontrol socket
    for (auto x:_wiznet->controlSockets())
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data send to "<<x.second->hostTo());      
      _tca->prepareSlowControl(x.second->hostTo());

      _wiznet->writeRamAvm(x.second,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

    }
    LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Fin ");
}



void lydaq::WiznetManager::setDelay()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Setting delays "<<_delay);
  for (auto x:_wiznet->controlSockets())
    {
      this->writeAddress(x.second->hostTo(),x.second->portTo(),0x222,_delay); 
    }
}
void lydaq::WiznetManager::setDuration()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Setting duration "<<_duration);
  for (auto x:_wiznet->controlSockets())
    {
      this->writeAddress(x.second->hostTo(),x.second->portTo(),0x223,_duration); 
    }
}
void lydaq::WiznetManager::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  // Create run file
  Json::Value jc=m->content();
  _run=jc["run"].asInt();

  // Clear buffers
  for (auto x:_vTdc)
     {
       x->clear();
     }

  // Turn run type on
   
  switch (_type)
    {
    case 0:		// ilc mode	
      {
	LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Starting ILC "<<_type);
	for (auto x:_wiznet->controlSockets())
	  {
	    this->writeAddress(x.second->hostTo(),x.second->portTo(),0x219,0); //ILC Mode
	    this->writeAddress(x.second->hostTo(),x.second->portTo(),0x220,1); //Sart acquisition
	  }
      
      break;
      }
    case 1:		// beamtest mode
      {
	LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Starting Beamtest "<<_type);
	for (auto x:_wiznet->controlSockets())
	  {
	    this->writeAddress(x.second->hostTo(),x.second->portTo(),0x219,1); // Beam test Mode
	    this->writeAddress(x.second->hostTo(),x.second->portTo(),0x220,1); //Sart acquisition
	  }
      break;
      }
     
    }
}
void lydaq::WiznetManager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  for (auto x:_wiznet->controlSockets())
    {
      
      this->writeAddress(x.second->hostTo(),x.second->portTo(),0x220,0); // Stop
    }

  ::sleep(2);


}
void lydaq::WiznetManager::destroy(zdaq::fsmmessage* m)
{

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
 
  for (auto x:_vTdc)
    delete x;
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data sockets deleted");
  _vTdc.clear();

  // To be done: _wiznet->clear();
}

