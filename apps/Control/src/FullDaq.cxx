#include "FullDaq.hh"
#include <unistd.h>
#include <stdint.h>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include "ReadoutLogger.hh"

using namespace zdaq;
using namespace lydaq;
FullDaq::FullDaq(std::string name) : zdaq::baseApplication(name)
  {
    _builderClient=0;_dbClient=0;_cccClient=0;_mdccClient=0;_zupClient=0;_gpioClient=0,_anClient=0,_finjClient=0;
    _context=NULL;
    _publisher=NULL;
    _survey=false;
    _DIFClients.clear();
    _GRICClients.clear();
    
    _fsm=this->fsm();
    
    
    _fsm->addState("DISCOVERED");
    _fsm->addState("PREPARED");
    _fsm->addState("INITIALISED");
    _fsm->addState("CONFIGURED");
    _fsm->addState("RUNNING");

    _fsm->addTransition("DISCOVER","CREATED","DISCOVERED",boost::bind(&FullDaq::discover, this,_1));
    _fsm->addTransition("DISCOVER","DISCOVERED","DISCOVERED",boost::bind(&FullDaq::discover, this,_1));
    _fsm->addTransition("PREPARE","DISCOVERED","PREPARED",boost::bind(&FullDaq::prepare, this,_1));
    _fsm->addTransition("PREPARE","PREPARED","PREPARED",boost::bind(&FullDaq::prepare, this,_1));
    _fsm->addTransition("INITIALISE","PREPARED","INITIALISED",boost::bind(&FullDaq::initialise, this,_1));
    _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&FullDaq::configure, this,_1));
    _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&FullDaq::configure, this,_1));
    _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&FullDaq::start, this,_1));
    _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&FullDaq::stop, this,_1));
    _fsm->addTransition("DESTROY","CONFIGURED","PREPARED",boost::bind(&FullDaq::destroy, this,_1));
    _fsm->addTransition("DESTROY","INITIALISED","PREPARED",boost::bind(&FullDaq::destroy, this,_1));

    // Commands

    _fsm->addCommand("DOUBLESWITCHZUP",boost::bind(&FullDaq::doubleSwitchZup,this,_1,_2));
    _fsm->addCommand("LISTPROCESS",boost::bind(&FullDaq::listProcess,this,_1,_2));
    _fsm->addCommand("LVSTATUS",boost::bind(&FullDaq::LVStatus,this,_1,_2));
    _fsm->addCommand("LVON",boost::bind(&FullDaq::LVON,this,_1,_2));
    _fsm->addCommand("FORCESTATE",boost::bind(&FullDaq::forceState,this,_1,_2));
    _fsm->addCommand("LVOFF",boost::bind(&FullDaq::LVOFF,this,_1,_2));
    _fsm->addCommand("DOWNLOADDB",boost::bind(&FullDaq::downloadDB,this,_1,_2));
    _fsm->addCommand("CTRLREG",boost::bind(&FullDaq::setControlRegister,this,_1,_2));
    _fsm->addCommand("DBSTATUS",boost::bind(&FullDaq::dbStatus,this,_1,_2));
    _fsm->addCommand("EVBSTATUS",boost::bind(&FullDaq::builderStatus,this,_1,_2));
    _fsm->addCommand("DIFSTATUS",boost::bind(&FullDaq::status,this,_1,_2));
    
    _fsm->addCommand("TDCSTATUS",boost::bind(&FullDaq::tdcstatus,this,_1,_2));
    _fsm->addCommand("PAUSE",boost::bind(&FullDaq::pauseTrigger,this,_1,_2));
    _fsm->addCommand("RESUME",boost::bind(&FullDaq::resumeTrigger,this,_1,_2));
    _fsm->addCommand("ECALPAUSE",boost::bind(&FullDaq::pauseEcal,this,_1,_2));
    _fsm->addCommand("ECALRESUME",boost::bind(&FullDaq::resumeEcal,this,_1,_2));
    _fsm->addCommand("RESETCOUNTERS",boost::bind(&FullDaq::resetTriggerCounters,this,_1,_2));
    _fsm->addCommand("TRIGGERSTATUS",boost::bind(&FullDaq::triggerStatus,this,_1,_2));
    _fsm->addCommand("SPILLON",boost::bind(&FullDaq::triggerSpillOn,this,_1,_2));
    _fsm->addCommand("SPILLOFF",boost::bind(&FullDaq::triggerSpillOff,this,_1,_2));
    _fsm->addCommand("BEAMON",boost::bind(&FullDaq::triggerBeam,this,_1,_2));
    _fsm->addCommand("SETTHRESHOLD",boost::bind(&FullDaq::setThreshold,this,_1,_2));
    _fsm->addCommand("SETGAIN",boost::bind(&FullDaq::setGain,this,_1,_2));
    _fsm->addCommand("REGISTERDS",boost::bind(&FullDaq::registerDataSource,this,_1,_2));

    //Calibration
    _fsm->addCommand("SPILLREGISTER",boost::bind(&FullDaq::triggerSpillRegister,this,_1,_2));
    _fsm->addCommand("SETHARDRESET",boost::bind(&FullDaq::triggerHardReset,this,_1,_2));
    _fsm->addCommand("CALIBCOUNT",boost::bind(&FullDaq::triggerCalibCount,this,_1,_2));
    _fsm->addCommand("CALIBON",boost::bind(&FullDaq::triggerCalibOn,this,_1,_2));
    _fsm->addCommand("CALIBREGISTER",boost::bind(&FullDaq::triggerCalibRegister,this,_1,_2));
    _fsm->addCommand("RELOADCALIB",boost::bind(&FullDaq::triggerReloadCalib,this,_1,_2));
    _fsm->addCommand("SET6BDAC",boost::bind(&FullDaq::tdcSet6bDac,this,_1,_2));
    _fsm->addCommand("CAL6BDAC",boost::bind(&FullDaq::tdcCal6bDac,this,_1,_2));
    _fsm->addCommand("SETVTHTIME",boost::bind(&FullDaq::tdcSetVthTime,this,_1,_2));
    _fsm->addCommand("SETTDCMODE",boost::bind(&FullDaq::tdcSetMode,this,_1,_2));
    _fsm->addCommand("SETTDCDELAYS",boost::bind(&FullDaq::tdcSetDelays,this,_1,_2));
    _fsm->addCommand("SETMASK",boost::bind(&FullDaq::tdcSetMask,this,_1,_2));
    _fsm->addCommand("SETRUNHEADER",boost::bind(&FullDaq::setRunHeader,this,_1,_2));

    
    _fsm->addCommand("TRIGGERSETREG",boost::bind(&FullDaq::triggerSetRegister,this,_1,_2));
    _fsm->addCommand("TRIGGERGETREG",boost::bind(&FullDaq::triggerGetRegister,this,_1,_2));
    _fsm->addCommand("RESETTDC",boost::bind(&FullDaq::resetTdc,this,_1,_2));

    _fsm->addCommand("MONITOR",boost::bind(&FullDaq::monitor,this,_1,_2));
  cout<<"Building FullDaq"<<endl;
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<"Service "<<name<<" started on port "<<atoi(wp));
    _fsm->start(atoi(wp));
    }

  _jConfigContent=Json::Value::null;
  
  }


