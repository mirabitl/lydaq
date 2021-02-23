
#include "zupPlugin.hh"
using namespace zdaq;
using namespace lydaq;
lydaq::zupPlugin::zupPlugin(): _lv(NULL){} 
void lydaq::zupPlugin::loadParameters(Json::Value params)
{
  if (params.isMember("zup"))
    _params=params["zup"];
}
void lydaq::zupPlugin::open(zdaq::fsmmessage* m)
{

   LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  std::string device;
  if (m->content().isMember("device"))
    { 
      device=m->content()["device"].asString();
      _params["device"]=m->content()["device"];
    }
  else
    device=_params["device"].asString();

  uint32_t port;
  if (m->content().isMember("port"))
  { 
    port=m->content()["port"].asUInt();
    _params["port"]=m->content()["port"];
  }
  else
    port=_params["port"].asUInt();
  std::cout<<"calling open "<<std::endl;
  if (_lv!=NULL)
    delete _lv;
  
  
  
  _lv= new lydaq::Zup(device,port);
 
  _lv->INFO();

}
void lydaq::zupPlugin::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_lv==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No HVZupInterface opened");
       return;
    }
  
  delete _lv;
  _lv=NULL;
}

Json::Value lydaq::zupPlugin::status()
{
  Json::Value r=Json::Value::null;
  r["name"]=this->hardware();
  r["status"]="UNKNOWN";
   if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No Zup Interface opened");
       return r;
    }
   Json::Value sr=_lv->Status();

   r["vset"]=sr["vset"];
   r["iset"]=sr["iset"];
   r["vout"]=sr["vout"];
   r["iout"]=sr["iout"];
   r["pwrstatus"]=sr["status"];
   int pws=sr["status"].asUInt();
   if ((pws>>4)&1)
     r["status"]="ON";
   else
     r["status"]="OFF";

   return r;
}


void lydaq::zupPlugin::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No Zup opened");
       response["ZUP_STATUS"]=Json::Value::null;
       return;
    }
  response["ZUP_STATUS"]=this->status();
}
void lydaq::zupPlugin::c_on(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No Zup opened");
    response["ZUP_STATUS"]=Json::Value::null;
    return;
  }
 _lv->ON();
  ::usleep(50000);

  response["ZUP_STATUS"]=this->status();
}

void lydaq::zupPlugin::c_off(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
 if (_lv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No Zup opened");
    response["ZUP_STATUS"]=Json::Value::null;
    return;
  }
 _lv->OFF();
  ::usleep(50000);

  response["ZUP_STATUS"]=this->status();
}

void lydaq::zupPlugin::c_setdevice(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
 
  uint32_t device = atol(request.get("device", "999").c_str());
  LOG4CXX_INFO(_logLdaq, " Zup device set to" << device);
  uint32_t address = atol(request.get("address", "999").c_str());
  LOG4CXX_INFO(_logLdaq, " Zup address set to" << address);

  if (device==999 || address==999)
    {
    LOG4CXX_ERROR(_logLdaq,"Zup Invalid device or address");
    response["STATUS"]=Json::Value::null;
    return;
    }
  std::stringstream sdev("");
  sdev<<"/dev/ttyUSB"<<device;
  _params["device"]=sdev.str();
  _params["port"]=address;
  
  response["STATUS"]="DONE";
}


void lydaq::zupPlugin::registerCommands(zdaq::fsmweb* fsmw) 
{
 fsmw->addCommand("ZUP_STATUS",boost::bind(&lydaq::zupPlugin::c_status,this,_1,_2));
 fsmw->addCommand("ZUP_ON",boost::bind(&lydaq::zupPlugin::c_on,this,_1,_2));
 fsmw->addCommand("ZUP_OFF",boost::bind(&lydaq::zupPlugin::c_off,this,_1,_2));
 fsmw->addCommand("ZUP_DEVICE",boost::bind(&lydaq::zupPlugin::c_setdevice,this,_1,_2));

   
}


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  zdaq::zmonPlugin* loadPlugin(void)
    {
      return (new lydaq::zupPlugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deletePlugin(zdaq::zmonPlugin* obj)
    {
      delete obj;
    }
}

