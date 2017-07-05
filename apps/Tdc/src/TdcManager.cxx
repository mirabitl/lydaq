#include "TdcManager.hh"
using namespace lytdc;
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <boost/format.hpp>
#include "fsmwebCaller.hh"
using namespace zdaq;
using namespace lydaq;


lydaq::TdcManager::TdcManager(std::string name) : zdaq::baseApplication(name), _group(NULL),_sCtrl(NULL),_sTDC1(NULL),_sTDC2(NULL),_context(NULL)
{
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::TdcManager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::TdcManager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::TdcManager::configure, this,_1));
  
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::TdcManager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&lydaq::TdcManager::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::TdcManager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&lydaq::TdcManager::destroy, this,_1));
  
  
  
  //_fsm->addCommand("JOBLOG",boost::bind(&lydaq::TdcManager::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS",boost::bind(&lydaq::TdcManager::c_status,this,_1,_2));
  _fsm->addCommand("DIFLIST",boost::bind(&lydaq::TdcManager::c_diflist,this,_1,_2));
  _fsm->addCommand("SET6BDAC",boost::bind(&lydaq::TdcManager::c_set6bdac,this,_1,_2));
  _fsm->addCommand("SETVTHTIME",boost::bind(&lydaq::TdcManager::c_setvthtime,this,_1,_2));
  _fsm->addCommand("SETMASK",boost::bind(&lydaq::TdcManager::c_setMask,this,_1,_2));
  
  
  
 
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink
  NL::init();
  
}
void lydaq::TdcManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";
   if (_msh==NULL) return;
  Json::Value jl;
  for (uint32_t i=0;i<2;i++)
    {
      if (_msh->tdc(i)==NULL) continue;
      Json::Value jt;
      jt["detid"]=_msh->tdc(i)->detectorId();
      jt["sourceid"]=_msh->tdc(i)->difId();
      jt["gtc"]=_msh->tdc(i)->gtc();
      jt["abcid"]=(Json::Value::UInt64)_msh->tdc(i)->abcid();
      jt["event"]=_msh->tdc(i)->event();
      jl.append(jt);
    }
  response["TDCSTATUS"]=jl;
}
void lydaq::TdcManager::c_diflist(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";
  response["DIFLIST"]="EMPTY";
  if (_msh==NULL) return;
  Json::Value jl;
  for (uint32_t i=0;i<2;i++)
    {
      if (_msh->tdc(i)==NULL) continue;
      Json::Value jt;
      jt["detid"]=_msh->tdc(i)->detectorId();
      jt["sourceid"]=_msh->tdc(i)->difId();
      jl.append(jt);
    }
  response["DIFLIST"]=jl;
}