void  FullDaq::userCreate(zdaq::fsmmessage* m)
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
}

void FullDaq::listProcess(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  Json::Value rep;
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
	  if (p_name.compare("WRITER")==0 )
	    {
	      if (_builderClient == NULL) _builderClient= new fsmwebCaller(host,port);
	      Json::Value jstat=_builderClient->queryWebStatus();Json::Value jres;jres["NAME"]=p_name;jres["HOST"]=host;jres["PORT"]=port;jres["PID"]=jstat["PID"];jres["STATE"]=jstat["STATE"];rep.append(jres);
	      
	     
	    }
	  if (p_name.compare("DBSERVER")==0)
	    {
	       if (_dbClient == NULL) _dbClient= new fsmwebCaller(host,port);
	      Json::Value jstat=_dbClient->queryWebStatus();Json::Value jres;jres["NAME"]=p_name;jres["HOST"]=host;jres["PORT"]=port;jres["PID"]=jstat["PID"];jres["STATE"]=jstat["STATE"];rep.append(jres);

	      //printf("DB client %x \n",_dbClient);
	    }
	  if (p_name.compare("CCCSERVER")==0)
	    {
	      if (_cccClient == NULL) _cccClient= new fsmwebCaller(host,port);
	      Json::Value jstat=_cccClient->queryWebStatus();Json::Value jres;jres["NAME"]=p_name;jres["HOST"]=host;jres["PORT"]=port;jres["PID"]=jstat["PID"];jres["STATE"]=jstat["STATE"];rep.append(jres);
	     
	    }
	  if (p_name.compare("MDCCSERVER")==0)
	    {
	      if (_mdccClient == NULL) _mdccClient= new fsmwebCaller(host,port);
	      Json::Value jstat=_mdccClient->queryWebStatus();Json::Value jres;jres["NAME"]=p_name;jres["HOST"]=host;jres["PORT"]=port;jres["PID"]=jstat["PID"];jres["STATE"]=jstat["STATE"];rep.append(jres);
	      if (!p_param.empty()) this->parameters()["mdcc"]=p_param;
	     
	    }
	  if (p_name.compare("ZUPSERVER")==0)
	    {
	      if (_zupClient == NULL) _zupClient= new fsmwebCaller(host,port);
	      Json::Value jstat=_zupClient->queryWebStatus();Json::Value jres;jres["NAME"]=p_name;jres["HOST"]=host;jres["PORT"]=port;jres["PID"]=jstat["PID"];jres["STATE"]=jstat["STATE"];rep.append(jres);
	      
	    }
	  if (p_name.compare("GPIOSERVER")==0)
	    {
	      if (_gpioClient == NULL) _gpioClient= new fsmwebCaller(host,port);
	      Json::Value jstat=_gpioClient->queryWebStatus();Json::Value jres;jres["NAME"]=p_name;jres["HOST"]=host;jres["PORT"]=port;jres["PID"]=jstat["PID"];jres["STATE"]=jstat["STATE"];rep.append(jres);

	     
	
	    }
	   if (p_name.compare("DIFSERVER")==0)
	    {
	      fsmwebCaller dc(host,port);
	      //printf("DIF client %x \n",dc);
	      Json::Value jstat=dc.queryWebStatus();Json::Value jres;jres["NAME"]=p_name;jres["HOST"]=host;jres["PORT"]=port;jres["PID"]=jstat["PID"];jres["STATE"]=jstat["STATE"];rep.append(jres);

	    }
	   if (p_name.compare("GRICSERVER")==0)
	    {
	      fsmwebCaller dc(host,port);
	      //printf("DIF client %x \n",dc);
	      Json::Value jstat=dc.queryWebStatus();Json::Value jres;jres["NAME"]=p_name;jres["HOST"]=host;jres["PORT"]=port;jres["PID"]=jstat["PID"];jres["STATE"]=jstat["STATE"];rep.append(jres);

	    }
	   if (p_name.compare("TDCSERVER")==0)
	    {
	      fsmwebCaller tdc(host,port);
	      //printf("TDC client %x \n",tdc);
	      Json::Value jstat=tdc.queryWebStatus();Json::Value jres;jres["NAME"]=p_name;jres["HOST"]=host;jres["PORT"]=port;jres["PID"]=jstat["PID"];jres["STATE"]=jstat["STATE"];rep.append(jres);

	    }
	   
	  
	}

    }
  
  response["LIST"]=rep;
  
}
   
