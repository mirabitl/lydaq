

#include "FullSlow.hh"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>

#include <string.h>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
#include "ReadoutLogger.hh"
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

using namespace lydaq;
using namespace zdaq;

FullSlow::FullSlow(std::string name) : zdaq::baseApplication(name)
{
  _caenClient=0;_zupClient=0;_genesysClient=0;_bmpClient=0;_gpioClient=0;_hihClient=0;
  _isegClient=0;_monitorClient=0;
 
  _fsm=this->fsm();
 
  _fsm->addState("DISCOVERED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("MONITORING");
 
 
  _fsm->addTransition("DISCOVER","CREATED","DISCOVERED",boost::bind(&FullSlow::discover, this,_1));
  
  _fsm->addTransition("DESTROY","DISCOVERED","CREATED",boost::bind(&FullSlow::destroy, this,_1));
  _fsm->addTransition("CONFIGURE","DISCOVERED","CONFIGURED",boost::bind(&FullSlow::configure, this,_1));
  _fsm->addTransition("START","CONFIGURED","MONITORING",boost::bind(&FullSlow::start, this,_1));
  _fsm->addTransition("STOP","MONITORING","CONFIGURED",boost::bind(&FullSlow::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&FullSlow::destroy, this,_1));

  // Commands
  _fsm->addCommand("LVSTATUS",boost::bind(&FullSlow::LVStatus,this,_1,_2));
  _fsm->addCommand("PTSTATUS",boost::bind(&FullSlow::PTStatus,this,_1,_2));
  _fsm->addCommand("HUMSTATUS",boost::bind(&FullSlow::HumidityStatus,this,_1,_2));
  _fsm->addCommand("LVON",boost::bind(&FullSlow::LVON,this,_1,_2));
  _fsm->addCommand("LVOFF",boost::bind(&FullSlow::LVOFF,this,_1,_2));

  _fsm->addCommand("HVSTATUS",boost::bind(&FullSlow::HVStatus,this,_1,_2));
  _fsm->addCommand("HVON",boost::bind(&FullSlow::HVON,this,_1,_2));
  _fsm->addCommand("CLEARALARM",boost::bind(&FullSlow::CLEARALARM,this,_1,_2));
  _fsm->addCommand("HVOFF",boost::bind(&FullSlow::HVOFF,this,_1,_2));
  _fsm->addCommand("VSET",boost::bind(&FullSlow::setVoltage,this,_1,_2));
  _fsm->addCommand("ISET",boost::bind(&FullSlow::setCurrentLimit,this,_1,_2));
  _fsm->addCommand("RAMPUP",boost::bind(&FullSlow::setRampUp,this,_1,_2));

 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" Service "<<name<<" started on port "<<atoi(wp));
      _fsm->start(atoi(wp));
    }

  _jConfigContent=Json::Value::null;
  


  
}

void  FullSlow::userCreate(zdaq::fsmmessage* m)
{
  // Stored the configuration file used
  if (m->content().isMember("url"))
    {
      _jConfigContent["url"]=m->content()["url"];
      if (m->content().isMember("login"))
	_jConfigContent["login"]=m->content()["login"];
    }
    
  else
    if (m->content().isMember("file"))
      _jConfigContent["file"]=m->content()["file"];
    else
      if (m->content().isMember("mongo"))
	_jConfigContent["mongo"]=m->content()["mongo"];
}

