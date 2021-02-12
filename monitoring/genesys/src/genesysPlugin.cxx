
#include "genesysPlugin.hh"
using namespace zdaq;
using namespace lydaq;
lydaq::genesysPlugin::genesysPlugin(): _lv(NULL){} 
void lydaq::genesysPlugin::loadParameters(Json::Value params)
{
  if (params.isMember("genesys"))
    _params=params["genesys"];
}
void lydaq::genesysPlugin::open(zdaq::fsmmessage* m)
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
  
  
  
  _lv= new lydaq::Genesys(device,port);
 
  _lv->INFO();

}
void lydaq::genesysPlugin::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_lv==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No HVGenesysInterface opened");
       return;
    }
  
  delete _lv;
  _lv=NULL;
}

Json::Value lydaq::genesysPlugin::status()
{
  Json::Value r=Json::Value::null;
  r["name"]=this->hardware();
  r["status"]="UNKOWN";
   if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No Genesys Interface opened");
       return r;
    }

   float vset=_lv->ReadVoltageSet();
   float vout=_lv->ReadVoltageUsed();
   r["vset"]=vset;
   r["vout"]=vout;
   r["iout"]=_lv->ReadCurrentUsed();
   if (abs(1-abs(vset-vout)/vset)<0.8)
     r["status"]="OFF";
   else
     r["status"]="ON";

   return r;
}


void lydaq::genesysPlugin::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No Genesys opened");
       response["GS_STATUS"]=Json::Value::null;
       return;
    }
  response["GS_STATUS"]=this->status();
}
void lydaq::genesysPlugin::c_on(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No Genesys opened");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
 _lv->ON();
  ::sleep(1);

  response["WP_STATUS"]=this->status();
}

void lydaq::genesysPlugin::c_off(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
 if (_lv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No Genesys opened");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
 _lv->OFF();
  ::sleep(1);

  response["WP_STATUS"]=this->status();
}

void lydaq::genesysPlugin::c_setdevice(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
 
  uint32_t device = atol(request.get("device", "999").c_str());
  LOG4CXX_INFO(_logLdaq, " Genesys device set to" << device);
  uint32_t address = atol(request.get("address", "999").c_str());
  LOG4CXX_INFO(_logLdaq, " Genesys address set to" << address);

  if (device==999 || address==999)
    {
    LOG4CXX_ERROR(_logLdaq,"Genesys Invalid device or address");
    response["STATUS"]=Json::Value::null;
    return;
    }
  std::stringstream sdev("");
  sdev<<"/dev/ttyUSB"<<device;
  _params["device"]=sdev.str();
  _params["port"]=address;
  
  response["STATUS"]="DONE";
}


void lydaq::genesysPlugin::registerCommands(zdaq::fsmweb* fsmw) 
{
 fsmw->addCommand("GS_STATUS",boost::bind(&lydaq::genesysPlugin::c_status,this,_1,_2));
 fsmw->addCommand("GS_ON",boost::bind(&lydaq::genesysPlugin::c_on,this,_1,_2));
 fsmw->addCommand("GS_OFF",boost::bind(&lydaq::genesysPlugin::c_off,this,_1,_2));
 fsmw->addCommand("GS_DEVICE",boost::bind(&lydaq::genesysPlugin::c_setdevice,this,_1,_2));

   
}


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  zdaq::zmonPlugin* loadPlugin(void)
    {
      return (new lydaq::genesysPlugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deletePlugin(zdaq::zmonPlugin* obj)
    {
      delete obj;
    }
}