void FullDaq::discover(zdaq::fsmmessage* m)
{
  _DIFClients.clear();
  _GRICClients.clear();
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
	  if (p_name.compare("WRITER")==0)
	    {
	      _builderClient= new fsmwebCaller(host,port);
	      std::string state=_builderClient->queryState();
	      printf("Builder client %x  %s \n",_builderClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _builderClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["builder"]=p_param;
	    }
	  if (p_name.compare("DBSERVER")==0)
	    {
	      _dbClient= new fsmwebCaller(host,port);
	      std::string state=_dbClient->queryState();
	      printf("DB client %x  %s \n",_dbClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _dbClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["db"]=p_param;
	      //printf("DB client %x \n",_dbClient);
	    }
	  if (p_name.compare("CCCSERVER")==0)
	    {
	      _cccClient= new fsmwebCaller(host,port);
	      std::string state=_cccClient->queryState();
	      printf("Ccc client %x  %s \n",_cccClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _cccClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["ccc"]=p_param;
	      //printf("CCC client %x \n",_cccClient);
	    }
	  if (p_name.compare("MDCCSERVER")==0)
	    {
	      _mdccClient= new fsmwebCaller(host,port);
	      std::string state=_mdccClient->queryState();
	      printf("Mdcc client %x  %s \n",_mdccClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _mdccClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["mdcc"]=p_param;
		      
	      //printf("MDCC client %x \n",_mdccClient);
	    }
	  if (p_name.compare("ZUPSERVER")==0)
	    {
	      _zupClient= new fsmwebCaller(host,port);
	      std::string state=_zupClient->queryState();
	      printf("ZUP client %x  %s \n",_zupClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _zupClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["zup"]=p_param;
	      //printf("ZUP client %x \n",_zupClient);
	    }
	  if (p_name.compare("ANALYZER")==0)
	    {
	      _anClient= new fsmwebCaller(host,port);
	      std::string state=_anClient->queryState();
	      printf("ANALYZER client %x  %s \n",_anClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _anClient->sendTransition("CREATE",_jConfigContent);
		  _anClient->sendTransition("CONFIGURE");
		}
	      if (!p_param.empty()) this->parameters()["analysis"]=p_param;
	      //printf("ZUP client %x \n",_zupClient);
	    }
	  if (p_name.compare("FEBINJ")==0)
	    {
	      _finjClient= new fsmwebCaller(host,port);
	      std::string state=_finjClient->queryState();
	      printf("FEBINJ client %x  %s \n",_finjClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _finjClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["febinj"]=p_param;
	      //printf("ZUP client %x \n",_zupClient);
	    }

	  if (p_name.compare("GPIOSERVER")==0)
	    {
	      _gpioClient= new fsmwebCaller(host,port);
	      std::string state=_gpioClient->queryState();
	      printf("GPIO client %x  %s \n",_gpioClient,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  _gpioClient->sendTransition("CREATE",_jConfigContent);
		}
	      if (!p_param.empty()) this->parameters()["gpio"]=p_param;

	      //printf("Gpio client %x \n",_gpioClient);
	      _gpioClient->sendTransition("CONFIGURE");
	      _gpioClient->sendCommand("VMEON");
	      _gpioClient->sendCommand("VMEOFF");
	      _gpioClient->sendCommand("VMEON");
	      
	    }
	   if (p_name.compare("DIFSERVER")==0)
	    {
	      fsmwebCaller* dc= new fsmwebCaller(host,port);
	      //printf("DIF client %x \n",dc);
	      std::string state=dc->queryState();
	      printf("DIF client %x  %s \n",dc,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  dc->sendTransition("CREATE",_jConfigContent);
		}

	      _DIFClients.push_back(dc);
	    }
	   if (p_name.compare("GRICSERVER")==0)
	    {
	      fsmwebCaller* dc= new fsmwebCaller(host,port);
	      //printf("DIF client %x \n",dc);
	      std::string state=dc->queryState();
	      printf("GRIC client %x  %s \n",dc,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  dc->sendTransition("CREATE",_jConfigContent);
		}

	      _GRICClients.push_back(dc);
	    }
	   if (p_name.compare("TDCSERVER")==0)
	    {
	      fsmwebCaller* tdc= new fsmwebCaller(host,port);
	      //printf("TDC client %x \n",tdc);
	      std::string state=tdc->queryState();
	      printf("TDC client %x  %s \n",tdc,state.c_str());
	      if (state.compare("VOID")==0 && !_jConfigContent.empty())
		{
		  tdc->sendTransition("CREATE",_jConfigContent);
		}

	      _tdcClients.push_back(tdc);
	    }
	   
	  
	}

    }
  
  printf("Clients: DB %x ZUP %x MDC %x SDCC %x \n",_dbClient,_zupClient,_mdccClient,_cccClient);

}
void FullDaq::prepare(zdaq::fsmmessage* m)
{
  // Tomuvol patch
  #ifdef TOMUVOL
   if (this->parameters().isMember("tmvdb"))
     {
       Json::Value jtmv=this->parameters()["tmvdb"];
       if (jtmv.isMember("account"))
	 {
	   _tmv.connect(jtmv["account"].asString());
	 }
     }
   #endif
  if (this->parameters().isMember("s_ctrlreg"))
    {
      uint32_t ctrlreg=0;
      sscanf(this->parameters()["s_ctrlreg"].asString().c_str(),"%x",&ctrlreg);
      this->parameters()["ctrlreg"]=ctrlreg;
    }
  
  //std::cout<<"ON RENTREEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE"<<std::endl;
  //  for (int i=0;i<100;i++)
   printf("Clients: DB %x ZUP %x MDC %x SDCC %x \n",_dbClient,_zupClient,_mdccClient,_cccClient);

  // DB
  if (_dbClient!=0)
    {
      printf("DB Config %x\n",_dbClient);
      if (this->parameters().isMember("db"))
	{
	  //std::cout<<this->parameters()<<std::endl;
	  //std::cout<<this->parameters()["db"]<<std::endl;
      _dbClient->sendTransition("INITIALISE",this->parameters()["db"]);
      
	  _dbClient->sendTransition("CONFIGURE",this->parameters()["db"]);
	}
      else
      {
        _dbClient->sendTransition("INITIALISE");
      _dbClient->sendTransition("CONFIGURE");
      }
    }
  // Zup
  if (_zupClient)
    {
      printf("ZUP Config\n");
       if (this->parameters().isMember("zup"))
	{
	  _zupClient->sendTransition("CONFIGURE",this->parameters()["zup"]);
	}
       else
	 _zupClient->sendTransition("CONFIGURE");
      //std::cout<<"Current zup values "<<_zupClient->reply()<<std::endl;
    }
   // gpio

  // Ccc
  //std::cout<<" CCC client "<<_cccClient<<std::endl;
  if (_cccClient)
    {
      printf("DCC Config\n");
      if (this->parameters().isMember("ccc"))
	{
	  _cccClient->sendTransition("OPEN",this->parameters()["ccc"]);
	}
       else
	 _cccClient->sendTransition("OPEN");

      //if (_cccClient->answer()!=Json::Value::null)
      _cccClient->sendTransition("INITIALISE");
    }
  // Mdc
  if (_mdccClient)
    {
      printf("MDCC Config\n");
      if (this->parameters().isMember("mdcc"))
	{
	  _mdccClient->sendTransition("OPEN",this->parameters()["mdcc"]);
	}
       else
	 _mdccClient->sendTransition("OPEN");

      _mdccClient->sendCommand("STATUS");
      //std::cout<<"Current MDCC values "<<_mdccClient->reply()<<std::endl;
      _mdccClient->sendTransition("ECALRESUME");
    }
  // Builder
  if (_builderClient)
    {
      fprintf(stderr,"Builder Config\n");
      if (this->parameters().isMember("builder"))
	{
	  _builderClient->sendTransition("CONFIGURE",this->parameters()["builder"]);
	}
       else
	 _builderClient->sendTransition("CONFIGURE");
      fprintf(stderr,"End of Builder configuretion \n");
    }
}


std::string FullDaq::difstatus()
{
  //printf("%s \n",__PRETTY_FUNCTION__);
  Json::Value devlist;
  for (auto dc:_DIFClients)
    {
      dc->sendTransition("STATUS");
      const Json::Value& jdevs=dc->answer();
      for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	devlist.append(*jt);
    }
  
 
  //std::cout<<devlist<<std::endl;
  Json::FastWriter fastWriter;
  return fastWriter.write(devlist);
}

void FullDaq::singlescan(fsmwebCaller* d)
{
 
  d->sendTransition("SCAN");
}
void FullDaq::singleinit(fsmwebCaller* d)
{
  Json::Value c;
  c["difid"]=0;
  d->sendTransition("INITIALISE",c);
}

void FullDaq::singleregisterdb(fsmwebCaller* d)
{
  
  d->sendTransition("REGISTERDB",this->parameters()["db"]);
  //  std::cout<<"Register DB"<<d->parameters()<<std::endl;
}
void FullDaq::singleconfigure(fsmwebCaller* d)
{
  // Essai Json::Value jc=this->parameters()["db"];
  Json::Value jc;
  jc["difid"]=0;
  jc["ctrlreg"]=this->parameters()["ctrlreg"];
  d->sendTransition("CONFIGURE",jc);
}
void FullDaq::singlestart(fsmwebCaller* d)
{
 
  Json::Value c;
  c["difid"]=0;
  d->sendTransition("START",c);
  //std::cout<<"received "<<d->reply()<<std::endl;
}
void FullDaq::singlestop(fsmwebCaller* d)
{

  Json::Value c;
  c["difid"]=0;
  d->sendTransition("STOP",c);
}

Json::Value FullDaq::toJson(std::string s)
{
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(s,jsta);
  return jsta;
}
void FullDaq::initialise(zdaq::fsmmessage* m)
{
  // Configure CCC
  if (_cccClient)
    {
   
      _cccClient->sendTransition("CONFIGURE");
      _cccClient->sendTransition("STOP");
      ::sleep(1);
      _cccClient->sendCommand("CCCRESET");
      _cccClient->sendCommand("DIFRESET");
      ::sleep(1);
    }

  // Make a DIF SCAN
  boost::thread_group g;
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      g.create_thread(boost::bind(&FullDaq::singlescan, this,(*it)));
  g.join_all();

  // Merge devlist
  Json::Value devlist;
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
       const Json::Value& jdevs=(*it)->answer();
      for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	devlist.append(*jt);
    }
  //std::cout<<devlist<<std::endl;

  // Initialise
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
      g.create_thread(boost::bind(&FullDaq::singleinit, this,(*it)));
  g.join_all();
  


  // Initialise TDC
  for (auto tdc:_tdcClients)
    {
      fprintf(stderr,"send transition to TDC \n");
      tdc->sendTransition("INITIALISE");
      fprintf(stderr,"End  of transition to TDC \n");
    }
  for (auto tdc:_GRICClients)
    {
      fprintf(stderr,"send transition to GRIC \n");
      tdc->sendTransition("INITIALISE");
      fprintf(stderr,"End  of transition to GRIC \n");
    }
  
  // Fill status
  m->setAnswer(toJson(this->difstatus()));
}

void FullDaq::configure(zdaq::fsmmessage* m)
{
  // Configure CCC
  //std::cout<<m->content();
  if (_cccClient)
    {
      _cccClient->sendCommand("CCCRESET");
      _cccClient->sendCommand("DIFRESET");
      //std::cout<<"RESET DONE"<<std::endl;
    }
  //PUblish current DIF DB data
  if (_dbClient)
  {
    _dbClient->sendCommand("PUBLISH");
  }
  // register to the dbstate
  boost::thread_group g;
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      g.create_thread(boost::bind(&FullDaq::singleregisterdb, this,(*it)));
    }
  g.join_all();
  //std::cout<<"REGISTER DB DONE"<<std::endl;
  ::sleep(2);
  //Configure them
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
     
      g.create_thread(boost::bind(&FullDaq::singleconfigure, this,(*it)));
    }
  g.join_all();
  //std::cout<<"DIF CONFIGURE  DONE"<<std::endl;
  // Status
  Json::Value jsta= toJson(this->difstatus());
  // Configure the builder
      // Build complete list of data sources
  Json::Value jsou;
  jsou.clear();
  const Json::Value& jdevs=jsta;
  for (Json::ValueConstIterator it = jdevs.begin(); it != jdevs.end(); ++it)
    {
      Json::Value jd;
      jd["detid"]=(*it)["detid"];
      jd["sourceid"]=(*it)["id"];
      jsou.append(jd);
    }
  // Configure GRIC
  for (auto tdc:_GRICClients)
    {
      tdc->sendTransition("CONFIGURE");
            LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<"sending command GRIC STATUS ");
      tdc->sendCommand("STATUS");
      if (!tdc->answer().empty())
	{
	  //std::cout<<"ANSWER "<<tdc->answer()<<std::endl;
	  Json::Value rep=Json::Value::null;
	  if ( tdc->answer().isMember("answer"))
	    rep=tdc->answer()["answer"];
	  if (rep.isMember("GRICSTATUS"))
	    {
	      //std::cout<<rep["DIFLIST"]<<"\n";
	      const Json::Value& jdevs=rep["GRICSTATUS"];
	      for (Json::ValueConstIterator it = jdevs.begin(); it != jdevs.end(); ++it)
		{
		  Json::Value jd;
		  jd["detid"]=(*it)["detid"];
		  jd["sourceid"]=(*it)["sourceid"];
		  jsou.append(jd);
		} 
	    }
	}
      else
	LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<"No answer from STATUS in GRIC!!!!");
    }

  // Configure and Checking tdc
  for (auto tdc:_tdcClients)
    {
      tdc->sendTransition("CONFIGURE");
      //
      LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<"sending command DIFLIST ");
      tdc->sendCommand("DIFLIST");
      if (!tdc->answer().empty())
	{
	  //std::cout<<"ANSWER "<<tdc->answer()<<std::endl;
	  Json::Value rep=Json::Value::null;
	  if ( tdc->answer().isMember("answer"))
	    rep=tdc->answer()["answer"];
	  if (rep.isMember("DIFLIST"))
	    {
	      //std::cout<<rep["DIFLIST"]<<"\n";
	      const Json::Value& jdevs=rep["DIFLIST"];
	      for (Json::ValueConstIterator it = jdevs.begin(); it != jdevs.end(); ++it)
		{
		  Json::Value jd;
		  jd["detid"]=(*it)["detid"];
		  jd["sourceid"]=(*it)["sourceid"];
		  jsou.append(jd);
		} 
	    }
	}
      else
	LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<<"No answer from DIFLIST!!!!");
    }
