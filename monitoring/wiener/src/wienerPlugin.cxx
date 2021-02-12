
#include "wienerPlugin.hh"
using namespace zdaq;
using namespace lydaq;
lydaq::wienerPlugin::wienerPlugin(): _hv(NULL){} 
void lydaq::wienerPlugin::loadParameters(Json::Value params)
{
  if (params.isMember("wiener"))
    _params=params["wiener"];
}
void lydaq::wienerPlugin::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  std::string address;
  if (m->content().isMember("address"))
    { 
      address=m->content()["address"].asString();
      _params["address"]=m->content()["address"];
    }
  else
    address=_params["address"].asString();


  std::cout<<"calling open "<<std::endl;
  if (_hv!=NULL)
    delete _hv;
  
  
  
  
  _hv= new lydaq::WienerSnmp(address);
  

  if (m->content().isMember("first"))
    { 
      _params["first"]=m->content()["first"];
    }
  if (m->content().isMember("last"))
    { 
      _params["last"]=m->content()["last"];
    }
}
void lydaq::wienerPlugin::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_hv==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No HVWienerInterface opened");
       return;
    }
  
  delete _hv;
  _hv=NULL;
}
Json::Value lydaq::wienerPlugin::channelStatus(uint32_t channel)
{
  Json::Value r=Json::Value::null;
  r["id"]=channel;
  r["status"]=Json::Value::null;
   if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
       return r;
    }
   // std::cout<<channel<<" gives "<<_hv->getOutputVoltage(channel/8,channel%8)<<std::endl;
   r["vset"]=_hv->getOutputVoltage(channel/8,channel%8);
   r["iset"]=_hv->getOutputCurrentLimit(channel/8,channel%8);
   r["rampup"]=_hv->getOutputVoltageRiseRate(channel/8,channel%8);
   r["iout"]=_hv->getOutputMeasurementCurrent(channel/8,channel%8);
   r["vout"]=_hv->getOutputMeasurementSenseVoltage(channel/8,channel%8);
   r["status"]=_hv->getOutputStatus(channel/8,channel%8);
   
   return r;
}
Json::Value lydaq::wienerPlugin::status()
{ return status(-1,-1);}
Json::Value lydaq::wienerPlugin::status(int32_t first,int32_t last)
{

  Json::Value r;
  r["name"]=this->hardware();
  Json::Value jsonArray;
  jsonArray.append(Json::Value::null);
  jsonArray.clear();
  r["channels"]=jsonArray;
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
    return r;
  }
  int32_t fi=0,la=0;
  if (!_params.isMember("first") && first<0)
  {
    LOG4CXX_ERROR(_logLdaq,"Please define first channel");
    return r;
  }
  if (first<0)
    fi=_params["first"].asUInt();
  else
    fi=first;
  if (!_params.isMember("last") && last<0)
  {
    LOG4CXX_ERROR(_logLdaq,"Please define last channel");
    return r;
  }
  if (last<0)
    la=_params["last"].asUInt();
  else
    la=last;

  for (uint32_t i=fi;i<=la;i++)
    {
      Json::Value v=this->channelStatus(i);
      //std::cout <<v<<std::endl;
    r["channels"].append(v);
    }

  return r;
}



void lydaq::wienerPlugin::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
       response["WP_STATUS"]=Json::Value::null;
       return;
    }
  int32_t first=atol(request.get("first","-1").c_str());
  std::cout<<first<<std::endl;
  //if (first!=9999) _params["first"]=first;
  int32_t last=atol(request.get("last","-1").c_str());
  std::cout<<last<<std::endl;
  //if (last!=9999 ) _params["last"]=last;
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
 
  response["WP_STATUS"]=this->status(first,last);
}
void lydaq::wienerPlugin::c_on(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && _params.isMember("first")) first=_params["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && _params.isMember("last")) last=_params["last"].asUInt();
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputSwitch(i/8,i%8,1);
  ::sleep(2);
  response["WP_STATUS"]=this->status(first,last);
}
void lydaq::wienerPlugin::c_off(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && _params.isMember("first")) first=_params["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && _params.isMember("last")) last=_params["last"].asUInt();
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputSwitch(i/8,i%8,0);
  
  ::sleep(2);
  response["WP_STATUS"]=this->status(first,last);
}
void lydaq::wienerPlugin::c_clearalarm(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && _params.isMember("first")) first=_params["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && _params.isMember("last")) last=_params["last"].asUInt();
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputSwitch(i/8,i%8,10);
  
  ::sleep(2);
  response["WP_STATUS"]=this->status(first,last);
}

void lydaq::wienerPlugin::c_vset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
    response["WP_STATUS"]="Please open MDC01 first";
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && _params.isMember("first")) first=_params["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && _params.isMember("last")) last=_params["last"].asUInt();
  float vset=atof(request.get("value","-1.0").c_str());
  if (first==9999 || last==9999 || vset<0)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels , and value should be specified");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputVoltage(i/8,i%8,vset);
  ::sleep(2);
  response["WP_STATUS"]=this->status(first,last);
}
void lydaq::wienerPlugin::c_iset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVWienerInterface opened");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && _params.isMember("first")) first=_params["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && _params.isMember("last")) last=_params["last"].asUInt();
  float iset=atof(request.get("value","-1.0").c_str());
  if (first==9999 || last==9999 || iset<0)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels , and value should be specified");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputCurrentLimit(i/8,i%8,iset);
  ::sleep(2);
  response["WP_STATUS"]=this->status(first,last);
}
void lydaq::wienerPlugin::c_rampup(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVWienerInterface opened");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && _params.isMember("first")) first=_params["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && _params.isMember("last")) last=_params["last"].asUInt();
  float rup=atof(request.get("value","-1.0").c_str());
  if (first==9999 || last==9999 || rup<0)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels , and value should be specified");
    response["WP_STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->setOutputVoltageRiseRate(i/8,i%8,-1.*rup);
  ::sleep(2);
  response["WP_STATUS"]=this->status(first,last);
}




void lydaq::wienerPlugin::registerCommands(zdaq::fsmweb* fsmw) 
{
 fsmw->addCommand("WP_STATUS",boost::bind(&lydaq::wienerPlugin::c_status,this,_1,_2));
 fsmw->addCommand("WP_ON",boost::bind(&lydaq::wienerPlugin::c_on,this,_1,_2));
 fsmw->addCommand("WP_OFF",boost::bind(&lydaq::wienerPlugin::c_off,this,_1,_2));
 fsmw->addCommand("WP_VSET",boost::bind(&lydaq::wienerPlugin::c_vset,this,_1,_2));
 fsmw->addCommand("WP_ISET",boost::bind(&lydaq::wienerPlugin::c_iset,this,_1,_2));
 fsmw->addCommand("WP_RAMPUP",boost::bind(&lydaq::wienerPlugin::c_rampup,this,_1,_2));
 fsmw->addCommand("WP_CLEARALARM",boost::bind(&lydaq::wienerPlugin::c_clearalarm,this,_1,_2));

   
}


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  zdaq::zmonPlugin* loadPlugin(void)
    {
      return (new lydaq::wienerPlugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deletePlugin(zdaq::zmonPlugin* obj)
    {
      delete obj;
    }
}

