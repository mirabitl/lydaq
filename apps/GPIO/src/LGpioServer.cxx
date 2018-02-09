
#include "LGpioServer.hh"
using namespace zdaq;
using namespace lydaq;

void lydaq::LGPIOServer::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  
  std::cout<<"calling open "<<std::endl;
  if (_GPIO!=NULL)
    delete _GPIO;
  
  
  
  _GPIO= new lydaq::GPIO();
 

  
}
void lydaq::LGPIOServer::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_GPIO==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No GPIOInterface opened");
       return;
    }

  delete _GPIO;
  _GPIO=NULL;
}
Json::Value lydaq::LGPIOServer::status()
{
  Json::Value r=Json::Value::null;
  r["name"]=this->hardware();
  r["status"]=Json::Value::null;
   if (_GPIO==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No GPIOInterface opened");
       return r;
    }
   lock();
   r["dif"]=_GPIO->getDIFPower();
   r["vme"]=_GPIO->getVMEPower();
   if (_GPIO->getDIFPower()==1)
     {
       r["vset"]=5;
   r["vout"]=5;
   r["iout"]=1.11;
     r["status"]="ON";
     }
   else
     {
       r["vset"]=5;
       r["vout"]=0;
       r["iout"]=0;
	 r["status"]="OFF";
     }

   unlock();
   r["status"]="READ";
   return r;
}



void lydaq::LGPIOServer::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_GPIO==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No GPIOInterface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
 
  response["STATUS"]=this->status();
}
void lydaq::LGPIOServer::c_vmeon(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_GPIO==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No GPIOInterface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
  _GPIO->VMEON();
  response["STATUS"]=this->status();
}

void lydaq::LGPIOServer::c_vmeoff(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_GPIO==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No GPIOInterface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
  _GPIO->VMEOFF();
  response["STATUS"]=this->status();
}
void lydaq::LGPIOServer::c_difon(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_GPIO==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No GPIOInterface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
  _GPIO->DIFON();
  response["STATUS"]=this->status();
}

void lydaq::LGPIOServer::c_difoff(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_GPIO==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No GPIOInterface opened");
       response["STATUS"]=Json::Value::null;
       return;
    }
  _GPIO->DIFOFF();
  response["STATUS"]=this->status();
}



lydaq::LGPIOServer::LGPIOServer(std::string name) : zdaq::monitorApplication(name),_GPIO(NULL)
{

  
 

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();
 _fsm->addCommand("STATUS",boost::bind(&lydaq::LGPIOServer::c_status,this,_1,_2));
 _fsm->addCommand("VMEON",boost::bind(&lydaq::LGPIOServer::c_vmeon,this,_1,_2));
 _fsm->addCommand("VMEOFF",boost::bind(&lydaq::LGPIOServer::c_vmeoff,this,_1,_2));

 _fsm->addCommand("DIFON",boost::bind(&lydaq::LGPIOServer::c_difon,this,_1,_2));
 _fsm->addCommand("DIFOFF",boost::bind(&lydaq::LGPIOServer::c_difoff,this,_1,_2));


 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }
}