//       std::cout<<"SENDING "<<jsou<<std::endl;
  if (_builderClient)
    {
      
      Json::Value jl;
      jl["sources"]=jsou;
      std::stringstream sp;sp<<"&ndif="<<jsou.size();
      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<"Sending REGISTERDS "<<sp.str());
      _builderClient->sendCommand("REGISTERDS",sp.str());
    }
  m->setAnswer(jsta);
}

void FullDaq::start(zdaq::fsmmessage* m)
{
  // Get the new run number
 
  LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" calling for new runs from the web interface");
      std::string url="https://ilcconfdb.ipnl.in2p3.fr/runid";
      std::string jsconf=fsmwebCaller::curlQuery(url,this->login());
      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<jsconf);
      Json::Reader reader;
      Json::Value jcc;
      bool parsingSuccessful = reader.parse(jsconf,jcc);
      if (jcc.isMember("runid"))
        _run=jcc["runid"].asUInt();
      else
        _run=10000;
      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" new run "<<_run);
   // Tomuvol patch
      #ifdef TOMUVOL
      if (this->parameters().isMember("tmvdb"))
	{
	  _tmv.initRunTable(_run);
	}
      #endif
   // Start the DIFs
  boost::thread_group g;
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<" calling  singlestart for DIFS ");
      //std::cout<<"Creating thread"<<std::endl;
      g.create_thread(boost::bind(&FullDaq::singlestart, this,(*it)));
    }
  g.join_all();
  //::sleep(5);
  // Start the builder
   if (_builderClient)
    {
      LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<" calling Builder start ");
      Json::Value jl;
      jl["run"]=_run;
      _builderClient->sendTransition("START",jl);
    }
   // Start TDC
   for (auto tdc:_tdcClients)
	{
	  LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<" calling  start for TDCs ");
	  Json::Value jl;
	  jl["run"]=_run;
	  jl["type"]=0;
	  tdc->sendTransition("START",jl);
	}
   // Start the GRIC
   for (auto tdc:_GRICClients)
     {
      tdc->sendTransition("START");
     }

  //Start the CCC
   if (_cccClient)
     {
       LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<" calling SDCC start ");
       _cccClient->sendTransition("START");
     }
  // Resume the MDCC
   if (_mdccClient)
     {
       LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<" calling MDCC start ");
       _mdccClient->sendTransition("RESET");
       //_mdccClient->sendTransition("RESUME");
     }
   LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<" calling ends");
   m->setAnswer(toJson(this->difstatus()));  
}
void FullDaq::stop(zdaq::fsmmessage* m)
{
    // Pause the MDCC
   if (_mdccClient)
     {
       _mdccClient->sendTransition("PAUSE");
     }

    //Stop the CCC
   if (_cccClient)
     {
       _cccClient->sendTransition("STOP");
     }

   // Stop the DIFs
  boost::thread_group g;
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      g.create_thread(boost::bind(&FullDaq::singlestop, this,(*it)));
    }
  g.join_all();
  LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<"end of STOP of DIF Status ");
  // Stop TDC
   for (auto tdc:_tdcClients)
	{
	  tdc->sendTransition("STOP");
	}

   // Stop the GRIC
   for (auto tdc:_GRICClients)
     {
      tdc->sendTransition("STOP");
     }

  ::sleep(1);
   LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<"end of STOP of TDC Status ");
  // Stop the builder
   if (_builderClient)
    {
      _builderClient->sendTransition("STOP");
    }
   LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<"end of STOP waiting for DIF Status ");
   // Tomuvol patch
   #ifdef TOMUVOL
   if (this->parameters().isMember("tmvdb"))
	{
	  _tmv.endRunTable();
	}
   #endif
   m->setAnswer(toJson(this->difstatus()));  
}

