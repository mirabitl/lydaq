#include "DbCache.hh"
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <libgen.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <boost/format.hpp>

using namespace lydaq;
using namespace zdaq;


extern  int alphasort(); //Inbuilt sorting function  


int file_select_tdcmanager(const struct direct *entry)  
{  
  if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))  
    return (0);  
  else  
    return (1);  
}  
lydaq::DbCache::DbCache(std::string name) : zdaq::baseApplication(name), _mode("NONE"),_stateName(""),_path(""),_state(NULL),_publisher(NULL),_publish("")
{
  //_fsm=new lytdc::fsm(name);
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::DbCache::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::DbCache::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::DbCache::configure, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","INITIALISED",boost::bind(&lydaq::DbCache::destroy, this,_1));

  
  
  
  //_fsm->addCommand("JOBLOG",boost::bind(&lydaq::DbCache::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS",boost::bind(&lydaq::DbCache::c_status,this,_1,_2));

  _fsm->addCommand("PUBLISH",boost::bind(&lydaq::DbCache::c_publish,this,_1,_2));

  
  
 
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }
  // Initialise Oracle Acccess
#ifndef NO_DB
  std::cout<<"On initialise Oracle "<<std::endl;
  try {
    DBInit::init();
  }
  catch(...)
    {
      LOG4CXX_FATAL(_logDbCache,"Cannot initialise Oracle");
      return;
    }

#endif
  
 
  
}
void lydaq::DbCache::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";
  std::stringstream ss;
  ss<<_path<<"/"<<_stateName;
  std::vector<std::string> res;
  this->ls(ss.str(),res);
  Json::Value ar;
  for (auto x:res)
    {
      ar.append(x);


    }
  response["FILES"]=ar;
}

void lydaq::DbCache::c_publish(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";
  std::stringstream ss;
  ss<<_path<<"/"<<_stateName;
  std::vector<std::string> res;
  this->ls(ss.str(),res);
  Json::Value ar;
  uint8_t buf[0x20000];
  for (auto x:res)
    {
      std::stringstream sn;
      sn<<"/DB/"<<basename((char*) x.c_str())<<"/"<<_stateName<<"/";
      std::string name=sn.str();
      uint32_t len=name.length(),idx=0;
	
      // Read the files
      ar.append(x);
      int fd=::open(x.c_str(),O_RDONLY);
      if (fd<0) continue;
      memset(buf,0,0x20000);
      memcpy(buf,name.c_str(),len);
      idx+=len;
      int size_buf=::read(fd,&buf[idx],0x20000);
      if (size_buf<=0) {::close(fd);continue;}
      idx+=size_buf;
      printf(" Sending %s %d \n",name.c_str(),idx);
      
      zmq::message_t message(idx);
      memcpy(message.data(),buf,idx);
      _publisher->send(message);
      printf("publish done \n");
      ::close(fd);
	
    }
  response["FILES"]=ar;
}


