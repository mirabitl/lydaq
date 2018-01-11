
#include "LHih8000Server.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LHih8000Server::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  
  std::cout<<"calling open "<<std::endl;
  if (_hih8000!=NULL)
    delete _hih8000;
  
  
  
  _hih8000= new lydaq::hih8000();
 

  
}
void lydaq::LHih8000Server::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_hih8000==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No HIH8000Interface opened");
       return;
    }

  delete _hih8000;
  _hih8000=NULL;
}
Json::Value lydaq::LHih8000Server::status()
{
  Json::Value r=Json::Value::null;
  r["name"]=this->hardware();
  r["status"]=Json::Value::null;
   if (_hih8000==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HIH8000Interface opened");
       return r;
    }
   lock();
   _hih8000->Read();
   r["temperature0"]=_hih8000->temperature(0);
   r["temperature1"]=_hih8000->temperature(1);
   r["humidity0"]=_hih8000->humidity(0);
   r["humidity11"]=_hih8000->humidity(1);
   unlock();
   r["status"]="READ";
   return r;
}



void lydaq::LHih8000Server::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hih8000==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HIH8000Interface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
 
  response["STATUS"]=this->status();
}



lydaq::LHih8000Server::LHih8000Server(std::string name) : zdaq::monitorApplication(name),_hih8000(NULL)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LHih8000Server::c_status,this,_1,_2));

 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }
}