void lydaq::TdcManager::c_set6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  uint32_t nc=atol(request.get("value","31").c_str());
  
  this->set6bDac(nc&0xFF);
  response["6BDAC"]=nc;
}
void lydaq::TdcManager::c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  uint32_t nc=atol(request.get("value","380").c_str());
  
  this->setVthTime(nc);
  response["VTHTIME"]=nc;
}
void lydaq::TdcManager::c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  uint32_t nc=atol(request.get("value","4294967295").c_str());
  
  this->setMask(nc);
  response["MASK"]=nc;
}
void lydaq::TdcManager::initialise(zdaq::fsmmessage* m)
{
  ///LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;

  uint32_t pSLC=0,pTDC1=0,pTDC2=0;
  std::string hSLC=std::string("");
  std::string hTDC1=std::string("");
  std::string hTDC2=std::string("");
   if (m->content().isMember("socket"))
    { 

      pSLC=m->content()["socket"]["portSLC"].asInt();
      hSLC=m->content()["socket"]["hostSLC"].asString();
  
      pTDC1=m->content()["socket"]["portTDC1"].asInt();
      hTDC1=m->content()["socket"]["hostTDC1"].asString();
  
  
      pTDC2=m->content()["socket"]["portTDC2"].asInt();
      hTDC2=m->content()["socket"]["hostTDC2"].asString();

      this->parameters()["socket"]=m->content()["socket"];
    }
   else
     {
       pSLC=this->parameters()["socket"]["portSLC"].asInt();
      hSLC=this->parameters()["socket"]["hostSLC"].asString();
  
      pTDC1=this->parameters()["socket"]["portTDC1"].asInt();
      hTDC1=this->parameters()["socket"]["hostTDC1"].asString();
  
  
      pTDC2=this->parameters()["socket"]["portTDC2"].asInt();
      hTDC2=this->parameters()["socket"]["hostTDC2"].asString();
  
     }
  
  
  if (_group!=NULL) delete _group;
  _group=new NL::SocketGroup();
  _msh =new TdcMessageHandler("/dev/shm");
  _onRead= new lytdc::OnRead(_msh);
  _onClientDisconnect= new lytdc::OnClientDisconnect();
  _onDisconnect= new lytdc::OnDisconnect(_msh);
  
  _onAccept=new lytdc::OnAccept(_msh);
  
  _group->setCmdOnRead(_onRead);
  _group->setCmdOnAccept(_onAccept);
 // _group->setCmdOnDisconnect(_onDisconnect);
  _group->setCmdOnDisconnect(_onClientDisconnect);
  //return;
  std::cout<<hSLC<<":"<<pSLC<<std::endl;
  if (_sCtrl!=NULL) delete _sCtrl;
  try {
  _sCtrl=new NL::Socket(hSLC.c_str(),pSLC);
  _group->add(_sCtrl);
   this->startAcquisition(false);
  }
  catch (NL::Exception e)
  {
     std::stringstream s;
     s<<e.msg()<<" SLC socket "<<hSLC<<":"<<pSLC;
     std::cout<<s.str()<<std::endl;
    Json::Value array;
   array["ERROR"]=s.str();

   m->setAnswer(array);
    return;
  }
  std::cout<<hTDC1<<":"<<pTDC1<<std::endl;
  
  if (_sTDC1!=NULL) delete _sTDC1;
  try {
  _sTDC1=new NL::Socket(hTDC1.c_str(),pTDC1);
  _group->add(_sTDC1);
  _msh->setMezzanine(1,hTDC1);
  }
  catch (NL::Exception e)
  {
     std::stringstream s;
     s<<e.msg()<<" TDC1 socket "<<hTDC1<<":"<<pTDC1;
     std::cout<<s.str()<<std::endl;
    Json::Value array;
   array["ERROR"]=s.str();

   m->setAnswer(array);
    return;
  }
  #define USE_TDC2
  #ifdef USE_TDC2
  std::cout<<hTDC2<<":"<<pTDC2<<std::endl;
  try {
  if (_sTDC2!=NULL) delete _sTDC2;
  _sTDC2=new NL::Socket(hTDC2.c_str(),pTDC2);
  _group->add(_sTDC2);
  _msh->setMezzanine(2,hTDC2);
  }
 catch (NL::Exception e)
  {
     std::stringstream s;
     s<<e.msg()<<" TDC2 socket "<<hTDC2<<":"<<pTDC2;
     std::cout<<s.str()<<std::endl;
    Json::Value array;
   array["ERROR"]=s.str();

   m->setAnswer(array);
    return;
  }
  #endif

  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);

  if (m->content().isMember("publish"))
    {
      _msh->connect(_context,m->content()["publish"].asString());
    }
  // Listen Wiznet socket
  this->listen();
  disconnected_=0;
  
}

