
#include "LZupServer.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LZupServer::open(zdaq::fsmmessage* m)
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
  
  
  
  _lv= new lydaq::Zup(device,port);
 

  
}
void lydaq::LZupServer::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_lv==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No ZUPInterface opened");
       return;
    }

  delete _lv;
  _lv=NULL;
}
void lydaq::LZupServer::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No ZUPInterface opened");
       return;
    }
    _running=false;
  g_store.join_all();
}
void lydaq::LZupServer::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No ZUPInterface opened");
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
  
  
  g_store.create_thread(boost::bind(&lydaq::LZupServer::monitor, this));
  _running=true;
    
}

void lydaq::LZupServer::monitor()
{
  Json::FastWriter fastWriter;
  while (_running)
  {
    
    if (_publisher==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No publisher defined");
      continue;
    }
    std::stringstream sheader;
    sheader<<"ZUPLV";
    zmq::message_t ma1((void*)sheader.str().c_str(), sheader.str().length(), NULL); 
    _publisher->send(ma1, ZMQ_SNDMORE); 
    Json::Value jstatus=this->status();
    std::string scont= fastWriter.write(jstatus);
    zmq::message_t ma2((void*)scont.c_str(), scont.length(), NULL); 
    _publisher->send(ma2);
    if (!_running) break;
    ::sleep(_period);
  }
}
Json::Value lydaq::LZupServer::status()
{
  Json::Value r=Json::Value::null;
  r["name"]="ZUP";
  r["status"]=Json::Value::null;
   if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No ZUPInterface opened");
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



void lydaq::LZupServer::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No ZUPInterface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
 
  response["STATUS"]=this->status();
}
void lydaq::LZupServer::c_on(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No ZUPInterface opened");
    response["STATUS"]=Json::Value::null;
    return;
  }
 
  _lv->ON();
  ::sleep(2);
  response["STATUS"]=this->status();
}
void lydaq::LZupServer::c_off(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_lv==NULL)
  {
    LOG4CXX_ERROR(_logLdaq,"No ZUPInterface opened");
    response["STATUS"]=Json::Value::null;
    return;
  }
 
  _lv->OFF();
  ::sleep(2);
  response["STATUS"]=this->status();
}



lydaq::LZupServer::LZupServer(std::string name) : zdaq::baseApplication(name),_lv(NULL),_running(false),_context(NULL),_publisher(NULL),_period(120)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();

  
// Register state
  _fsm->addState("STOPPED");
  _fsm->addState("RUNNING");

  _fsm->addTransition("OPEN","CREATED","STOPPED",boost::bind(&lydaq::LZupServer::open, this,_1));
  _fsm->addTransition("START","STOPPED","RUNNING",boost::bind(&lydaq::LZupServer::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","STOPPED",boost::bind(&lydaq::LZupServer::stop, this,_1));
  _fsm->addTransition("CLOSE","STOPPED","CREATED",boost::bind(&lydaq::LZupServer::close, this,_1));
  
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LZupServer::c_status,this,_1,_2));
 _fsm->addCommand("ON",boost::bind(&lydaq::LZupServer::c_on,this,_1,_2));
 _fsm->addCommand("OFF",boost::bind(&lydaq::LZupServer::c_off,this,_1,_2));
 

 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }

    if (this->parameters().isMember("serverName"))
    {
      _context= new zmq::context_t(1);
      _publisher= new zmq::socket_t((*_context), ZMQ_PUB);
      _publisher->bind(this->parameters()["serverName"].asString());
      
    }
}



