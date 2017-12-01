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


lydaq::TdcManager::TdcManager(std::string name) : zdaq::baseApplication(name), _group(NULL),_context(NULL)
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
  _fsm->addCommand("DOWNLOADDB",boost::bind(&lydaq::TdcManager::c_downloadDB,this,_1,_2));
  
  
  
 
  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logLdaq," Service "<<name<<" is starting on "<<atoi(wp));

      
    _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink
  NL::init();
  
}
void lydaq::TdcManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"Status CMD called ");
  response["STATUS"]="DONE";
   if (_msh==NULL) return;
  Json::Value jl;
  for (uint32_t i=0;i<255;i++)
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
  LOG4CXX_INFO(_logLdaq,"Diflist CMD called ");
  response["STATUS"]="DONE";
  response["DIFLIST"]="EMPTY";
  if (_msh==NULL) return;
  Json::Value jl;
  for (uint32_t i=0;i<255;i++)
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
  LOG4CXX_INFO(_logLdaq,"Set6bdac called ");
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  uint32_t nc=atol(request.get("value","31").c_str());
  
  this->set6bDac(nc&0xFF);
  response["6BDAC"]=nc;
}
void lydaq::TdcManager::c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"set VThTime called ");
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  uint32_t nc=atol(request.get("value","380").c_str());
  LOG4CXX_INFO(_logLdaq,"Value set "<<nc);
  this->setVthTime(nc);
  response["VTHTIME"]=nc;
}
void lydaq::TdcManager::c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"SetMask called ");
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  uint32_t nc=atol(request.get("value","4294967295").c_str());
  
  this->setMask(nc);
  response["MASK"]=nc;
}
void lydaq::TdcManager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"downloadDB called ");
  response["STATUS"]="DONE";

  if (_msh==NULL) return;
  
  std::string dbstate=request.get("state","NONE");
  Json::Value jTDC=this->parameters()["tdc"];
   if (jTDC.isMember("db"))
     {
       Json::Value jTDCdb=jTDC["db"];
       _tca->clear();
       _tca->parseDb(dbstate,jTDCdb["mode"].asString());
     }
  response["DBSTATE"]=dbstate;
}

