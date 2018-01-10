
#include "LN1470Server.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LN1470Server::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  std::string device;
  uint32_t board;
  if (m->content().isMember("device"))
    { 
      device=m->content()["device"].asString();
      this->parameters()["device"]=m->content()["device"];
    }
  else
    device=this->parameters()["device"].asString();
  if (m->content().isMember("board"))
    { 
      board=m->content()["board"].asUInt();
      this->parameters()["board"]=m->content()["board"];
    }
  else
    board=this->parameters()["board"].asUInt();


  std::cout<<"calling open "<<std::endl;
  if (_hv!=NULL)
    delete _hv;
  
  
  
  _hv= new lydaq::N1470(device,board);


  if (m->content().isMember("first"))
    { 
      this->parameters()["first"]=m->content()["first"];
    }
  if (m->content().isMember("last"))
    { 
      this->parameters()["last"]=m->content()["last"];
    }
}
void lydaq::LN1470Server::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_hv==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No HVN1470Interface opened");
       return;
    }
  delete _hv;
  _hv=NULL;
}
Json::Value lydaq::LN1470Server::channelStatus(uint32_t channel)
{
  Json::Value r=Json::Value::null;
  r["id"]=channel;
  r["status"]=Json::Value::null;
   if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HVN1470Interface opened");
       return r;
    }
   _hv->status(channel);
   r["vset"]=_hv->voltageSet(channel);
   r["iset"]=_hv->currentSet(channel);
   r["rampup"]=_hv->rampUp(channel);
   r["iout"]=_hv->currentUsed(channel);
   r["vout"]=_hv->voltageUsed(channel);
   r["status"]=_hv->statusBits(channel);
   std::cout<<"channel "<<channel<<"=>"<<r<<std::endl;
   return r;
}
Json::Value lydaq::LN1470Server::status()
{ return status(0,3);}
Json::Value lydaq::LN1470Server::status(int32_t first,int32_t last)
{
  Json::Value r;
  r["name"]=this->hardware();
  
 
  Json::Value jsonArray;
  jsonArray.append(Json::Value::null);
  jsonArray.clear();
  r["channels"]=jsonArray;
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVN1470Interface opened");
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
      std::cout<<" Checking channel "<<i<<std::endl;
      Json::Value v=this->channelStatus(i);
      std::cout <<v<<std::endl;
    r["channels"].append(v);
    }
  // for (uint32_t i=this->parameters()["first"].asUInt();i<=this->parameters()["last"].asUInt();i++)
  //   r["channels"].append(this->channelStatus(i));
   unlock();
  return r;
}



void lydaq::LN1470Server::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HVN1470Interface opened");
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
void lydaq::LN1470Server::c_on(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVN1470Interface opened");
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
    _hv->setOn(i);
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LN1470Server::c_off(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
    _hv->setOff(i);
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LN1470Server::c_clearalarm(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVN1470 opened");
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

void lydaq::LN1470Server::c_vset(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
    _hv->setVoltage(i,vset);
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LN1470Server::c_iset(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVN1470Interface opened");
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
    _hv->setCurrent(i,iset);
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LN1470Server::c_rampup(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVN1470Interface opened");
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
    _hv->setRampUp(i,rup);
  ::sleep(2);
  response["STATUS"]=this->status();
}




lydaq::LN1470Server::LN1470Server(std::string name) : zdaq::monitorApplication(name),_hv(NULL)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();

  

  
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LN1470Server::c_status,this,_1,_2));
 _fsm->addCommand("ON",boost::bind(&lydaq::LN1470Server::c_on,this,_1,_2));
 _fsm->addCommand("OFF",boost::bind(&lydaq::LN1470Server::c_off,this,_1,_2));
 _fsm->addCommand("VSET",boost::bind(&lydaq::LN1470Server::c_vset,this,_1,_2));
 _fsm->addCommand("ISET",boost::bind(&lydaq::LN1470Server::c_iset,this,_1,_2));
 _fsm->addCommand("RAMPUP",boost::bind(&lydaq::LN1470Server::c_rampup,this,_1,_2));
 _fsm->addCommand("CLEARALARM",boost::bind(&lydaq::LN1470Server::c_clearalarm,this,_1,_2));


 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

   
}



