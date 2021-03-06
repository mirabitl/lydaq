
#include "LWienerServer.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LWienerServer::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  std::string address;
  if (m->content().isMember("address"))
    { 
      address=m->content()["address"].asString();
      this->parameters()["address"]=m->content()["address"];
    }
  else
    address=this->parameters()["address"].asString();


  std::cout<<"calling open "<<std::endl;
  if (_hv!=NULL)
    delete _hv;
  
  
  
  
  _hv= new lydaq::WienerSnmp(address);
  

  if (m->content().isMember("first"))
    { 
      this->parameters()["first"]=m->content()["first"];
    }
  if (m->content().isMember("last"))
    { 
      this->parameters()["last"]=m->content()["last"];
    }
}
void lydaq::LWienerServer::close(zdaq::fsmmessage* m)
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
Json::Value lydaq::LWienerServer::channelStatus(uint32_t channel)
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
Json::Value lydaq::LWienerServer::status()
{ return status(-1,-1);}
Json::Value lydaq::LWienerServer::status(int32_t first,int32_t last)
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
  unlock();
  return r;
}



void lydaq::LWienerServer::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
  int32_t first=atol(request.get("first","-1").c_str());
  std::cout<<first<<std::endl;
  //if (first!=9999) this->parameters()["first"]=first;
  int32_t last=atol(request.get("last","-1").c_str());
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
void lydaq::LWienerServer::c_on(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
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
    _hv->setOutputSwitch(i/8,i%8,1);
  ::sleep(2);
  response["STATUS"]=this->status(first,last);
}
void lydaq::LWienerServer::c_off(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
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
    _hv->setOutputSwitch(i/8,i%8,0);
  
  ::sleep(2);
  response["STATUS"]=this->status(first,last);
}
void lydaq::LWienerServer::c_clearalarm(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
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
    _hv->setOutputSwitch(i/8,i%8,10);
  
  ::sleep(2);
  response["STATUS"]=this->status(first,last);
}

void lydaq::LWienerServer::c_vset(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
    _hv->setOutputVoltage(i/8,i%8,vset);
  ::sleep(2);
  response["STATUS"]=this->status(first,last);
}
void lydaq::LWienerServer::c_iset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVWienerInterface opened");
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
    _hv->setOutputCurrentLimit(i/8,i%8,iset);
  ::sleep(2);
  response["STATUS"]=this->status(first,last);
}
void lydaq::LWienerServer::c_rampup(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVWienerInterface opened");
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
    _hv->setOutputVoltageRiseRate(i/8,i%8,-1.*rup);
  ::sleep(2);
  response["STATUS"]=this->status(first,last);
}




lydaq::LWienerServer::LWienerServer(std::string name) : zdaq::monitorApplication(name),_hv(NULL)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();
  

 _fsm->addCommand("STATUS",boost::bind(&lydaq::LWienerServer::c_status,this,_1,_2));
 _fsm->addCommand("ON",boost::bind(&lydaq::LWienerServer::c_on,this,_1,_2));
 _fsm->addCommand("OFF",boost::bind(&lydaq::LWienerServer::c_off,this,_1,_2));
 _fsm->addCommand("VSET",boost::bind(&lydaq::LWienerServer::c_vset,this,_1,_2));
 _fsm->addCommand("ISET",boost::bind(&lydaq::LWienerServer::c_iset,this,_1,_2));
 _fsm->addCommand("RAMPUP",boost::bind(&lydaq::LWienerServer::c_rampup,this,_1,_2));
 _fsm->addCommand("CLEARALARM",boost::bind(&lydaq::LWienerServer::c_clearalarm,this,_1,_2));


 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

   
}



