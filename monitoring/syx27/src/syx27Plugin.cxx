
#include "syx27Plugin.hh"
using namespace zdaq;
using namespace lydaq;
lydaq::syx27Plugin::syx27Plugin(): _hv(NULL){} 
void lydaq::syx27Plugin::loadParameters(Json::Value params)
{
  if (params.isMember("syx27"))
    _params=params["syx27"];
}
void lydaq::syx27Plugin::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  std::string account;
  if (m->content().isMember("account"))
    { 
      account=m->content()["account"].asString();
      _params["account"]=m->content()["account"];
    }
  else
    account=_params["account"].asString();


  std::cout<<"calling open "<<std::endl;
  if (_hv!=NULL)
    delete _hv;
  
  int ipass = account.find("/");
  int ipath = account.find("@");
  std::string Name,Pwd,Host;
  Name.clear();
  Name=account.substr(0,ipass); 
  Pwd.clear();
  Pwd=account.substr(ipass+1,ipath-ipass-1); 
  Host.clear();
  Host=account.substr(ipath+1,account.size()-ipath); 
  std::cout<<Name<<"|"<<std::endl;
  std::cout<<Pwd<<"|"<<std::endl;
  std::cout<<Host<<"|"<<std::endl;
  
  
  _hv= new lydaq::HVCaenInterface(Host,Name,Pwd);

 

  if (m->content().isMember("first"))
    { 
      _params["first"]=m->content()["first"];
    }
  if (m->content().isMember("last"))
    { 
      _params["last"]=m->content()["last"];
    }
  _hv->Connect();
}
void lydaq::syx27Plugin::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_hv==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
       return;
    }
  _hv->Disconnect();
  delete _hv;
  _hv=NULL;
}
Json::Value lydaq::syx27Plugin::channelStatus(uint32_t channel)
{
   _hv->Connect();
  Json::Value r=Json::Value::null;
  r["id"]=channel;
  r["status"]=Json::Value::null;
   if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logCAEN,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
       return r;
    }
   r["name"]=_hv->GetName(channel);
   r["slot"]=_hv->BoardSlot(channel);
   r["channel"]=_hv->BoardChannel(channel);
   r["vset"]=_hv->GetVoltageSet(channel);
   r["iset"]=_hv->GetCurrentSet(channel);
   r["rampup"]=_hv->GetVoltageRampUp(channel);
   r["iout"]=_hv->GetCurrentRead(channel);
   r["vout"]=_hv->GetVoltageRead(channel);
   r["status"]=_hv->GetStatus(channel);
   return r;
}
Json::Value lydaq::syx27Plugin::status()
{ return status(-1,-1);}
Json::Value lydaq::syx27Plugin::status(int32_t first,int32_t last)
{
    Json::Value r;
  r["name"]=this->hardware();
  
 
  Json::Value jsonArray;
  jsonArray.append(Json::Value::null);
  jsonArray.clear();
  r["channels"]=jsonArray;
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logCAEN,__PRETTY_FUNCTION__<<"No HVCaenInterface opened");
    return r;
  }
  int32_t fi=0,la=0;
  if (!_params.isMember("first") && first<0)
  {
    LOG4CXX_ERROR(_logCAEN,__PRETTY_FUNCTION__<<"Please define first channel");
    return r;
  }
  if (first<0)
    fi=_params["first"].asUInt();
  else
    fi=first;
  if (!_params.isMember("last") && last<0)
  {
    LOG4CXX_ERROR(_logCAEN,__PRETTY_FUNCTION__<<"Please define last channel");
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
  // for (uint32_t i=_params["first"].asUInt();i<=_params["last"].asUInt();i++)
  //   r["channels"].append(this->channelStatus(i));

  return r;

}



void lydaq::syx27Plugin::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
       response["SY_STATUS"]=Json::Value::null;
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
    response["SY_STATUS"]=Json::Value::null;
    return;
  }
 
  response["SY_STATUS"]=this->status(first,last);
}
void lydaq::syx27Plugin::c_on(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    response["SY_STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && _params.isMember("first")) first=_params["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && _params.isMember("last")) last=_params["last"].asUInt();
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["SY_STATUS"]=Json::Value::null;
    return;
  }
  _hv->Connect();
  for (uint32_t i=first;i<=last;i++)
    _hv->SetOn(i);

  response["SY_STATUS"]=this->status(first,last);
}
void lydaq::syx27Plugin::c_off(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    response["SY_STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && _params.isMember("first")) first=_params["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && _params.isMember("last")) last=_params["last"].asUInt();
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["SY_STATUS"]=Json::Value::null;
    return;
  }
  _hv->Connect();
  for (uint32_t i=first;i<=last;i++)
    _hv->SetOff(i);


  response["SY_STATUS"]=this->status(first,last);
}
void lydaq::syx27Plugin::c_clearalarm(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    response["SY_STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && _params.isMember("first")) first=_params["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && _params.isMember("last")) last=_params["last"].asUInt();
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["SY_STATUS"]=Json::Value::null;
    return;
  }
  // Not implemented
  response["SY_STATUS"]=this->status(first,last);
}

void lydaq::syx27Plugin::c_vset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
    response["SY_STATUS"]="Please open MDC01 first";
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
    response["SY_STATUS"]=Json::Value::null;
    return;
  }
  _hv->Connect();
  for (uint32_t i=first;i<=last;i++)
    _hv->SetVoltage(i,vset);

  response["SY_STATUS"]=this->status(first,last);
}
void lydaq::syx27Plugin::c_iset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    response["SY_STATUS"]=Json::Value::null;
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
    response["SY_STATUS"]=Json::Value::null;
    return;
  }
  _hv->Connect();
  for (uint32_t i=first;i<=last;i++)
    _hv->SetCurrent(i,iset);
  response["SY_STATUS"]=this->status(first,last);
}
void lydaq::syx27Plugin::c_rampup(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    response["SY_STATUS"]=Json::Value::null;
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
    response["SY_STATUS"]=Json::Value::null;
    return;
  }

    _hv->Connect();
  for (uint32_t i=first;i<=last;i++)
    _hv->SetVoltageRampUp(i,rup);

  response["SY_STATUS"]=this->status(first,last);
}




void lydaq::syx27Plugin::registerCommands(zdaq::fsmweb* fsmw) 
{
 fsmw->addCommand("SY_STATUS",boost::bind(&lydaq::syx27Plugin::c_status,this,_1,_2));
 fsmw->addCommand("SY_ON",boost::bind(&lydaq::syx27Plugin::c_on,this,_1,_2));
 fsmw->addCommand("SY_OFF",boost::bind(&lydaq::syx27Plugin::c_off,this,_1,_2));
 fsmw->addCommand("SY_VSET",boost::bind(&lydaq::syx27Plugin::c_vset,this,_1,_2));
 fsmw->addCommand("SY_ISET",boost::bind(&lydaq::syx27Plugin::c_iset,this,_1,_2));
 fsmw->addCommand("SY_RAMPUP",boost::bind(&lydaq::syx27Plugin::c_rampup,this,_1,_2));
 fsmw->addCommand("SY_CLEARALARM",boost::bind(&lydaq::syx27Plugin::c_clearalarm,this,_1,_2));

   
}


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  zdaq::zmonPlugin* loadPlugin(void)
    {
      return (new lydaq::syx27Plugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deletePlugin(zdaq::zmonPlugin* obj)
    {
      delete obj;
    }
}