void FullSlow::destroy(zdaq::fsmmessage* m)
{
  _caenClient=0;_zupClient=0;_genesysClient=0;_bmpClient=0;_gpioClient=0;_hihClient=0;_monitorClient=0;
 
}
void FullSlow::configure(zdaq::fsmmessage* m)
{
  if (_caenClient!=0) _caenClient->sendTransition("OPEN");
  if (_zupClient!=0) _zupClient->sendTransition("OPEN");
  if (_genesysClient!=0) _genesysClient->sendTransition("OPEN");
  if (_isegClient!=0) _isegClient->sendTransition("OPEN");
  if (_bmpClient!=0) _bmpClient->sendTransition("OPEN");
  if (_hihClient!=0) _hihClient->sendTransition("OPEN");
  if (_monitorClient!=0) _monitorClient->sendTransition("INITIALISE");
 
}
void FullSlow::start(zdaq::fsmmessage* m)
{
  if (_caenClient!=0) _caenClient->sendTransition("START");
  if (_zupClient!=0) _zupClient->sendTransition("START");
  if (_genesysClient!=0) _genesysClient->sendTransition("START");
  if (_isegClient!=0) _isegClient->sendTransition("START");
  if (_bmpClient!=0) _bmpClient->sendTransition("START");
  if (_hihClient!=0) _hihClient->sendTransition("START");
  if (_monitorClient!=0) _monitorClient->sendTransition("START");
 
}
void FullSlow::stop(zdaq::fsmmessage* m)
{
  if (_caenClient!=0) _caenClient->sendTransition("STOP");
  if (_zupClient!=0) _zupClient->sendTransition("STOP");
  if (_genesysClient!=0) _genesysClient->sendTransition("STOP");
  if (_isegClient!=0) _isegClient->sendTransition("STOP");
  if (_bmpClient!=0) _bmpClient->sendTransition("STOP");
  if (_hihClient!=0) _hihClient->sendTransition("STOP");
  if (_monitorClient!=0) _monitorClient->sendTransition("STOP");
}
void FullSlow::discover(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" Discovering devices");
  Json::Value cjs=this->configuration()["HOSTS"];
  //  std::cout<<cjs<<std::endl;
  std::vector<std::string> lhosts=this->configuration()["HOSTS"].getMemberNames();
  // Loop on hosts
  for (auto host:lhosts)
    {
      //std::cout<<" Host "<<host<<" found"<<std::endl;
      // Loop on processes
      const Json::Value cjsources=this->configuration()["HOSTS"][host];
      //std::cout<<cjsources<<std::endl;
      for (Json::ValueConstIterator it = cjsources.begin(); it != cjsources.end(); ++it)
	{
	  const Json::Value& process = *it;
	  std::string p_name=process["NAME"].asString();
	  Json::Value p_param=Json::Value::null;
	  if (process.isMember("PARAMETER")) p_param=process["PARAMETER"];
	  // Loop on environenemntal variable
	  uint32_t port=0;
	  const Json::Value& cenv=process["ENV"];
	  for (Json::ValueConstIterator iev = cenv.begin(); iev != cenv.end(); ++iev)
	    {
	      std::string envp=(*iev).asString();
	      //      std::cout<<"Env found "<<envp.substr(0,7)<<std::endl;
	      //std::cout<<"Env found "<<envp.substr(8,envp.length()-7)<<std::endl;
	      if (envp.substr(0,7).compare("WEBPORT")==0)
		{
		  port=atol(envp.substr(8,envp.length()-7).c_str());
		  break;
		}
	    }
	  if (port==0) continue;
	  // Now analyse process Name
	  if (p_name.compare("CAEN")==0)
	    {
	      _caenClient= new fsmwebCaller(host,port);
	      std::string state=_caenClient->queryState();
	      printf("CAEN client %x  %s \n",_caenClient,state.c_str());
	      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" CAEN client State="<<state); 
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _caenClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["caen"]=p_param;
	    }
	  if (p_name.compare("ISEG")==0)
	    {
	      _isegClient= new fsmwebCaller(host,port);
	      std::string state=_isegClient->queryState();
	      printf("ISEG client %x  %s \n",_isegClient,state.c_str());
	      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" ISEG client State="<<state); 
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _isegClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["iseg"]=p_param;
	    }
	  if (p_name.compare("GENESYS")==0)
	    {
	      _genesysClient= new fsmwebCaller(host,port);
	      std::string state=_genesysClient->queryState();
	      printf("GENESYS client %x  %s \n",_genesysClient,state.c_str());
	      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" GENESYS client State="<<state); 
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _genesysClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["genesys"]=p_param;
	      //printf("DB client %x \n",_dbClient);
	    }
	  if (p_name.compare("BMP")==0)
	    {
	      _bmpClient= new fsmwebCaller(host,port);
	      std::string state=_bmpClient->queryState();
	      printf("BMP183 client %x  %s \n",_bmpClient,state.c_str());
	      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" BMP183 client State="<<state); 
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _bmpClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["bmp"]=p_param;
	      //printf("CCC client %x \n",_cccClient);
	    }
	  if (p_name.compare("HIH")==0)
	    {
	      _hihClient= new fsmwebCaller(host,port);
	      std::string state=_hihClient->queryState();
	      printf("HIH8000 client %x  %s \n",_hihClient,state.c_str());
	      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" HIH8000 client State="<<state); 
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _hihClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["hih"]=p_param;
	      //printf("CCC client %x \n",_cccClient);
	    }
	  if (p_name.compare("ZUP")==0)
	    {
	      _zupClient= new fsmwebCaller(host,port);
	      std::string state=_zupClient->queryState();
	      printf("ZUP client %x  %s \n",_zupClient,state.c_str());
	      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" ZUP client State="<<state); 
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _zupClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["zup"]=p_param;
	      //printf("ZUP client %x \n",_zupClient);
	    }
	  if (p_name.compare("GPIO")==0)
	    {
	      _gpioClient= new fsmwebCaller(host,port);
	      std::string state=_gpioClient->queryState();
	      printf("GPIO client %x  %s \n",_gpioClient,state.c_str());
	      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" GPIO client State="<<state); 
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _gpioClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["gpio"]=p_param;

	      //printf("Gpio client %x \n",_gpioClient);
	      _gpioClient->sendTransition("OPEN");
	      _gpioClient->sendCommand("VMEON");
	      ::sleep(1);
	      _gpioClient->sendCommand("VMEOFF");
	      ::sleep(1);
	      _gpioClient->sendCommand("VMEON");
	      
	    }
	  if (p_name.compare("MONITORING")==0)
	    {
	      _monitorClient= new fsmwebCaller(host,port);
	      std::string state=_monitorClient->queryState();
	      printf("MONITORING client %x  %s \n",_monitorClient,state.c_str());
	      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" MONITORING client State="<<state); 
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _monitorClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["monitor"]=p_param;
	      //printf("ZUP client %x \n",_zupClient);
	    }

	}

    }
  
  printf("Clients: CAEN %x GENESYS %x ZUP %x BMP %x HIH %x GPIO %x \n",_caenClient,_genesysClient,_zupClient,_bmpClient,_hihClient,_gpioClient);
}

