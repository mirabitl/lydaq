
#include "LBmpServer.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LBmpServer::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  
  std::cout<<"calling open "<<std::endl;
  if (_bmp!=NULL)
    delete _bmp;
  
  
  
  _bmp= new lydaq::BMP183();
 

  
}
void lydaq::LBmpServer::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_bmp==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No BMPInterface opened");
       return;
    }

  delete _bmp;
  _bmp=NULL;
}
void lydaq::LBmpServer::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_bmp==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No BMPInterface opened");
       return;
    }
    _running=false;
  g_store.join_all();
}
void lydaq::LBmpServer::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_bmp==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No BMPInterface opened");
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
  g_store.create_thread(boost::bind(&lydaq::LBmpServer::monitor, this));
  _running=true;
    
}

void lydaq::LBmpServer::monitor()
{
  Json::FastWriter fastWriter;
 std::stringstream sheader;
  sheader<<"BMP:"<<this->parameters()["deviceName"].asString();
  std::string head=sheader.str();
  while (_running)
  {
    
    if (_publisher==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No publisher defined");
      continue;
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
}
Json::Value lydaq::LBmpServer::status()
{
  Json::Value r=Json::Value::null;
  r["name"]="BMP";
  r["status"]=Json::Value::null;
   if (_bmp==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No BMPInterface opened");
       return r;
    }
   r["pressure"]=_bmp->BMP183PressionRead();
   r["temperature"]=_bmp->BMP183TemperatureRead();

   r["status"]="READ";
   return r;
}



void lydaq::LBmpServer::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_bmp==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No BMPInterface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
 
  response["STATUS"]=this->status();
}



lydaq::LBmpServer::LBmpServer(std::string name) : zdaq::baseApplication(name),_bmp(NULL),_running(false),_context(NULL),_publisher(NULL),_period(120)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();

  
// Register state
  _fsm->addState("STOPPED");
  _fsm->addState("RUNNING");

  _fsm->addTransition("OPEN","CREATED","STOPPED",boost::bind(&lydaq::LBmpServer::open, this,_1));
  _fsm->addTransition("START","STOPPED","RUNNING",boost::bind(&lydaq::LBmpServer::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","STOPPED",boost::bind(&lydaq::LBmpServer::stop, this,_1));
  _fsm->addTransition("CLOSE","STOPPED","CREATED",boost::bind(&lydaq::LBmpServer::close, this,_1));
  
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LBmpServer::c_status,this,_1,_2));

 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }
}



