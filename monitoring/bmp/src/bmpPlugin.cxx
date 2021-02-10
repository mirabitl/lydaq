
#include "bmpPlugin.hh"
using namespace zdaq;
using namespace lydaq;
void lydaq::bmpPlugin::loadParameters(Json::Value params)
{
  if (params.isMember("bmp"))
    _params=params["bmp"];
}
void lydaq::bmpPlugin::registerCommands(zdaq::fsmweb* fsmw) 
{
 fsmw->addCommand("BMP_STATUS",boost::bind(&lydaq::bmpPlugin::c_status,this,_1,_2));
   
}
void lydaq::bmpPlugin::open(zdaq::fsmmessage* m)
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
void lydaq::bmpPlugin::close(zdaq::fsmmessage* m)
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
Json::Value lydaq::bmpPlugin::status()
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



void lydaq::bmpPlugin::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_bmp==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No BMPInterface opened");
       response["BMP_STATUS"]=Json::Value::null;
       return;
    }
 
  response["BMP_STATUS"]=this->status();
}



lydaq::bmpPlugin::bmpPlugin(): _bmp(NULL)
{}