void lydaq::TdcManager::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;

   // Need a TDC tag
   if (m->content().isMember("tdc"))
     {
       this->parameters()["tdc"]=m->content()["tdc"];
     }
   if (!this->parameters().isMember("tdc"))
     {
       LOG4CXX_ERROR(_logLdaq," No tdc tag found ");
       return;
     }
   // Now create the Message handler
   Json::Value jTDC=this->parameters()["tdc"];
   _msh =new lydaq::TdcMessageHandler("/dev/shm");
   if (!jTDC.isMember("network"))
     {
       LOG4CXX_ERROR(_logLdaq," No tdc:network tag found ");
       return;
     }
   // Scan the network
   std::map<uint32_t,std::string> diflist=lydaq::TdcMessageHandler::scanNetwork(jTDC["network"].asString());
   // Download the configuration
   _tca=new lydaq::TdcConfigAccess();
   if (jTDC.isMember("json"))
     {
       Json::Value jTDCjson=jTDC["json"];
       if (jTDCjson.isMember("file"))
	 {
	   _tca->parseJsonFile(jTDCjson["file"].asString());
	 }
       else
	 if (jTDCjson.isMember("url"))
	   {
	     _tca->parseJsonUrl(jTDCjson["url"].asString());
	   }
     }
    if (jTDC.isMember("db"))
     {
       Json::Value jTDCdb=jTDC["db"];
       _tca->parseDb(jTDCdb["state"].asString(),jTDCdb["mode"].asString());
     }
   if (_tca->asicMap().size()==0)
     {
        LOG4CXX_ERROR(_logLdaq," No ASIC found in the configuration ");
       return;
     }
   // Initialise the network
   if (_group!=NULL) delete _group;
   _group=new NL::SocketGroup();
   _onRead= new lytdc::OnRead(_msh);
  _onClientDisconnect= new lytdc::OnClientDisconnect();
  _onDisconnect= new lytdc::OnDisconnect(_msh);
  _onAccept=new lytdc::OnAccept(_msh);
  _group->setCmdOnRead(_onRead);
  _group->setCmdOnAccept(_onAccept);
  _group->setCmdOnDisconnect(_onClientDisconnect);
   // Loop on Asic Map and find existing DIF
  // Register their slow control socket (10001) and readout one (10002)
  _vsCtrl.clear();
  _vsTdc.clear();
   for (auto x:_tca->asicMap())
     {
       uint32_t eip= ((x.first)>>32)&0XFFFFFFFF;
       std::map<uint32_t,std::string>::iterator idif=diflist.find(eip);
       if (idif==diflist.end()) continue;

       // Slow control
         try
	   {
	     NL::Socket* sc=new NL::Socket(idif->second.c_str(),10001);
	     _group->add(sc);
	     _vsCtrl.push_back(sc);
	     this->startAcquisition(sc,false);
	   }
	 catch (NL::Exception e)
	   {
	     std::stringstream s;
	     s<<e.msg()<<" SLC socket "<<idif->second<<":10001";
	     std::cout<<s.str()<<std::endl;
	     Json::Value array;
	     array["ERROR"]=s.str();
	     
	     m->setAnswer(array);
	     return;
	   }
	 // TDC
         try
	   {
	     NL::Socket* stdc=new NL::Socket(idif->second.c_str(),10002);
	     _group->add(stdc);
	     std::cout<<" TDC socket added "<<idif->second<<":10002"<<std::endl;

	     _msh->setMezzanine(idif->second);
	   }
	 catch (NL::Exception e)
	   {
	     std::stringstream s;
	     s<<e.msg()<<" TDC socket "<<idif->second<<":10002";
	     std::cout<<s.str()<<std::endl;
	     Json::Value array;
	     array["ERROR"]=s.str();
	     
	     m->setAnswer(array);
	     return;
	   }
	 

     }
   //std::string network=
  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);

  if (m->content().isMember("publish"))
    {
      this->parameters()["publish"]=m->content()["publish"];
    }
  if (!this->parameters().isMember("publish"))
    {
      
       LOG4CXX_ERROR(_logLdaq," No publish tag found ");
       return;
    }
  _msh->connect(_context,this->parameters()["publish"].asString());

  // Listen All Wiznet sockets
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
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
   // Now loop on slowcontrol socket
  for (auto x:_vsCtrl)
    {
      this->startAcquisition(x,false);

      // find the hosts and prepare the buffers
      _tca->prepareSlowControl(x->hostTo());
      this->writeRamAvm(x,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

  // do it twice
      //this->writeRamAvm(x,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());
     
      
    }

}