void FullDaq::destroy(zdaq::fsmmessage* m)
{
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      Json::Value jl;
      jl["difid"]=0;
      (*it)->sendTransition("DESTROY",jl);
    }
  // Tdc
  for (auto tdc:_tdcClients)
    {
      tdc->sendTransition("DESTROY");
    }
  // Destroy the GRIC
   for (auto tdc:_GRICClients)
     {
      tdc->sendTransition("DESTROY");
     }

}

FullDaq::~FullDaq() 
{
  if (_dbClient) delete _dbClient;
  if (_cccClient) delete _cccClient;
  if (_zupClient) delete _zupClient;
  if (_gpioClient) delete _gpioClient;
  if (_mdccClient) delete _mdccClient;
  if (_builderClient) delete _builderClient;
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    delete (*it);
  
  _DIFClients.clear();
  for (auto tdc:_tdcClients)
    {
      delete tdc;

    }
  _tdcClients.clear();
  
}
void FullDaq::doubleSwitchZup(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
    if (_zupClient==NULL && _gpioClient==NULL)
      {
	LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No zup or GPIO client");
	response["STATUS"]="NO Zup/GPIO CLient";
	return;
      }
    uint32_t npause=atoi(request.get("pause","2").c_str());
    if (_zupClient!=NULL){
      
    _zupClient->sendTransition("OFF");
    sleep((unsigned int) 2);
    _zupClient->sendTransition("ON");
    sleep((unsigned int)npause);
    _zupClient->sendTransition("OFF");
    sleep((unsigned int) 2);
    _zupClient->sendTransition("ON");
    ::sleep( npause);
    LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" LV is ON");
    response["STATUS"]="DONE";

    response["DOUBLESWITCHZUP"]="ON";
    }
    else
      {
	      
    _gpioClient->sendTransition("OFF");
    sleep((unsigned int) 2);
    _gpioClient->sendTransition("ON");
    sleep((unsigned int)npause);
    _gpioClient->sendTransition("OFF");
    sleep((unsigned int) 2);
    _gpioClient->sendTransition("ON");
    ::sleep( npause);
    LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<" LV is ON");
    response["STATUS"]="DONE";

    response["DOUBLESWITCHZUP"]="ON";
      }
  }
void  FullDaq::LVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_zupClient)
    {
      Json::FastWriter fastWriter;
      _zupClient->sendTransition("READ");
      response["STATUS"]="DONE";
      response["LVSTATUS"]=_zupClient->answer();
      return;
    }
   if (_gpioClient)
    {
      Json::FastWriter fastWriter;
      _gpioClient->sendTransition("READ");
      response["STATUS"]="DONE";
      response["LVSTATUS"]=_gpioClient->answer();
      return;
    }
  response["STATUS"]="NO Zup Client";


}
void FullDaq::LVON(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
    if (_zupClient!=NULL){

    _zupClient->sendTransition("ON");
     response["STATUS"]="DONE";
     response["LVON"]=_zupClient->answer();
     return;
    }
    if (_gpioClient!=NULL){

    _gpioClient->sendTransition("ON");
     response["STATUS"]="DONE";
     response["LVON"]=_gpioClient->answer();
     return;
    }
     LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No zup client");response["STATUS"]="NO Zup CLient";return;
  }
void FullDaq::forceState(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
    
    std::string states=request.get("state",_fsm->state());
    _fsm->setState(states);
     response["STATUS"]="DONE";
     response["NEWSTATE"]=states;
  }