void lydaq::TdcManager::dolisten()
{
 
       while(true) {

                if(!_group->listen(2000))
		  std::cout << "\nNo msg recieved during the last 2 seconds";
        }

    
}
void lydaq::TdcManager::listen()
{
  g_store.create_thread(boost::bind(&lydaq::TdcManager::dolisten, this));
  _running=true;
  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));

}
void lydaq::TdcManager::configure(zdaq::fsmmessage* m)
{
  //LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  this->startAcquisition(false);

  std::string ffile=std::string("");
  std::string furl=std::string("");
  if (m->content().isMember("file"))
    { 
      ffile=m->content()["file"].asString();
      this->parameters()["file"]=m->content()["file"];
    }
  else
    if (m->content().isMember("url"))
      {
	furl=m->content()["url"].asString();
	this->parameters()["url"]=m->content()["url"];
      }
    else
      if (this->parameters().isMember("file"))
	{
	  ffile=this->parameters()["file"].asString();
	}
      else
	if (this->parameters().isMember("url"))
	  {
	    furl=this->parameters()["url"].asString();
	  }
	else
	  {
	    std::cout<<" No configuration given"<<std::endl;
	    Json::Value r;
	    r["ERROR"]="No configuration given (file or url)";

	    m->setAnswer(r);
	    return;
	  }
  
  //std::string config=m->content()["configFile"].asString();

  std::cout<<this->parameters()<<std::endl;
  std::cout<<"File is "<<ffile<<std::endl;
  std::cout<<"URL is "<<furl<<std::endl;
  // Read the file
  Json::Value jall;
  if (ffile.length()>2)
    {
  //PRSlow::loadAsics(config,_s1,_s2);
      std::cout<<"File is "<<ffile.length()<<std::endl;
      Json::Reader reader;
      std::ifstream ifs (ffile.c_str(), std::ifstream::in);
      //      Json::Value jall;
      bool parsingSuccessful = reader.parse(ifs,jall,false);
    }
  else
    if (furl.length()>2)
      {
	std::string jsconf=fsmwebCaller::curlQuery(furl);
	std::cout<<jsconf<<std::endl;
	Json::Reader reader;
	bool parsingSuccessful = reader.parse(jsconf,jall);
      
      }
  _s1.setJson(jall["ASIC1"]);
  _s1.dumpJson();
  _s1.Print();
  _s2.setJson(jall["ASIC2"]);
  _s2.dumpJson();
  _s2.Print();

  //  _s1.setVthDiscriCharge(900);
  //_s2.setVthDiscriCharge(900);
  // _s1.setVthTime(900);
  //_s2.setVthTime(900);
  _s2.prepare4Tdc(_slcAddr,_slcBuffer);
  _s1.prepare4Tdc(_slcAddr,_slcBuffer,80);
  //s2.prepare4Tdc(adr,val,80);
  _slcBytes=160;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;

  
  //this->parseConfig(config);
  // Now write data
  this->writeRamAvm();
  //this->queryCRC();
  // Do it twice
  this->writeRamAvm();
}

void lydaq::TdcManager::set6bDac(uint8_t dac)
{
  //this->startAcquisition(false);
  ::sleep(1);
  for (int i=0;i<32;i++)
    {
      _s1.set6bDac(i,dac);
      _s2.set6bDac(i,dac);
    }
  _s2.prepare4Tdc(_slcAddr,_slcBuffer);
  _s1.prepare4Tdc(_slcAddr,_slcBuffer,80);
  //s2.prepare4Tdc(adr,val,80);
  _slcBytes=160;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;
  this->writeRamAvm();

  // do it twice
  this->writeRamAvm();

  // store an "event"
  //this->startAcquisition(true);
  ::sleep(1);

}
void lydaq::TdcManager::setMask(uint32_t mask)
{
  //this->startAcquisition(false);
  ::sleep(1);
  for (int i=0;i<32;i++)
    {
      if ((mask>>i)&1)
	{
	  _s1.setMaskDiscriTime(i,0);
	  _s2.setMaskDiscriTime(i,0);
	}
      else
	{
	  _s1.setMaskDiscriTime(i,1);
	  _s2.setMaskDiscriTime(i,1);
	}
    }
  _s2.prepare4Tdc(_slcAddr,_slcBuffer);
  _s1.prepare4Tdc(_slcAddr,_slcBuffer,80);
  //s2.prepare4Tdc(adr,val,80);
  _slcBytes=160;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;
  this->writeRamAvm();

  // do it twice
  this->writeRamAvm();

  //this->startAcquisition(true);
  ::sleep(1);

}