FullSlow::~FullSlow()
{
 
}


void FullSlow::HVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  
  uint32_t first=atoi(request.get("first","9999").c_str());
  uint32_t last=atoi(request.get("last","9999").c_str());

  if (_caenClient==NULL && _isegClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No HV client");
      response["STATUS"]=Json::Value::null;
      return;
    }
  if (_caenClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last;
      _caenClient->sendCommand("STATUS",sp.str());
  
      response["STATUS"]=_caenClient->answer()["answer"]["STATUS"];
      return;
    }
  if (_isegClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last;
      _isegClient->sendCommand("STATUS",sp.str());
      std::cout<<_isegClient->answer()<<std::endl;
      response["STATUS"]=_isegClient->answer()["answer"]["STATUS"];
      return;
    }
}

void FullSlow::setVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
  float vset=atof(request.get("value","-1.0").c_str());
  if (_caenClient==NULL && _isegClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq, __PRETTY_FUNCTION__<<" No CAEN client");
      response["STATUS"]=Json::Value::null;
      return;
    }
  if (vset<0)
    {
            LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No value set");
	    response["STATUS"]=Json::Value::null;
	    return;

    }
  if (_caenClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value="<<vset;
      _caenClient->sendCommand("VSET",sp.str());
      response["STATUS"]=_caenClient->answer()["answer"]["STATUS"];
      return;
    }
  if (_isegClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value="<<vset;
      _isegClient->sendCommand("VSET",sp.str());
      response["STATUS"]=_isegClient->answer()["answer"]["STATUS"];
      return;
    }
 
}
void FullSlow::setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response)
{

  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
  float iset=atof(request.get("value","-1.0").c_str());
  if (_caenClient==NULL && _isegClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No CAEN client");
      response["STATUS"]=Json::Value::null;
      return;
    }
  if (iset<0)
    {
            LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No value set");
	    response["STATUS"]=Json::Value::null;
	    return;

    }
  if (_caenClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value="<<iset;
      _caenClient->sendCommand("ISET",sp.str());
      response["STATUS"]=_caenClient->answer()["answer"]["STATUS"];
      return;
    }
  if (_isegClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value="<<iset;
      _isegClient->sendCommand("ISET",sp.str());
      response["STATUS"]=_isegClient->answer()["answer"]["STATUS"];
      return;
    }

 
}
void FullSlow::setRampUp(Mongoose::Request &request, Mongoose::JsonResponse &response)
{

  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
  float rup=atof(request.get("value","-1.0").c_str());
  if (_caenClient==NULL && _isegClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No CAEN client");
      response["STATUS"]=Json::Value::null;
      return;
    }
  if (rup<0)
    {
            LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No value set");
	    response["STATUS"]=Json::Value::null;
	    return;

    }
  if (_caenClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value="<<rup;
      _caenClient->sendCommand("RAMPUP",sp.str());
      response["STATUS"]=_caenClient->answer()["answer"]["STATUS"];
      return;
    }
  if (_isegClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last<<"&value="<<rup;
      _isegClient->sendCommand("RAMPUP",sp.str());
      response["STATUS"]=_isegClient->answer()["answer"]["STATUS"];
      return;
    }

 
}

