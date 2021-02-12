
#include "LBmpServer.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LBmpServer::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  
  std::cout<<"calling open "<<std::endl;
  if (_bmp!=NULL)
    delete _bmp;
  
  
#ifdef BMP183  
  _bmp= new lydaq::BMP183();
#else
  _bmp= new lydaq::BMP280();
#endif
  
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
Json::Value lydaq::LBmpServer::status()
{
  Json::Value r=Json::Value::null;
  r["name"]=this->hardware();
  r["status"]=Json::Value::null;
   if (_bmp==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No BMPInterface opened");
       return r;
    }
   lock();
#ifdef BMP183
   r["pressure"]=_bmp->BMP183PressionRead();
   r["temperature"]=_bmp->BMP183TemperatureRead();
#else
   float t,p;
   _bmp->TemperaturePressionRead(&t,&p);
   r["pressure"]=p;
   r["temperature"]=t;
#endif
   unlock();
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



lydaq::LBmpServer::LBmpServer(std::string name) : zdaq::monitorApplication(name),_bmp(NULL)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LBmpServer::c_status,this,_1,_2));

 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }
}



