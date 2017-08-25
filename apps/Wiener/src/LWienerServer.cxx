
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
void lydaq::LWienerServer::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HVWienerInterface opened");
       return;
    }
    _running=false;
  g_store.join_all();
}
void lydaq::LWienerServer::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_hv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HVWienerInterface opened");
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
  
   if (this->parameters().isMember("serverName"))
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
  g_store.create_thread(boost::bind(&lydaq::LWienerServer::monitor, this));
  _running=true;
    
}

void lydaq::LWienerServer::monitor()
{
  Json::FastWriter fastWriter;
  std::stringstream sheader;
  sheader<<"WIENERHV:"<<this->parameters()["deviceName"].asString();
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
   r["iout"]=_hv->getOutputVoltageRiseRate(channel/8,channel%8);
   r["vout"]=_hv->getOutputMeasurementSenseVoltage(channel/8,channel%8);
   r["status"]=_hv->getOutputStatus(channel/8,channel%8);
   
   return r;
}
Json::Value lydaq::LWienerServer::status()
{
  Json::Value r;
  r["name"]=this->parameters()["deviceName"].asString();
  Json::Value jsonArray;
  jsonArray.append(Json::Value::null);
  jsonArray.clear();
  r["channels"]=jsonArray;
  if (_hv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No WienerSnmp opened");
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
    {
      Json::Value v=this->channelStatus(i);
      //std::cout <<v<<std::endl;
    r["channels"].append(v);
    }
  
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
  response["STATUS"]=this->status();
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
  response["STATUS"]=this->status();
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
  response["STATUS"]=this->status();
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
  response["STATUS"]=this->status();
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
  response["STATUS"]=this->status();
}




lydaq::LWienerServer::LWienerServer(std::string name) : zdaq::baseApplication(name),_hv(NULL),_running(false),_context(NULL),_publisher(NULL),_period(120)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();

  
// Register state
  _fsm->addState("STOPPED");
  _fsm->addState("RUNNING");

  _fsm->addTransition("OPEN","CREATED","STOPPED",boost::bind(&lydaq::LWienerServer::open, this,_1));
  _fsm->addTransition("START","STOPPED","RUNNING",boost::bind(&lydaq::LWienerServer::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","STOPPED",boost::bind(&lydaq::LWienerServer::stop, this,_1));
  _fsm->addTransition("CLOSE","STOPPED","CREATED",boost::bind(&lydaq::LWienerServer::close, this,_1));
  
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LWienerServer::c_status,this,_1,_2));
 _fsm->addCommand("ON",boost::bind(&lydaq::LWienerServer::c_on,this,_1,_2));
 _fsm->addCommand("OFF",boost::bind(&lydaq::LWienerServer::c_off,this,_1,_2));
 _fsm->addCommand("VSET",boost::bind(&lydaq::LWienerServer::c_vset,this,_1,_2));
 _fsm->addCommand("ISET",boost::bind(&lydaq::LWienerServer::c_iset,this,_1,_2));
 _fsm->addCommand("RAMPUP",boost::bind(&lydaq::LWienerServer::c_rampup,this,_1,_2));
 

 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

   
}