void lydaq::TdcManager::setVthTime(uint32_t vth)
{
  _s1.setVthTime(vth);
  _s2.setVthTime(vth);
  
  _s2.prepare4Tdc(_slcAddr,_slcBuffer);
  _s1.prepare4Tdc(_slcAddr,_slcBuffer,80);
  //s2.prepare4Tdc(adr,val,80);
  _slcBytes=160;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;
  this->writeRamAvm();

  // do it twice
  this->writeRamAvm();

  // store an "event"

}



void lydaq::TdcManager::start(zdaq::fsmmessage* m)
{
  //LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  // Create run file
  Json::Value jc=m->content();
  _run=jc["run"].asInt();
  _type=jc["type"].asInt();
  // Clear evnt number
   for (uint32_t i=0;i<2;i++)
    {
      if (_msh->tdc(i)==NULL) continue;
      _msh->tdc(i)->clear();
    }
 
  switch (_type)
    {
    case 0:
      {
      this->startAcquisition(true);
      break;
      }
    
    }
}
void lydaq::TdcManager::stop(zdaq::fsmmessage* m)
{
  //LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  
  this->startAcquisition(false);
  ::sleep(2);
  //g_run.join_all();

}
void lydaq::TdcManager::destroy(zdaq::fsmmessage* m)
{
  _running=false;
  g_run.join_all();
  //LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  delete _group;
  delete _sCtrl;
  delete _sTDC1;
  #ifdef USE_TDC2
  delete _sTDC2;
  #endif
  delete _msh;
  
}
void lydaq::TdcManager::parseConfig(std::string name)
{
  _slcBytes=0;
  memset(_slcBuffer,0,0x1000*sizeof(uint16_t));
  memset(_slcAddr,0,0x1000*sizeof(uint16_t));
  
  FILE *fp=fopen(name.c_str(), "r");
  int ier;
  do
  {
    ier=fscanf(fp,"%x",&_slcBuffer[_slcBytes]);
    _slcAddr[_slcBytes]=_slcBytes;
    _slcBytes++;
  } while (ier!=EOF && _slcBytes<0x1000);
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
  _slcBytes++;
  fclose(fp);
}

void lydaq::TdcManager::writeRamAvm()
{
  if (_slcBuffer[1]<2) return;
  uint16_t sockbuf[0x20000];
  sockbuf[0]=htons(0xFF00);
  sockbuf[1]=htons(_slcBytes);
  int idx=2;
  for (int i=0;i<_slcBytes;i++)
  {
    sockbuf[idx]=htons(_slcAddr[i]);
    sockbuf[idx+1]=htons(_slcBuffer[i]);
    idx+=2;
  }
  // Send the Buffer
  try
  {
    _sCtrl->send((const void*) sockbuf,idx*sizeof(uint16_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
  
}
void lydaq::TdcManager::queryCRC()
{
  _slcBuffer[0]=0x208;
  _slcAddr[0]=0x1;
  _slcBytes=1;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
   _slcBytes++;
  this->writeRamAvm();
  
}
void lydaq::TdcManager::startAcquisition( bool start)
{

 uint16_t sockbuf[0x200];
 sockbuf[0]=htons(0xFF00);
 sockbuf[1]=htons(1);
 sockbuf[2]=htons(0x220);
 if (start)
   sockbuf[3]=htons(1);
 else
   sockbuf[3]=htons(0);
  // Send the Buffer
  try
  {
    _sCtrl->send((const void*) sockbuf,4*sizeof(uint16_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }




  
  
}
