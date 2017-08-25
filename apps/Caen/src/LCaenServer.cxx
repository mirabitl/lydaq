
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
void lydaq::LCaenServer::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
       return;
    }
    _running=false;
  g_store.join_all();
}
void lydaq::LCaenServer::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
       return;
    }
  if (m->content().isMember("period"))
    { 
      this->parameters()["period"]=m->content()["period"];
    }
  if (!this->parameters().isMember("period"))
    {
      LOG4CXX_ERROR(_logLdaq,"Please define Reading period");
      return;
    }
  else
    _period=this->parameters()["period"].asUInt();
 
  
   if (_publisher==NULL && this->parameters().isMember("serverName"))
    {
      _context= new zmq::context_t(1);
      _publisher= new zmq::socket_t((*_context), ZMQ_PUB);
      _publisher->bind(this->parameters()["serverName"].asString());
      
    }
   if (m->content().isMember("deviceName"))
    { 
      this->parameters()["deviceName"]=m->content()["deviceName"];
    }
   else
     if (!this->parameters().isMember("deviceName"))
	this->parameters()["deviceName"]="/TEST";	 	
  g_store.create_thread(boost::bind(&lydaq::LCaenServer::monitor, this));
  _running=true;
    
}

void lydaq::LCaenServer::monitor()
{
  Json::FastWriter fastWriter;
  std::stringstream sheader;
  sheader<<"CAENHV:"<<this->parameters()["deviceName"].asString();
  std::string head=sheader.str();
  while (_running)
  {
    
    if (_publisher==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No publisher defined");
      break;
    }
   
    
    zmq::message_t ma1((void*)head.c_str(), head.length(), NULL); 
    _publisher->send(ma1, ZMQ_SNDMORE); 
    Json::Value jstatus=this->status();
    std::string scont= fastWriter.write(jstatus);
    zmq::message_t ma2((void*)scont.c_str(), scont.length(), NULL); 
    _publisher->send(ma2);
    if (!_running) break;
    ::sleep(_period);
  }
  LOG4CXX_INFO(_logLdaq,"End of monitoring task");
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
{
  Json::Value r;
  r["name"]=this->parameters()["deviceName"].asString();
  
 
  Json::Value jsonArray;
  jsonArray.append(Json::Value::null);
  jsonArray.clear();
  r["channels"]=jsonArray;
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No HVCaenInterface opened");
    return r;
  }
  if (!this->parameters().isMember("first"))
  {
    LOG4CXX_ERROR(_logLdaq,"Please define first channel");
    return r;
  }
  if (!this->parameters().isMember("last"))
  {
    LOG4CXX_ERROR(_logLdaq,"Please define last channel");
    return r;
  }
  for (uint32_t i=this->parameters()["first"].asUInt();i<=this->parameters()["last"].asUInt();i++)
    r["channels"].append(this->channelStatus(i));
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
  if (first!=9999) this->parameters()["first"]=first;
  uint32_t last=atol(request.get("last","9999").c_str());
  std::cout<<last<<std::endl;
  if (last!=9999 ) this->parameters()["last"]=last;
  if (first==9999 || last==9999)
  {
    LOG4CXX_ERROR(_logLdaq,"First and last channels should be specified");
    response["STATUS"]=Json::Value::null;
    return;
  }
 
  response["STATUS"]=this->status();
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




lydaq::LCaenServer::LCaenServer(std::string name) : zdaq::baseApplication(name),_hv(NULL),_running(false),_context(NULL),_publisher(NULL),_period(120)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();

  
// Register state
  _fsm->addState("STOPPED");
  _fsm->addState("RUNNING");

  _fsm->addTransition("OPEN","CREATED","STOPPED",boost::bind(&lydaq::LCaenServer::open, this,_1));
  _fsm->addTransition("START","STOPPED","RUNNING",boost::bind(&lydaq::LCaenServer::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","STOPPED",boost::bind(&lydaq::LCaenServer::stop, this,_1));
  _fsm->addTransition("CLOSE","STOPPED","CREATED",boost::bind(&lydaq::LCaenServer::close, this,_1));
  
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LCaenServer::c_status,this,_1,_2));
 _fsm->addCommand("ON",boost::bind(&lydaq::LCaenServer::c_on,this,_1,_2));
 _fsm->addCommand("OFF",boost::bind(&lydaq::LCaenServer::c_off,this,_1,_2));
 _fsm->addCommand("VSET",boost::bind(&lydaq::LCaenServer::c_vset,this,_1,_2));
 _fsm->addCommand("ISET",boost::bind(&lydaq::LCaenServer::c_iset,this,_1,_2));
 _fsm->addCommand("RAMPUP",boost::bind(&lydaq::LCaenServer::c_rampup,this,_1,_2));
 

 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

   
}



