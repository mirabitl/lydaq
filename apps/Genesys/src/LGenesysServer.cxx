
#include "LGenesysServer.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LGenesysServer::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  std::string device;
  if (m->content().isMember("device"))
    { 
      device=m->content()["device"].asString();
      this->parameters()["device"]=m->content()["device"];
    }
  else
    device=this->parameters()["device"].asString();

  uint32_t port;
  if (m->content().isMember("port"))
  { 
    port=m->content()["port"].asUInt();
    this->parameters()["port"]=m->content()["port"];
  }
  else
    port=this->parameters()["port"].asUInt();
  std::cout<<"calling open "<<std::endl;
  if (_lv!=NULL)
    delete _lv;
  
  
  
  _lv= new lydaq::Genesys(device,port);
 
  _lv->INFO();

  
}
void lydaq::LGenesysServer::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_lv==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No Genesys Interface opened");
       return;
    }

  delete _lv;
  _lv=NULL;
}
Json::Value lydaq::LGenesysServer::status()
{
  Json::Value r=Json::Value::null;
  r["name"]=this->hardware();
  r["status"]="UNKOWN";
   if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No Genesys Interface opened");
       return r;
    }
   lock();
   float vset=_lv->ReadVoltageSet();
   float vout=_lv->ReadVoltageUsed();
   r["vset"]=vset;
   r["vout"]=vout;
   r["iout"]=_lv->ReadCurrentUsed();
   if (abs(1-abs(vset-vout)/vset)<0.8)
     r["status"]="OFF";
   else
     r["status"]="ON";
   unlock();
   return r;
}



void lydaq::LGenesysServer::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No Genesys Interface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
 
  response["STATUS"]=this->status();
}
void lydaq::LGenesysServer::c_on(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No  Genesys Interface opened");
    response["STATUS"]=Json::Value::null;
    return;
  }
 
  _lv->ON();
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LGenesysServer::c_off(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No Genesys Interface opened");
    response["STATUS"]=Json::Value::null;
    return;
  }
 
  _lv->OFF();
  ::sleep(2);
  response["STATUS"]=this->status();
}



lydaq::LGenesysServer::LGenesysServer(std::string name) : zdaq::monitorApplication(name),_lv(NULL)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();

  
  
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LGenesysServer::c_status,this,_1,_2));
 _fsm->addCommand("ON",boost::bind(&lydaq::LGenesysServer::c_on,this,_1,_2));
 _fsm->addCommand("OFF",boost::bind(&lydaq::LGenesysServer::c_off,this,_1,_2));
 

 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

   
}