void FullDaq::LVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response)
  {
        if (_zupClient!=NULL){

    _zupClient->sendTransition("OFF");
     response["STATUS"]="DONE";
     response["LVON"]=_zupClient->answer();
     return;
    }
    if (_gpioClient!=NULL){

    _gpioClient->sendTransition("OFF");
     response["STATUS"]="DONE";
     response["LVON"]=_gpioClient->answer();
     return;
    }
     LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No zup client");response["STATUS"]="NO Zup CLient";return;
     
   

  }



void FullDaq::downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  std::string statereq=request.get("state","NONE");
  if (statereq.compare("NONE")==0)
    {
      response["STATUS"]="NO STATE GIVEN";
      return;
    }
  Json::FastWriter fastWriter;
  Json::Value fromScratch;
  fromScratch.clear();
  if (_dbClient!=NULL)
    {
      _dbstate=statereq;
      this->parameters()["db"]["dbstate"]=statereq;
      this->parameters()["db"]["state"]=statereq;

      _dbClient->sendTransition("DELETE",this->parameters()["db"]);
      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" Downloading"<<this->parameters()["db"]["dbstate"].asString());
  
      _dbClient->sendTransition("CONFIGURE",this->parameters()["db"]);
  
      _dbClient->sendCommand("PUBLISH");
  
      response["STATUS"]="DONE";
      response["DOWNLOADBD"]=_dbstate;
    }
  else
   {
     // Tdc
     if (_tdcClients.size()>0)
       {
	 _dbstate=statereq;
	 this->parameters()["db"]["dbstate"]=statereq;
	 this->parameters()["db"]["state"]=statereq;
	 
	 for (auto tdc:_tdcClients)
	   {
	     std::stringstream sp;sp<<"&state="<<statereq;
	     tdc->sendCommand("DOWNLOADDB",sp.str());
	   }
	 response["STATUS"]="DONE";
	 response["DOWNLOADBD"]=_dbstate;
       }
     else
       {
	 LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No DB client"); response["STATUS"]="NO DB Client";return;
       }
   }
}

void FullDaq::setControlRegister(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t ctrlreg=0;
  sscanf(request.get("value","0x0").c_str(),"%x",&ctrlreg);
  this->parameters()["ctrlreg"]=ctrlreg;
  
    
  // uint32_t reg=atoi();
  // this->parameters()["ctrlreg"]=reg;
 
  response["STATUS"]="DONE";
  response["CTRLREG"]=ctrlreg;
  LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" Change Ctrlreg to "<<std::hex<<_ctrlreg<<std::dec);
}

void FullDaq::dbStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  //  printf("%s \n",__PRETTY_FUNCTION__);
  response["run"]=_run;
  response["state"]= "unknown";
  //std::cout<<this->parameters()<<std::endl;
  if (!this->parameters().empty())
    {
      if (this->parameters().isMember("db"))
	{
	   response["state"]= this->parameters()["db"]["state"];
	}
    }
  
  response["STATUS"]="DONE";
}
void FullDaq::registerDataSource(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t detid=atoi(request.get("detid","0").c_str());
  uint32_t sid=atoi(request.get("sourceid","0").c_str());
  if (detid==0 || sid==0)
    {
      response["STATUS"]="Missing detid or sourceid";
      return;
    }
  if (_dbClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No DB client"); response["STATUS"]="NO DB Client";return;}
  Json::Value jl;
  jl["detid"]=detid;
  jl["sourceid"]=sid;
  
  //_builderClient->sendTransition("REGISTERDS",jl);
   response["STATUS"]="DONE";
   response["IDS"]=sid;
}

void  FullDaq::builderStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  printf("%s \n",__PRETTY_FUNCTION__);
  Json::FastWriter fastWriter;
  Json::Value fromScratch;
  response["run"]=-1;
  response["event"]=-1;
  
  if (_builderClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No SHM client");response["STATUS"]= "No SHM client";return;}
  _builderClient->sendCommand("STATUS");
  LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<_builderClient->answer());
  if (!_builderClient->answer().empty())
    {
      if (_builderClient->answer().isMember("answer"))
	
	  response["run"]=_builderClient->answer()["answer"]["answer"]["run"];
	  response["event"]=_builderClient->answer()["answer"]["answer"]["event"];
	  response["builder"]=_builderClient->answer();
	
    }
  response["STATUS"]="DONE";
}

void FullDaq::tdcstatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  
  Json::Value devlist;
  for (std::vector<fsmwebCaller*>::iterator it=_tdcClients.begin();it!=_tdcClients.end();it++)
    {

      (*it)->sendCommand("STATUS");
      const Json::Value& jdevs=(*it)->answer();
      if (jdevs.isMember("answer"))
	if (jdevs["answer"].isMember("TDCSTATUS"))
      //for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	  devlist.append(jdevs["answer"]["TDCSTATUS"]);
    }
  response["tdclist"]=devlist;
  response["STATUS"]="DONE";
}

void FullDaq::status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  Json::Value devlist;
  for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {

      (*it)->sendCommand("STATUS");
      const Json::Value& jdevs=(*it)->answer();
      if (jdevs.isMember("answer"))
	if (jdevs["answer"].isMember("DIFLIST"))
	  {
	    const Json::Value& jdev1=jdevs["answer"]["DIFLIST"];
	    //std::cout<<"GROS DEBUG "<<jdevs<<std::endl;
	    for (Json::ValueConstIterator jt = jdev1.begin(); jt != jdev1.end(); ++jt)
	      devlist.append(*jt);
	  }
    }
  response["diflist"]=devlist;
  response["STATUS"]="DONE";
}
void FullDaq::pauseTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("PAUSE");
  response["TRIGGER"]="PAUSED";
  response["STATUS"]="DONE";
}
void FullDaq::resumeTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("RESUME");
  response["TRIGGER"]="RESUMED";
  response["STATUS"]="DONE";
}

void FullDaq::pauseEcal(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("ECALPAUSE");
  response["ECAL"]="PAUSED";
  response["STATUS"]="DONE";

}
void FullDaq::resumeEcal(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("ECALRESUME");
  response["ECAL"]="RESUMED";
  response["STATUS"]="DONE";
}
void FullDaq::resetTriggerCounters(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendTransition("RESET");
  response["COUNTERS"]="RESETTED";
  response["STATUS"]="DONE";
}
// a continuer
void FullDaq::triggerStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
  _mdccClient->sendCommand("STATUS");
  response["COUNTERS"]=_mdccClient->answer()["answer"]["COUNTERS"];
  response["STATUS"]="DONE";
}
void FullDaq::triggerSpillOn(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","50").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
 
  std::stringstream sp;sp<<"&nclock="<<nc;
  _mdccClient->sendCommand("SPILLON",sp.str());
  
  response["SPILLON"]=nc;
  response["STATUS"]="DONE";
  return;
}
void FullDaq::triggerSpillOff(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","50000").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
   std::stringstream sp;sp<<"&nclock="<<nc;
  _mdccClient->sendCommand("SPILLOFF",sp.str());
  
  response["SPILLOFF"]=nc;
  response["STATUS"]="DONE";
  
  return;
}
void FullDaq::triggerBeam(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","250000000").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
   std::stringstream sp;sp<<"&nclock="<<nc;
  _mdccClient->sendCommand("BEAMON",sp.str());
  response["BEAM"]=nc;
  response["STATUS"]="DONE";
  
  return;
}

