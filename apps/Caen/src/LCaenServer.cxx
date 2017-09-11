
#include "LCaenServer.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LCaenServer::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  std::string account;
  if (m->content().isMember("account"))
    { 
      account=m->content()["account"].asString();
      this->parameters()["account"]=m->content()["account"];
    }
  else
    account=this->parameters()["account"].asString();


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
  std::cout<<Name<<std::endl;
  std::cout<<Pwd<<std::endl;
  std::cout<<Host<<std::endl;
  
  
  _hv= new lydaq::HVCaenInterface(Host,Name,Pwd);
  _hv->Connect();

  if (m->content().isMember("first"))
    { 
      this->parameters()["first"]=m->content()["first"];
    }
  if (m->content().isMember("last"))
    { 
      this->parameters()["last"]=m->content()["last"];
    }
}
void lydaq::LCaenServer::close(zdaq::fsmmessage* m)
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
Json::Value lydaq::LCaenServer::channelStatus(uint32_t channel)
{
  Json::Value r=Json::Value::null;
  r["id"]=channel;
  r["status"]=Json::Value::null;
   if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
       return r;
    }
   r["vset"]=_hv->GetVoltageSet(channel);
   r["iset"]=_hv->GetCurrentSet(channel);
   r["rampup"]=_hv->GetVoltageRampUp(channel);
   r["iout"]=_hv->GetCurrentRead(channel);
   r["vout"]=_hv->GetVoltageRead(channel);
   r["status"]=_hv->GetStatus(channel);
   return r;
}
Json::Value lydaq::LCaenServer::status()
{ return status(-1,-1);}
Json::Value lydaq::LCaenServer::status(int32_t first,int32_t last)
{
  Json::Value r;
  r["name"]=this->hardware();
  
 
  Json::Value jsonArray;
  jsonArray.append(Json::Value::null);
  jsonArray.clear();
  r["channels"]=jsonArray;
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    return r;
  }
  int32_t fi=0,la=0;
  if (!this->parameters().isMember("first") && first<0)
  {
    LOG4CXX_ERROR(_logLdaq,"Please define first channel");
    return r;
  }
  if (first<0)
    fi=this->parameters()["first"].asUInt();
  else
    fi=first;
  if (!this->parameters().isMember("last") && last<0)
  {
    LOG4CXX_ERROR(_logLdaq,"Please define last channel");
    return r;
  }
 if (last<0)
    la=this->parameters()["last"].asUInt();
  else
    la=last;
  lock();
   for (uint32_t i=fi;i<=la;i++)
    {
      Json::Value v=this->channelStatus(i);
      //std::cout <<v<<std::endl;
    r["channels"].append(v);
    }
  // for (uint32_t i=this->parameters()["first"].asUInt();i<=this->parameters()["last"].asUInt();i++)
  //   r["channels"].append(this->channelStatus(i));
   unlock();
  return r;
}



void lydaq::LCaenServer::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
   uint32_t first=atol(request.get("first","9999").c_str());
  std::cout<<first<<std::endl;
  //if (first!=9999) this->parameters()["first"]=first;
  uint32_t last=atol(request.get("last","9999").c_str());
  std::cout<<last<<std::endl;
  //if (last!=9999 ) this->parameters()["last"]=last;
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["STATUS"]=Json::Value::null;
    return;
  }
 
  response["STATUS"]=this->status(first,last);
}
void lydaq::LCaenServer::c_on(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    response["STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && this->parameters().isMember("first")) first=this->parameters()["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && this->parameters().isMember("last")) last=this->parameters()["last"].asUInt();
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->SetOn(i);
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LCaenServer::c_off(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
    response["STATUS"]="Please open MDC01 first";
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && this->parameters().isMember("first")) first=this->parameters()["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && this->parameters().isMember("last")) last=this->parameters()["last"].asUInt();
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->SetOff(i);
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LCaenServer::c_clearalarm(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaen opened");
    response["STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && this->parameters().isMember("first")) first=this->parameters()["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && this->parameters().isMember("last")) last=this->parameters()["last"].asUInt();
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["STATUS"]=Json::Value::null;
    return;
  }
  // for (uint32_t i=first;i<=last;i++)
  //   _hv->setOutputSwitch(i/8,i%8,10);
  
  // ::sleep(2);
  response["STATUS"]=this->status(first,last);
}

void lydaq::LCaenServer::c_vset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"Please open MDC01 first");
    response["STATUS"]="Please open MDC01 first";
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && this->parameters().isMember("first")) first=this->parameters()["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && this->parameters().isMember("last")) last=this->parameters()["last"].asUInt();
  float vset=atof(request.get("value","-1.0").c_str());
  if (first==9999 || last==9999 || vset<0)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels , and value should be specified");
    response["STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->SetVoltage(i,vset);
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LCaenServer::c_iset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    response["STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && this->parameters().isMember("first")) first=this->parameters()["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && this->parameters().isMember("last")) last=this->parameters()["last"].asUInt();
  float iset=atof(request.get("value","-1.0").c_str());
  if (first==9999 || last==9999 || iset<0)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels , and value should be specified");
    response["STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->SetCurrent(i,iset);
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LCaenServer::c_rampup(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    response["STATUS"]=Json::Value::null;
    return;
  }
  uint32_t first=atol(request.get("first","9999").c_str());
  if (first==9999 && this->parameters().isMember("first")) first=this->parameters()["first"].asUInt();
  uint32_t last=atol(request.get("last","9999").c_str());
  if (last==9999 && this->parameters().isMember("last")) last=this->parameters()["last"].asUInt();
  float rup=atof(request.get("value","-1.0").c_str());
  if (first==9999 || last==9999 || rup<0)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels , and value should be specified");
    response["STATUS"]=Json::Value::null;
    return;
  }
  for (uint32_t i=first;i<=last;i++)
    _hv->SetVoltageRampUp(i,rup);
  ::sleep(2);
  response["STATUS"]=this->status();
}




lydaq::LCaenServer::LCaenServer(std::string name) : zdaq::monitorApplication(name),_hv(NULL)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();

  

  
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LCaenServer::c_status,this,_1,_2));
 _fsm->addCommand("ON",boost::bind(&lydaq::LCaenServer::c_on,this,_1,_2));
 _fsm->addCommand("OFF",boost::bind(&lydaq::LCaenServer::c_off,this,_1,_2));
 _fsm->addCommand("VSET",boost::bind(&lydaq::LCaenServer::c_vset,this,_1,_2));
 _fsm->addCommand("ISET",boost::bind(&lydaq::LCaenServer::c_iset,this,_1,_2));
 _fsm->addCommand("RAMPUP",boost::bind(&lydaq::LCaenServer::c_rampup,this,_1,_2));
 _fsm->addCommand("CLEARALARM",boost::bind(&lydaq::LCaenServer::c_clearalarm,this,_1,_2));


 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

   
}



