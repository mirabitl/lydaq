
#include "hihPlugin.hh"
using namespace zdaq;
using namespace lydaq;
void lydaq::hihPlugin::loadParameters(Json::Value params)
{
  if (params.isMember("hih"))
    _params=params["hih"];
}
void lydaq::hihPlugin::registerCommands(zdaq::fsmweb* fsmw) 
{
 fsmw->addCommand("HIH_STATUS",boost::bind(&lydaq::hihPlugin::c_status,this,_1,_2));
   
}
void lydaq::hihPlugin::open(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  
  std::cout<<"calling open "<<std::endl;
  if (_hih!=NULL)
    delete _hih;
  
  _hih= new lydaq::hih8000();
  
}
void lydaq::hihPlugin::close(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  if (_hih==NULL)
    {
       LOG4CXX_ERROR(_logLdaq,"No HIHInterface opened");
       return;
    }

  delete _hih;
  _hih=NULL;
}
Json::Value lydaq::hihPlugin::status()
{
    Json::Value r=Json::Value::null;
  r["name"]=this->hardware();
  r["status"]=Json::Value::null;
   if (_hih==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HIH8000Interface opened");
       return r;
    }

   _hih->Read();
   r["temperature0"]=_hih->temperature(0);
   r["temperature1"]=_hih->temperature(1);
   r["humidity0"]=_hih->humidity(0);
   r["humidity1"]=_hih->humidity(1);

   r["status"]="READ";
   return r;
}



void lydaq::hihPlugin::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hih==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,"No HIHInterface opened");
       response["HIH_STATUS"]=Json::Value::null;
       return;
    }
 
  response["HIH_STATUS"]=this->status();
}



lydaq::hihPlugin::hihPlugin(): _hih(NULL)
{}

extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  zdaq::zmonPlugin* loadPlugin(void)
    {
      return (new lydaq::hihPlugin);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deletePlugin(zdaq::zmonPlugin* obj)
    {
      delete obj;
    }
}