void FullDaq::triggerSpillRegister(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("value","0").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
 
  std::stringstream sp;sp<<"&value="<<nc;
  _mdccClient->sendCommand("SETSPILLREGISTER",sp.str());
  
  response["SPILLREGISTER"]=nc;
  response["STATUS"]="DONE";
  return;
}

void FullDaq::triggerHardReset(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("value","1").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
 
  std::stringstream sp;sp<<"&value="<<nc;
  _mdccClient->sendCommand("SETHARDRESET",sp.str());
  
  response["SETHARDRESET"]=nc;
  response["STATUS"]="DONE";
  return;
}
void FullDaq::triggerCalibCount(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("clock","50").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
   std::stringstream sp;sp<<"&nclock="<<nc;
  _mdccClient->sendCommand("SETCALIBCOUNT",sp.str());
  response["CALIBCOUNT"]=nc;
  response["STATUS"]="DONE";
  
  return;
}

void FullDaq::triggerCalibOn(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("value","1").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
 

  if (nc==0)
    _mdccClient->sendCommand("CALIBOFF");
  else
    _mdccClient->sendCommand("CALIBON");
  response["CALIBON"]=nc;
  response["STATUS"]="DONE";
  return;
}
void FullDaq::triggerCalibRegister(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t nc=atoi(request.get("value","4").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
   std::stringstream sp;sp<<"&value="<<nc;
  _mdccClient->sendCommand("SETCALIBREGISTER",sp.str());
 
  response["CALIBREGISTER"]=nc;
  response["STATUS"]="DONE";
  return;
}
void FullDaq::triggerSetRegister(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t adr=atoi(request.get("address","2").c_str());
  uint32_t value=atoi(request.get("value","0").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
  std::stringstream sp;sp<<"&value="<<value<<"&address="<<adr;
  _mdccClient->sendCommand("SETREG",sp.str());
 
  response["REGISTER"]=_mdccClient->answer()["answer"]["VALUE"];
  response["STATUS"]="DONE";
  return;
}
void FullDaq::triggerGetRegister(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  uint32_t adr=atoi(request.get("address","2").c_str());
  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
  std::stringstream sp;sp<<"&address="<<adr;
  _mdccClient->sendCommand("GETREG",sp.str());
  //std::cout<<"GETREG "<<_mdccClient->answer()<<std::endl;
  response["REGISTER"]=_mdccClient->answer()["answer"]["VALUE"];
  response["STATUS"]="DONE";
  return;
}
void FullDaq::resetTdc(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{

  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}
  std::stringstream sp;sp<<"&value="<<0;
  _mdccClient->sendCommand("RESETTDC",sp.str());
  std::stringstream sp1;sp1<<"&value="<<1;
  _mdccClient->sendCommand("RESETTDC",sp1.str());
  //std::cout<<"GETREG "<<_mdccClient->answer()<<std::endl;
  response["RESET"]=_mdccClient->answer()["answer"]["VALUE"];
  response["STATUS"]="DONE";
  return;
}

void FullDaq::triggerReloadCalib(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{

  if (_mdccClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No MDC client");response["STATUS"]= "No MDCC client";return;}

    _mdccClient->sendCommand("RELOADCALIB");

  response["STATUS"]="DONE";
  return;
}

void FullDaq::tdcSet6bDac(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{

  uint32_t nc=atoi(request.get("value","31").c_str());
  for (auto tdc:_tdcClients)
    {
      std::stringstream sp;sp<<"&value="<<nc;
      tdc->sendCommand("SET6BDAC",sp.str());
    }
  response["DAC6B"]=nc;
  response["STATUS"]="DONE";
  return;
}
void FullDaq::tdcCal6bDac(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{


  uint32_t mask = atol(request.get("mask", "4294967295").c_str());
  int32_t shift = atol(request.get("shift", "0").c_str());
  for (auto tdc:_tdcClients)
    {
      std::stringstream sp;
      sp<<"&mask="<<mask;
      sp<<"&shift="<<shift;
      tdc->sendCommand("CAL6BDAC",sp.str());
    }
  response["DAC6B"]=shift;
  response["MASK"]=mask;
  response["STATUS"]="DONE";
  return;
}
void FullDaq::tdcSetVthTime(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{

  uint32_t nc=atoi(request.get("value","380").c_str());
  for (auto tdc:_tdcClients)
    {
      std::stringstream sp;sp<<"&value="<<nc;
      tdc->sendCommand("SETVTHTIME",sp.str());
    }
  response["VTHTIME"]=nc;
  response["STATUS"]="DONE";
  return;
}
void FullDaq::tdcSetMode(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{

  uint32_t nc=atoi(request.get("value","2").c_str());
  for (auto tdc:_tdcClients)
    {
      std::stringstream sp;sp<<"&value="<<nc;
      tdc->sendCommand("SETMODE",sp.str());
    }
  response["TDCMODE"]=nc;
  response["STATUS"]="DONE";
  return;
}
void FullDaq::tdcSetDelays(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{

  uint32_t active=atoi(request.get("active","3").c_str());
  uint32_t dead=atoi(request.get("dead","5").c_str());
  for (auto tdc:_tdcClients)
    {
      std::stringstream sp;sp<<"&value="<<active;
      tdc->sendCommand("SETDELAY",sp.str());
      std::cout<<"Sending SETTDCDELAY"<<sp.str()<<std::endl;
      std::stringstream sp1;sp1<<"&value="<<dead;
      tdc->sendCommand("SETDURATION",sp1.str());
    }
  response["ACTIVE"]=active;
  response["DEAD"]=dead;
  response["STATUS"]="DONE";
  return;
}



void FullDaq::tdcSetMask(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{

  uint32_t nc=atoi(request.get("value","31").c_str());
  uint32_t asic=atoi(request.get("asic","255").c_str());
  for (auto tdc:_tdcClients)
    {
      std::stringstream sp;sp<<"&value="<<nc<<"&asic="<<asic;
      tdc->sendCommand("SETMASK",sp.str());
    }
  response["MASK"]=nc;
  response["STATUS"]="DONE";
  return;
}

void FullDaq::setRunHeader(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{

  if (_builderClient==NULL){LOG4CXX_ERROR(_logLdaq,__PRETTY_FUNCTION__<< "No Builder client");response["STATUS"]= "No Builder client";return;}
    uint32_t rtyp=atoi(request.get("type","0").c_str());
    uint32_t rval=atoi(request.get("value","0").c_str());
    uint32_t mask=atoi(request.get("mask","4294967295").c_str());
    std::stringstream sp;sp<<"&header=["<<rtyp<<","<<rval<<"]";
    _builderClient->sendCommand("SETHEADER",sp.str());

  response["STATUS"]="DONE";
  response["HEADER"]=sp.str();
  return;
}



void FullDaq::setThreshold(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  if (state().compare("CONFIGURED")!=0)
    {
      response["STATUS"]="SetThreshold cannot be called if not in CONFIGURED state";
      return;
    }
  std::stringstream saction;
  saction<<"&B0="<<request.get("B0","250");
  saction<<"&B1="<<request.get("B1","250");
  saction<<"&B2="<<request.get("B2","250");
  saction<<"&CTRLREG="<<std::hex<<this->parameters()["ctrlreg"].asUInt();

  Json::Value jlist;
 for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      
      std::string rep=(*it)->sendCommand("SETTHRESHOLD",saction.str());
      Json::Value jrep;
      jrep["diflist"]=rep;
      jlist.append(jrep);
    }
 response["HOSTS"]=jlist;
 response["STATUS"]="DONE";
}

void FullDaq::setGain(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{
  if (state().compare("CONFIGURED")!=0)
    {
      response["STATUS"]="SetGAIN cannot be called if not in CONFIGURED state";
      return;
    }

  std::stringstream saction;
  saction<<"&GAIN="<<request.get("GAIN","128");
  saction<<"&CTRLREG="<<std::hex<<this->parameters()["ctrlreg"].asUInt();
  Json::Value jlist;
 for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
    {
      
      std::string rep=(*it)->sendCommand("SETGAIN",saction.str());
      Json::Value jrep;
      jrep["diflist"]=rep;
      jlist.append(jrep);
    }
 response["HOSTS"]=jlist;
 response["STATUS"]="DONE";
  
  
}
void FullDaq::startSurvey()
{
  if (!this->parameters().isMember("TCPPort")) return;
      
  if (!this->parameters().isMember("period"))
    {
      std::cout<<"Please define Reading period"<<std::endl;
      return;
    }
  else
    _period=this->parameters()["period"].asUInt();
  
  if (this->parameters().isMember("TCPPort") && _context==NULL)
    {
      _context= new zmq::context_t(1);
      _publisher= new zmq::socket_t((*_context), ZMQ_PUB);
      std::stringstream sport;
      sport<<"tcp://*:"<<this->parameters()["TCPPort"].asUInt();
      std::cout<<"Binding to "<<sport.str()<<std::endl;
      _publisher->bind(sport.str());
      
    }
    
  if (!this->parameters().isMember("location"))
    this->parameters()["location"]="/HOME";
  _survey=true;
  g_survey.create_thread(boost::bind(&lydaq::FullDaq::survey, this));

    

}
void FullDaq::stopSurvey()
{
  if (!this->parameters().isMember("TCPPort")) return;
  //
  _survey=false;
  g_survey.join_all();
}

void FullDaq::survey()
{
  Json::FastWriter fastWriter;
  ::sleep(1);
  while (_survey)
    {
    
      if (_publisher==NULL)
	{
	  std::cout<<"No publisher defined"<<std::endl;
	  break;
	}
          // ID
      std::stringstream sheader;
      sheader<<"FDAQ@"<<this->parameters()["location"].asString()<<"@"<<time(0);
      std::string head=sheader.str();
    
      zmq::message_t ma1((void*)head.c_str(), head.length(), NULL); 
      _publisher->send(ma1, ZMQ_SNDMORE);
      // Status
      Json::Value jstatus=this->surveyStatus();
      std::string scont= fastWriter.write(jstatus);
      zmq::message_t ma2((void*)scont.c_str(), scont.length(), NULL); 

      std::cout<<"publishing "<<head<<" =>"<<scont<<std::endl;
      _publisher->send(ma2);
      if (!_survey) break;
      ::sleep(_period);
    }
  std::cout<<"End of survey task"<<std::endl;

}

Json::Value FullDaq::surveyStatus()
{
  Json::Value rep;
  
  if (_builderClient)
    {
      _builderClient->sendCommand("STATUS");
      LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<_builderClient->answer());
      if (!_builderClient->answer().empty())
	{
	  if (_builderClient->answer().isMember("answer"))
	    {
	      rep["builder"]["run"]=_builderClient->answer()["answer"]["answer"]["run"];
	      rep["builder"]["event"]=_builderClient->answer()["answer"]["answer"]["event"];
	    }
	}
    }
  if (!this->parameters().empty())
    {
      if (this->parameters().isMember("db"))
	{
	  rep["db"]["state"]= this->parameters()["db"]["state"];
	}
    }
  if (_tdcClients.size()>0)
    {
      Json::Value devlist;
      for (std::vector<fsmwebCaller*>::iterator it=_tdcClients.begin();it!=_tdcClients.end();it++)
	{

	  (*it)->sendCommand("STATUS");
	  const Json::Value& jdevs=(*it)->answer();
	  if (jdevs.isMember("answer"))
	    if (jdevs["answer"].isMember("TDCSTATUS"))
	      //for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	      devlist.append(jdevs["answer"]["TDCSTATUS"]);
	}
      rep["FEB"]=devlist;
    }
  if (_DIFClients.size()>0)
    {
      Json::Value devlist;
      for (std::vector<fsmwebCaller*>::iterator it=_DIFClients.begin();it!=_DIFClients.end();it++)
	{

	  (*it)->sendCommand("STATUS");
	  const Json::Value& jdevs=(*it)->answer();
	  if (jdevs.isMember("answer"))
	    if (jdevs["answer"].isMember("DIFLIST"))
	      {
		const Json::Value& jdev1=jdevs["answer"]["DIFLIST"];
		//std::cout<<"GROS DEBUG "<<jdevs<<std::endl;
		for (Json::ValueConstIterator jt = jdev1.begin(); jt != jdev1.end(); ++jt)
		  devlist.append(*jt);
	      }
	}
      rep["DIF"]=devlist;
    }

  rep["STATE"]=this->fsm()->state();
  return rep;
}
void FullDaq::monitor(Mongoose::Request &request, Mongoose::JsonResponse &response)//uint32_t nc)
{

  uint32_t nc=atoi(request.get("value","0").c_str());
  if (nc!=0 && !_survey)
    startSurvey();
  else
    if (_survey)
      stopSurvey();
  response["MODE"]=nc;

  response["STATUS"]="DONE";
  return;
}