void lydaq::DbCache::destroy(zdaq::fsmmessage* m)
{
  if (_state!=NULL) delete _state;
}
void lydaq::DbCache::initialise(zdaq::fsmmessage* m)
{
  ///LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;

 if (m->content().isMember("path"))
    {
      this->parameters()["path"]=m->content()["path"];
    }
 if (m->content().isMember("publish"))
    {
      this->parameters()["publish"]=m->content()["publish"];
    }
  if (!this->parameters().isMember("path"))
    {
      LOG4CXX_FATAL(_logDbCache,"No write path specified");
      Json::Value jerr="No write path specified";
      m->setAnswer(jerr);
      return;
      
    }
  _path=this->parameters()["path"].asString();

  if (this->parameters().isMember("publish"))
    {
      _publish=this->parameters()["publish"].asString();
      zmq::context_t* _context= new zmq::context_t(1);
      _publisher=  new zmq::socket_t((*_context), ZMQ_PUB);
      _publisher->bind(_publish);
    }
  
  return;
  
}
void lydaq::DbCache::configure(zdaq::fsmmessage* m)
{
  ///LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  if (_state!=NULL) delete _state;
#ifndef NO_DB
  Setup* theOracleSetup_=NULL;
#endif
  
  if (m->content().isMember("mode"))
    {
      this->parameters()["mode"]=m->content()["mode"];
    }
 if (m->content().isMember("path"))
    {
      this->parameters()["path"]=m->content()["path"];
    }
 if (m->content().isMember("state"))
    {
      LOG4CXX_ERROR(_logDbCache,"Changing DB state");
      this->parameters()["state"]=m->content()["state"];
    }
 if (m->content().isMember("dbstate"))
    {
      LOG4CXX_ERROR(_logDbCache,"Changing DB state");
      this->parameters()["state"]=m->content()["dbstate"];
    }
  if (!this->parameters().isMember("mode"))
    {
      LOG4CXX_FATAL(_logDbCache,"No access mode specified");
      Json::Value jerr="No access mode specified";
      m->setAnswer(jerr);
      return;
      
    }
  _mode=this->parameters()["mode"].asString();
  if (!this->parameters().isMember("path"))
    {
      LOG4CXX_FATAL(_logDbCache,"No write path specified");
      Json::Value jerr="No write path specified";
      m->setAnswer(jerr);
      return;
      
    }
  _path=this->parameters()["path"].asString();
  if (!this->parameters().isMember("state"))
    {
      LOG4CXX_FATAL(_logDbCache,"No state or file specified");
      Json::Value jerr="No state or file specified";
      m->setAnswer(jerr);
      return;
      
    }
  _stateName=this->parameters()["state"].asString();
  LOG4CXX_INFO(_logDbCache,"Selected state name "<<_stateName);
  if (_mode.compare("DB")==0)
    {
#ifndef NO_DB

      try {
       theOracleSetup_=Setup::getSetup(_stateName); 
       std::cout<<"On initialise Oracle "<<(long)theOracleSetup_<< std::endl;
       LOG4CXX_INFO(_logDbCache,"Downloading "<<_stateName);
       
     }
     catch(...)
       {
         LOG4CXX_FATAL(_logDbCache,"Setup initialisation failed");
       }
      _state=theOracleSetup_->getStates()[0];
#else
      LOG4CXX_FATAL(_logDbCache,"No Oracle DB access compiled");
      Json::Value jerr="No Oracle DB access compiled";
      m->setAnswer(jerr);
      return;
#endif
      
    }
  if (_mode.compare("WEB")==0)
    {
      try
	{
	  _state = State::getState_WebServer(_stateName);
	}
      catch (ILCException::Exception e)
	{
	  LOG4CXX_ERROR(_logDbCache," Error in Web access"<<e.getMessage());
	  Json::Value jerr=e.getMessage();
	  m->setAnswer(jerr);
	  return;
	}

    }
  if (_mode.compare("FILE")==0)
    {
      
      try
	{
	  std::string fullname=_stateName;
	  std::string basen(basename((char*)_stateName.c_str()));
	  size_t lastindex = basen.find_last_of("."); 
	  _stateName = basen.substr(0, lastindex);
	  _state = State::createStateFromXML_NODB(_stateName,fullname);
	}
      catch (ILCException::Exception e)
	{
	  LOG4CXX_ERROR(_logDbCache," Error in File access"<<e.getMessage());
	  Json::Value jerr=e.getMessage();
	  m->setAnswer(jerr);
	  return;
	}

    }
  OracleDIFDBInterface oi(_state);
  oi.download();
  oi.dumpToTree(_path,_stateName);
  Json::Value jerr=_stateName;
  m->setAnswer(jerr);


#ifndef NO_DB
  if (theOracleSetup_!=NULL)
    {
      delete  theOracleSetup_;
      _state=NULL;
    }
#else
  if (_state!=NULL)
    delete _state;
  _state=NULL;
#endif

  return;
  
}


void lydaq::DbCache::ls(std::string sourcedir,std::vector<std::string>& res)
{
 
  res.clear();
  int count,i;  
  struct direct **files;  
  
  count = scandir(sourcedir.c_str(), &files, file_select_tdcmanager, alphasort);          
  /* If no files found, make a non-selectable menu item */  
  if(count <= 0)    {return ;}
       
  std::stringstream sc;         
  //printf("Number of files = %d\n",count);  
  for (i=1; i<count+1; ++i)  
    {
      // file name
      sc.str(std::string());
      sc<<sourcedir<<"/"<<files[i-1]->d_name;
      res.push_back(sc.str());
       
      /* sc.str(std::string());
       sd.str(std::string());
       sc<<sourcedir<<"/closed/"<<files[i-1]->d_name;
       sd<<sourcedir<<"/"<<files[i-1]->d_name;
       ::unlink(sc.str().c_str());
       ::unlink(sd.str().c_str());
      */
       free(files[i-1]);
    }
  free(files);
  return;
}

