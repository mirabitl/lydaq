
#include "LMdccEcalServer.hh"
#include "fileTailer.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LMdccEcalServer::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

  std::string device;
  if (m->content().isMember("device"))
    { 
      device=m->content()["device"].asString();
      this->parameters()["device"]=m->content()["device"];
    }
  else
    device=this->parameters()["device"].asString();

  doOpen(device);
  
  if (m->content().isMember("spillon"))
    this->parameters()["spillon"]=m->content()["spillon"];
  
  if (m->content().isMember("spilloff"))
    this->parameters()["spilloff"]=m->content()["spilloff"];

  if (m->content().isMember("spillregister"))
    this->parameters()["spillregister"]=m->content()["spillregister"];
  
  if (this->parameters().isMember("spillon") && _mdcc!=NULL)
    {
      _mdcc->setSpillOn(this->parameters()["spillon"].asInt()); 
    }
  if (this->parameters().isMember("spilloff") && _mdcc!=NULL)
    {
      _mdcc->setSpillOff(this->parameters()["spilloff"].asInt()); 
    }
  _mdcc->maskTrigger();
  _mdcc->resetCounter();
}
void lydaq::LMdccEcalServer::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       return;
    }
  _mdcc->close();
  _mdcc=NULL;
}
void lydaq::LMdccEcalServer::pause(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       return;
    }
  _mdcc->maskTrigger();
}
void lydaq::LMdccEcalServer::resume(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       return;
    }
  _mdcc->unmaskTrigger();
}
void lydaq::LMdccEcalServer::ecalpause(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       return;
    }
  _mdcc->maskEcal();
}
void lydaq::LMdccEcalServer::ecalresume(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       return;
    }
  _mdcc->unmaskEcal();
}
void lydaq::LMdccEcalServer::reset(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       return;
    }
  _mdcc->resetCounter();
}


void lydaq::LMdccEcalServer::c_pause(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Pause called ");

  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       response["STATUS"]="Please open MDC01 first";
       return;
    }
  _mdcc->maskTrigger();
  response["STATUS"]="DONE";
}
void lydaq::LMdccEcalServer::c_resume(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
    LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Resume called ");

  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       response["STATUS"]="Please open MDC01 first";
       return;
    }
  _mdcc->unmaskTrigger();
  response["STATUS"]="DONE";
}
void lydaq::LMdccEcalServer::c_ecalpause(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Ecal Pause called ");
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       response["STATUS"]="Please open MDC01 first";
       return;
    }
  _mdcc->maskEcal();
  response["STATUS"]="DONE";
}
void lydaq::LMdccEcalServer::c_ecalresume(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Ecal Resume called ");
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       response["STATUS"]="Please open MDC01 first";
       return;
    }
  _mdcc->unmaskEcal();
  response["STATUS"]="DONE";
}

void lydaq::LMdccEcalServer::c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
   LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" RESET called ");
  if (_mdcc==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,__PRETTY_FUNCTION__<<"Please open MDC01 first");
       response["STATUS"]="Please open MDC01 first";
       return;
    }
  _mdcc->resetCounter();
  response["STATUS"]="DONE";
}

void lydaq::LMdccEcalServer::c_readreg(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
   LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Read Register called ");
  if (_mdcc==NULL)    {response["STATUS"]="NO Mdcc created"; return;}
  uint32_t adr=atol(request.get("address","2").c_str());
  uint32_t val =_mdcc->readRegister(adr);

  response["STATUS"]="DONE";
  response["ADDRESS"]=adr;
  response["VALUE"]=val;
} 
void lydaq::LMdccEcalServer::c_writereg(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Write Register called ");
  if (_mdcc==NULL)    {response["STATUS"]="NO Mdcc created"; return;}
  uint32_t adr=atol(request.get("address","2").c_str());
  uint32_t value=atol(request.get("value","1234").c_str());
  _mdcc->writeRegister(adr,value);

  response["STATUS"]="DONE";
  response["ADDRESS"]=adr;
  response["VALUE"]=value;
} 
void lydaq::LMdccEcalServer::c_spillon(Mongoose::Request &request, Mongoose::JsonResponse &response)
{

  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Spill ON called ");
  if (_mdcc==NULL)    {response["STATUS"]="NO Mdcc created"; return;}
  uint32_t nc=atol(request.get("nclock","50").c_str());
  _mdcc->setSpillOn(nc);

  response["STATUS"]="DONE";
  response["NCLOCK"]=nc;

} 
void lydaq::LMdccEcalServer::c_spilloff(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Spill Off called ");
  if (_mdcc==NULL)    {response["STATUS"]="NO Mdcc created"; return;}
  uint32_t nc=atol(request.get("nclock","5000").c_str());
  _mdcc->setSpillOff(nc);

  response["STATUS"]="DONE";
  response["NCLOCK"]=nc;

} 
void lydaq::LMdccEcalServer::c_setregister(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Set register called ");
  if (_mdcc==NULL)    {response["STATUS"]="NO Mdcc created"; return;}
  uint32_t adr=atol(request.get("address","2").c_str());
  uint32_t val=atol(request.get("value","0").c_str());
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Set register called with "<<adr<<" => "<<val);

  _mdcc->writeRegister(adr,val);

  response["STATUS"]="DONE";
  response["VALUE"]=_mdcc->readRegister(adr);

}
void lydaq::LMdccEcalServer::c_getregister(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Get register called ");
  if (_mdcc==NULL)    {response["STATUS"]="NO Mdcc created"; return;}
  uint32_t adr=atol(request.get("address","2").c_str());
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<"Get register called with "<<adr);

  response["STATUS"]="DONE";
  response["VALUE"]=_mdcc->readRegister(adr);
  //std::cout<<response<<std::endl;
}

