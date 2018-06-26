#include "Builder.hh"
using namespace lydaq;
using namespace zdaq;

lydaq::dsBuilder::dsBuilder(std::string name) : _running(false),_merger(NULL)
{
  _fsm=new zdaq::fsmweb(name);
  _context = new zmq::context_t (1);
  _merger= new zdaq::zmMerger(_context);
  // Register state
  _fsm->addState("CREATED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  _fsm->addTransition("CONFIGURE","CREATED","CONFIGURED",boost::bind(&lydaq::dsBuilder::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::dsBuilder::configure, this,_1));
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::dsBuilder::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&lydaq::dsBuilder::stop, this,_1));
  _fsm->addTransition("HALT","RUNNING","CREATED",boost::bind(&lydaq::dsBuilder::halt, this,_1));
  _fsm->addTransition("HALT","CONFIGURED","CREATED",boost::bind(&lydaq::dsBuilder::halt, this,_1));

  _fsm->addCommand("STATUS",boost::bind(&lydaq::dsBuilder::status, this,_1,_2));
  _fsm->addCommand("REGISTERDS",boost::bind(&lydaq::dsBuilder::registerds, this,_1,_2));
  _fsm->addCommand("SETHEADER",boost::bind(&lydaq::dsBuilder::c_setheader,this,_1,_2));

  //Start server
    char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<"Service "<<name<<" started on port "<<atoi(wp));
    _fsm->start(atoi(wp));
    }


}

void lydaq::dsBuilder::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<"Received "<<m->command()<<" Value "<<m->value());

  
  Json::Value jc=m->content();
  if (jc.isMember("dif"))
    _merger->setNumberOfDataSource(jc["dif"].asInt());
  
    
  const Json::Value& books = jc["stream"];
  Json::Value array_keys;
  for (Json::ValueConstIterator it = books.begin(); it != books.end(); ++it)
    {
      const Json::Value& book = *it;
      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<"Registering "<<(*it).asString());
      _merger->registerDataSource((*it).asString());
      
      array_keys.append((*it).asString());

    }
  const Json::Value& pbooks = jc["processor"];
  Json::Value parray_keys;
  for (Json::ValueConstIterator it = pbooks.begin(); it != pbooks.end(); ++it)
    {
      const Json::Value& book = *it;
      LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<"registering "<<(*it).asString());
      _merger->registerProcessor((*it).asString());
      parray_keys.append((*it).asString());
    }

  LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<" Setting parameters");
  _merger->loadParameters(jc);
  // Overwrite msg
    //Prepare complex answer
  Json::Value prep;
  prep["sourceRegistered"]=array_keys;
  prep["processorRegistered"]=parray_keys;
       
  m->setAnswer(prep);
  LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<"end of configure");
  return;
}

void lydaq::dsBuilder::start(zdaq::fsmmessage* m)
{

    Json::Value jc=m->content();
    _merger->start(jc["run"].asInt());
    _running=true;

    LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<"Run "<<jc["run"].asInt()<<" is started ");
}
void lydaq::dsBuilder::stop(zdaq::fsmmessage* m)
{
  
  _merger->stop();
  _running=false;
  LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<"Builder is stopped \n");fflush(stdout);
}
void lydaq::dsBuilder::halt(zdaq::fsmmessage* m)
{
  
  
  LOG4CXX_INFO(_logLdaq,__PRETTY_FUNCTION__<<"Received "<<m->command());
    if (_running)
      this->stop(m);
    std::cout<<"Destroying"<<std::endl;
    //stop data sources
    _merger->clear();
}
void lydaq::dsBuilder::status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  //std::cout<<"dowmnload"<<request.getUrl()<<" "<<request.getMethod()<<" "<<request.getData()<<std::endl;
    if (_merger!=NULL)
      {

      response["answer"]=_merger->status();

      }
    else
      response["answer"]="NO merger created yet";
}
void lydaq::dsBuilder::registerds(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  //std::cout<<"registerds"<<request.getUrl()<<" "<<request.getMethod()<<" "<<request.getData()<<std::endl;
    if (_merger!=NULL)
      {
	_merger->setNumberOfDataSource(atoi(request.get("ndif","0").c_str()));
	response["answer"]=atoi(request.get("ndif","0").c_str());

      }
    else
      response["answer"]="NO merger created yet";
}
void lydaq::dsBuilder::c_setheader(Mongoose::Request &request, Mongoose::JsonResponse &response)
{

  if (_merger==NULL)    {response["STATUS"]="NO EVB created"; return;}
  std::string shead=request.get("header","None");
  if (shead.compare("None")==0)
    {response["STATUS"]="NO header provided "; return;}
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(shead,jsta);
  if (!parsingSuccessful)
    {response["STATUS"]="Cannot parse header tag "; return;}
  const Json::Value& jdevs=jsta;
  LOG4CXX_DEBUG(_logLdaq,__PRETTY_FUNCTION__<<"Header "<<jdevs);
  std::vector<uint32_t>& v=_merger->runHeader();
  v.clear();
  for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
    v.push_back((*jt).asInt());

  //std::cout<<jdevs<<std::endl;
  //  std::cout<<" LOL "<<std::endl;
  _merger->processRunHeader();
  //  std::cout<<" LOL AGAIN "<<std::endl;
  response["STATUS"]="DONE";
  response["VALUE"]=jsta;

}