void lydaq::TdcManager::set6bDac(uint8_t dac)
{
  //this->startAcquisition(false);
  ::sleep(1);

 
  for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
    {
      for (int i=0;i<32;i++)
	{
	  it->second.set6bDac(i,dac);
	}      
    }
  // Now loop on slowcontrol socket
  for (auto x:_vsCtrl)
    {
      // find the hosts and prepare the buffers
      _tca->prepareSlowControl(x->hostTo());
      this->writeRamAvm(x,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

  // do it twice
  //    this->writeRamAvm(x,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

      
    }


  ::sleep(1);

}
void lydaq::TdcManager::setMask(uint32_t mask)
{
  //this->startAcquisition(false);
  ::sleep(1);
    // Change all Asics VthTime
  for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
    {
      for (int i=0;i<32;i++)
	{
	  if ((mask>>i)&1)
	    {
	      it->second.setMaskDiscriTime(i,0);
	    }
	  else
	    {
	      it->second.setMaskDiscriTime(i,1);
	    }
	}
      

    }
  // Now loop on slowcontrol socket
  for (auto x:_vsCtrl)
    {
      // find the hosts and prepare the buffers
      _tca->prepareSlowControl(x->hostTo());
      this->writeRamAvm(x,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

  // do it twice
      //  this->writeRamAvm(x,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());
    }

  
  ::sleep(1);

}

void lydaq::TdcManager::setVthTime(uint32_t vth)
{
  // Change all Asics VthTime
  // for (auto x:_tca->asicMap())
  //   {
  //     x.second.setVthTime(vth);
  //   }
  //   for (auto x:_tca->asicMap())
  //   {
  //     x.second.setVthTime(vth);
  //   }

    for (auto it=_tca->asicMap().begin();it!=_tca->asicMap().end();it++)
      it->second.setVthTime(vth);
    
  // Now loop on slowcontrol socket
  for (auto x:_vsCtrl)
    {
      // find the hosts and prepare the buffers
      _tca->prepareSlowControl(x->hostTo());
      this->writeRamAvm(x,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());

  // do it twice
      //  this->writeRamAvm(x,_tca->slcAddr(),_tca->slcBuffer(),_tca->slcBytes());
    }
  // store an "event"

}



void lydaq::TdcManager::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  // Create run file
  Json::Value jc=m->content();
  _run=jc["run"].asInt();
  _type=jc["type"].asInt();
  // Clear evnt number
   for (uint32_t i=0;i<MAX_TDC_NB;i++)
    {
      if (_msh->tdc(i)==NULL) continue;
      _msh->tdc(i)->clear();
    }
 
  switch (_type)
    {
    case 0:
      {
	for (auto x:_vsCtrl)
	  this->startAcquisition(x,true);
      break;
      }
    
    }
}
void lydaq::TdcManager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  for (auto x:_vsCtrl)
    this->startAcquisition(x,false);

  ::sleep(2);
  //g_run.join_all();

}
void lydaq::TdcManager::destroy(zdaq::fsmmessage* m)
{
  _running=false;
  g_run.join_all();
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  
  delete _group;
  LOG4CXX_INFO(_logLdaq," socket Group deleted");
  for (auto x:_vsCtrl)
    delete x;
  LOG4CXX_INFO(_logLdaq," Control sockets deleted");
  for (auto x:_vsTdc)
    delete x;
  LOG4CXX_INFO(_logLdaq," Data sockets deleted");
  _vsCtrl.clear();
  _vsTdc.clear();

  delete _msh;
  LOG4CXX_INFO(_logLdaq," Message Handler deleted");  
}
// void lydaq::TdcManager::parseConfig(std::string name)
// {
//   _slcBytes=0;
//   memset(_slcBuffer,0,0x1000*sizeof(uint16_t));
//   memset(_slcAddr,0,0x1000*sizeof(uint16_t));
  
//   FILE *fp=fopen(name.c_str(), "r");
//   int ier;
//   do
//   {
//     ier=fscanf(fp,"%x",&_slcBuffer[_slcBytes]);
//     _slcAddr[_slcBytes]=_slcBytes;
//     _slcBytes++;
//   } while (ier!=EOF && _slcBytes<0x1000);
//   _slcBuffer[_slcBytes]=0x3;
//   _slcAddr[_slcBytes]=0x201;
//   _slcBytes++;
//   fclose(fp);
// }

void lydaq::TdcManager::writeRamAvm(NL::Socket* sctrl,uint16_t* _slcAddr,uint16_t* _slcBuffer,uint32_t  _slcBytes )
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
  //
  // Creating file  with name = SLC
  std::stringstream sb;
  for (int i=0;i<_slcBytes-1;i++)
  {
   
    sb<<std::hex<<(int)(_slcBuffer[i]&0xFF);
  }
  char name[2512];
  memset(name,0,2512);
  sprintf(name,"/dev/shm/%s/%d/%s",sctrl->hostTo().c_str(),sctrl->portTo(),sb.str().c_str());
  int fd= ::open(name,O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
    printf(" Creating %s \n",name);

  ::close(fd);
  // Send the Buffer
  try
  {
    sctrl->send((const void*) sockbuf,idx*sizeof(uint16_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
 
}
void lydaq::TdcManager::queryCRC(NL::Socket* sctrl)
{
  uint16_t _slcAddr[2];uint16_t _slcBuffer[2];uint32_t  _slcBytes;
  _slcBuffer[0]=0x208;
  _slcAddr[0]=0x1;
  _slcBytes=1;
  _slcBuffer[_slcBytes]=0x3;
  _slcAddr[_slcBytes]=0x201;
   _slcBytes++;
   this->writeRamAvm(sctrl,_slcAddr,_slcBuffer,_slcBytes);
  
}
void lydaq::TdcManager::startAcquisition( NL::Socket* sctrl,bool start)
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
    sctrl->send((const void*) sockbuf,4*sizeof(uint16_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }




  
  
}