void lydaq::LMdccEcalServer::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Status called ");
  if (_mdcc==NULL)    {response["STATUS"]="NO Mdcc created"; return;}
  Json::Value rc;
  rc["version"]=_mdcc->version();
  rc["id"]=_mdcc->id();
  rc["mask"]=_mdcc->mask();
  rc["spill"]=_mdcc->spillCount();
  rc["busy1"]=_mdcc->busy1Count();
  rc["busy2"]=_mdcc->busy2Count();
  rc["busy3"]=_mdcc->busy3Count();
  rc["spillon"]=_mdcc->spillOn();
  rc["spilloff"]=_mdcc->spillOff();
  rc["ecalmask"]=_mdcc->ecalmask();
  rc["calib"]=_mdcc->calibCount();
  response["COUNTERS"]=rc;
  response["STATUS"]="DONE";


} 






lydaq::LMdccEcalServer::LMdccEcalServer(std::string name) : zdaq::baseApplication(name),_mdcc(NULL)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();

  
// Register state
  _fsm->addState("PAUSED");
  _fsm->addState("RUNNING");

  _fsm->addTransition("OPEN","CREATED","PAUSED",boost::bind(&lydaq::LMdccEcalServer::open, this,_1));
  _fsm->addTransition("RESUME","PAUSED","RUNNING",boost::bind(&lydaq::LMdccEcalServer::resume, this,_1));
  _fsm->addTransition("RESET","PAUSED","PAUSED",boost::bind(&lydaq::LMdccEcalServer::reset, this,_1));
  _fsm->addTransition("PAUSE","RUNNING","PAUSED",boost::bind(&lydaq::LMdccEcalServer::pause, this,_1));
  _fsm->addTransition("PAUSE","PAUSED","PAUSED",boost::bind(&lydaq::LMdccEcalServer::pause, this,_1));
  _fsm->addTransition("DESTROY","PAUSED","CREATED",boost::bind(&lydaq::LMdccEcalServer::close, this,_1));
  
  _fsm->addTransition("ECALPAUSE","PAUSED","PAUSED",boost::bind(&lydaq::LMdccEcalServer::ecalpause, this,_1));
  _fsm->addTransition("ECALPAUSE","RUNNING","RUNNING",boost::bind(&lydaq::LMdccEcalServer::ecalpause, this,_1));
  _fsm->addTransition("ECALRESUME","PAUSED","PAUSED",boost::bind(&lydaq::LMdccEcalServer::ecalresume, this,_1));
  _fsm->addTransition("ECALRESUME","RUNNING","RUNNING",boost::bind(&lydaq::LMdccEcalServer::ecalresume, this,_1));


 _fsm->addCommand("PAUSE",boost::bind(&lydaq::LMdccEcalServer::c_pause,this,_1,_2));
 _fsm->addCommand("RESUME",boost::bind(&lydaq::LMdccEcalServer::c_resume,this,_1,_2));
 _fsm->addCommand("RESET",boost::bind(&lydaq::LMdccEcalServer::c_reset,this,_1,_2));
 _fsm->addCommand("ECALPAUSE",boost::bind(&lydaq::LMdccEcalServer::c_ecalpause,this,_1,_2));
 _fsm->addCommand("ECALRESUME",boost::bind(&lydaq::LMdccEcalServer::c_ecalresume,this,_1,_2));
 _fsm->addCommand("WRITEREG",boost::bind(&lydaq::LMdccEcalServer::c_writereg,this,_1,_2));
 _fsm->addCommand("READREG",boost::bind(&lydaq::LMdccEcalServer::c_readreg,this,_1,_2));
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LMdccEcalServer::c_status,this,_1,_2));
 _fsm->addCommand("SPILLON",boost::bind(&lydaq::LMdccEcalServer::c_spillon,this,_1,_2));
 _fsm->addCommand("SPILLOFF",boost::bind(&lydaq::LMdccEcalServer::c_spilloff,this,_1,_2));

 _fsm->addCommand("SETREG",boost::bind(&lydaq::LMdccEcalServer::c_setregister,this,_1,_2));
 _fsm->addCommand("GETREG",boost::bind(&lydaq::LMdccEcalServer::c_getregister,this,_1,_2));

 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Service "<<name<<" is starting on "<<atoi(wp));
      //      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }


}


void lydaq::LMdccEcalServer::doOpen(std::string s)
{
  //  std::cout<<"calling open "<<std::endl;
  LOG4CXX_INFO(_logMDCC,__PRETTY_FUNCTION__<<" Opening "<<s);
  if (_mdcc!=NULL)
    delete _mdcc;
  _mdcc= new lydaq::MDCCReadout(s);
  _mdcc->open();
  //std::cout<<" Open Ptr "<<_mdcc<<std::endl;
}