void FullSlow::HVON(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
if (_caenClient==NULL && _isegClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No CAEN client");
      response["STATUS"]=Json::Value::null;
      return;
    }
  if (_caenClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last;
      _caenClient->sendCommand("ON",sp.str());
      response["STATUS"]=_caenClient->answer()["answer"]["STATUS"];
      return;
    }
  if (_isegClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last;
      _isegClient->sendCommand("ON",sp.str());
      response["STATUS"]=_isegClient->answer()["answer"]["STATUS"];
      return;
    }
  

}
void FullSlow::CLEARALARM(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
if (_isegClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No ISEG client");
      response["STATUS"]=Json::Value::null;
      return;
    }
  
  if (_isegClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last;
      _isegClient->sendCommand("CLEARALARM",sp.str());
      response["STATUS"]=_isegClient->answer()["answer"]["STATUS"];
      return;
    }
  

}
void FullSlow::HVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
    uint32_t first=atoi(request.get("first","0").c_str());
  uint32_t last=atoi(request.get("last","0").c_str());
if (_caenClient==NULL && _isegClient==NULL)
    {
      LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No CAEN client");
      response["STATUS"]=Json::Value::null;
      return;
    }
  if (_caenClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last;
      _caenClient->sendCommand("OFF",sp.str());
      response["STATUS"]=_caenClient->answer()["answer"]["STATUS"];
      return;
    }
  if (_isegClient)
    {
      std::stringstream sp;sp<<"&first="<<first<<"&last="<<last;
      _isegClient->sendCommand("OFF",sp.str());
      response["STATUS"]=_isegClient->answer()["answer"]["STATUS"];
      return;
    }
 
}

void FullSlow::LVON(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  bool found=false;
  if (_gpioClient!=NULL){

    _gpioClient->sendCommand("DIFON");
    response["STATUS"]=_gpioClient->answer()["answer"]["STATUS"];
    found=true;
  }
  if (_genesysClient!=NULL){

    _genesysClient->sendCommand("ON");
    response["STATUS"]=_genesysClient->answer()["answer"]["STATUS"];
    found=true;
  }
  if (_zupClient!=NULL){

    _zupClient->sendCommand("ON");
    response["STATUS"]=_zupClient->answer()["answer"]["STATUS"];
    found=true;
  }
  if (found) return;
  LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No LV client");
  response["STATUS"]=Json::Value::null;
  return;
}
void FullSlow::LVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  bool found=false;
  if (_gpioClient!=NULL){

    _gpioClient->sendCommand("DIFOFF");
    response["STATUS"]=_gpioClient->answer()["answer"]["STATUS"];
    found=true;
  }

  if (_genesysClient!=NULL){

    _genesysClient->sendCommand("OFF");
    response["STATUS"]=_genesysClient->answer()["answer"]["STATUS"];
    found=true;

  }
  if (_zupClient!=NULL){

    _zupClient->sendCommand("OFF");
    response["STATUS"]=_zupClient->answer()["answer"]["STATUS"];
    found=true;

  }
  if (found) return;
  LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No LV client");
  response["STATUS"]=Json::Value::null;
  return;
   

}
void  FullSlow::LVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{

  if (_genesysClient!=NULL){

    _genesysClient->sendCommand("STATUS");
    response["STATUS"]=_genesysClient->answer()["answer"]["STATUS"];
    return;
  }
  if (_zupClient!=NULL){

    _zupClient->sendCommand("STATUS");
    std::cout<<_zupClient->answer()<<std::endl;
    response["STATUS"]=_zupClient->answer()["answer"]["STATUS"];
    return;
  }
      if (_gpioClient!=NULL ){

    _gpioClient->sendCommand("STATUS");
    response["STATUS"]=_gpioClient->answer()["answer"]["STATUS"];
    return;
  }

  LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<"No LV client");
  response["STATUS"]=Json::Value::null;
  return;


}
void  FullSlow::PTStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_bmpClient!=NULL){

    _bmpClient->sendCommand("STATUS");
    std::cout<<_bmpClient->answer()<<std::endl;
    response["STATUS"]=_bmpClient->answer()["answer"]["STATUS"];
    return;
  }
  LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No PT client");
  response["STATUS"]=Json::Value::null;
  return;


}
void  FullSlow::HumidityStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_hihClient!=NULL){

    _hihClient->sendCommand("STATUS");
    std::cout<<_hihClient->answer()<<std::endl;
    response["STATUS"]=_hihClient->answer()["answer"]["STATUS"];
    return;
  }
  LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<" No HIH client");
  response["STATUS"]=Json::Value::null;
  return;


}
