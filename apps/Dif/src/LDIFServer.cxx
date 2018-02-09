
#include "LDIFServer.hh"
#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include "ftdi.hpp"
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <cstring>
#include <string.h>

using namespace Ftdi;
using namespace lydaq;
using namespace zdaq;

void lydaq::LDIFServer::registerdb(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  _dbstate=m->content()["dbstate"].asString();
}

void lydaq::LDIFServer::dbcache(std::string server,std::vector<uint32_t> vids)
{
  std::cout << "Collecting updates from DBCACHE server...\n" << std::endl;
  zmq::context_t c(2);
  zmq::socket_t subscriber(c, ZMQ_SUB);
  subscriber.connect(server);
  for (auto x:vids)
    {
      std::stringstream ss;
      ss<<"/DB/"<<(x&0xFFFF)<<"/";
      //ss<<"*/"<<i<<"/";
      subscriber.setsockopt(ZMQ_SUBSCRIBE,ss.str().c_str(),ss.str().length());
    }
  while (_dbcacheRunning)
    {
       zmq::message_t update;
       subscriber.recv(&update);
       std::cout<<"update size "<<update.size()<<"\n";
       char cid[200],cdb[200];
       memset(cid,0,200);
       memset(cdb,0,200);

       uint32_t difid;
        char *msg=((char*) update.data());
       char *omsg=msg;
       
       msg+=4;
       char *ret=strstr(msg, "/");
       memcpy(cid,msg,ret-msg);
       msg=ret+1;
       ret=strstr(msg, "/");
       memcpy(cdb,msg,ret-msg);
       unsigned char* buf=(unsigned char*)ret;
       buf+=1;
       sscanf(cid,"%d",&difid);
       //std::string sdbst(dbst);
       uint32_t hsi=ret-omsg+1;
       printf("recived %d bytes %s %s buf size=%d \n",update.size(),cid,cdb,hsi);
       uint32_t *ibuf=(uint32_t *) buf;
       if (theDIFMap_.find(difid)== theDIFMap_.end()) continue;
      //memcpy(&theDIFDbInfo_[i],curr->getData(),sizeof(DIFDbInfo));
       memcpy(theDIFMap_[difid]->dbdif(),buf,update.size()-((char*) buf-omsg));
       //printf("Dim info read %d %d \n",theDIFMap_[i]->dbdif()->id,theDIFMap_[i]->dbdif()->nbasic);
       LOG4CXX_INFO(_logLdaq,"DIF "<<theDIFMap_[difid]->dbdif()->id<<" is read from DB with nasic="<<theDIFMap_[difid]->dbdif()->nbasic<<" from state "<<cdb);
       std::cout<<"DIF "<<theDIFMap_[difid]->dbdif()->id<<" is read from DB with nasic="<<theDIFMap_[difid]->dbdif()->nbasic<<" from state "<<cdb<<std::endl;
       
    }
  std::cout << "Exiting updates from DBCACHE server...\n" << std::endl;
  
}
void lydaq::LDIFServer::scan(zdaq::fsmmessage* m) 
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  // Store dbcache if changed
  if (m->content().isMember("dbcache"))
    this->parameters()["dbcache"]=m->content()["dbcache"];
  //
   this->prepareDevices();
   std::map<uint32_t,FtdiDeviceInfo*>& fm=this->getFtdiMap();
   std::map<uint32_t,LDIF*> dm=this->getDIFMap();
   //LOG4CXX_INFO(_logLdaq," CMD: SCANDEVICE clear Maps");
   for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
     { if (it->second!=NULL) delete it->second;}
   dm.clear();
   // _ndif=0;
   std::vector<uint32_t> vids;
   Json::Value array;
   for ( std::map<uint32_t,FtdiDeviceInfo*>::iterator it=fm.begin();it!=fm.end();it++)
     {

       LDIF* d= new LDIF(it->second);
       this->getDIFMap().insert(std::make_pair(it->first,d));
       LOG4CXX_INFO(_logLdaq," CMD: SCANDEVICE created LDIF @ "<<std::hex<<d<<std::dec);
       Json::Value jd;
       jd["detid"]=d->detectorId();
       jd["sourceid"]=it->first;
       vids.push_back( (d->detectorId()<<16|it->first));
       array.append(jd);
     }

   if (this->parameters().isMember("dbcache"))
     {
       if (!_dbcacheRunning)
	 {
	   _dbcacheRunning=true;
	   g_db.create_thread(boost::bind(&lydaq::LDIFServer::dbcache,this,this->parameters()["dbcache"].asString(),vids));
	 }
     }

   
   m->setAnswer(array);
}


void lydaq::LDIFServer::initialise(zdaq::fsmmessage* m)
{
  
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();

  // Found the detid
  uint32_t detid=100;
  if (this->parameters().isMember("detectorId")) detid=this->parameters()["detectorId"].asUInt();
    
  // Found the builder address
  
  std::string builderAddress="";
  if (this->parameters().isMember("builderAddress"))
    {
      builderAddress=this->parameters()["builderAddress"].asString();
      std::cout<<" builder address is "<<builderAddress<<" length" <<builderAddress.length()<<std::endl;
    }
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}
      zdaq::zmPusher* push=NULL;
      if (_context!=NULL && difid>0 && builderAddress.length()>4)
	{
	  std::cout<<" Creating pusher to "<<builderAddress<<std::endl;
	  push=new zdaq::zmPusher(_context,detid,difid);
	  push->connect(builderAddress);
	}
      itd->second->initialise(push);
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  LOG4CXX_INFO(_logLdaq," calling initialise LDIF @ "<<std::hex<<it->second<<std::dec);
	  zdaq::zmPusher* push=NULL;
	  if (_context!=NULL  && builderAddress.length()>4)
	    {
	      std::cout<<" Creating pusher to "<<builderAddress<<std::endl;
	      push=new zdaq::zmPusher(_context,detid,it->first);
	      push->connect(builderAddress);
	    }

	  it->second->initialise(push);
	}
      return;
    }
}

void lydaq::LDIFServer::setGain(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t difid=atoi(request.get("difid","0").c_str());
  uint32_t gain=atoi(request.get("gain","0").c_str());
  uint32_t ctrlreg1;
  sscanf(request.get("CTRLREG","0").c_str(),"%x",&ctrlreg1);
  printf("ctrl reg = %u %x \n",ctrlreg1,ctrlreg1);
  // uint32_t ctrlreg= jt["ctrlreg"].asUInt();
  //printf("ctrl reg after = %d %x \n",ctrlreg,ctrlreg);
  LOG4CXX_INFO(_logLdaq," Gain changed with "<<difid<<" ctr "<<ctrlreg1<<" gain "<<gain);
  //response["STATUS"]="TRY";
  //return;

  if (gain==0|| ctrlreg1==0)
    {
      LOG4CXX_ERROR(_logLdaq," Invalid parameters dif "<<difid<<" ctr "<<ctrlreg1<<" Gain "<<gain)
      response["STATUS"]="Invalid params ";
      return;
    }
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  Json::Value array_slc;
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");

	  response["STATUS"]="DIFID not found ";
	  return;

	}
      itd->second->setGain(gain);
      itd->second->configure(ctrlreg1);
      Json::Value ds;
      ds["id"]=itd->first;
      ds["slc"]=itd->second->status()->slc;
      array_slc.append(ds);
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->setGain(gain);
	  it->second->configure(ctrlreg1);
	  Json::Value ds;
	  ds["id"]=it->first;
	  ds["slc"]=it->second->status()->slc;
	  array_slc.append(ds);


	}
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;
      return;
    }

}

void lydaq::LDIFServer::setThreshold(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t difid=atoi(request.get("difid","0").c_str());
  uint32_t B0=atoi(request.get("B0","0").c_str());
  uint32_t B1=atoi(request.get("B1","0").c_str());
  uint32_t B2=atoi(request.get("B2","0").c_str());
  uint32_t ctrlreg1;
  sscanf(request.get("CTRLREG","0").c_str(),"%x",&ctrlreg1);
  printf("ctrl reg = %u %x \n",ctrlreg1,ctrlreg1);
  Json::Value jt;jt.clear();

  jt["ctrlreg"]=request.get("CTRLREG","0");
  std::cout<<" Json value "<<jt<<std::endl;
  // uint32_t ctrlreg= jt["ctrlreg"].asUInt();
  //printf("ctrl reg after = %d %x \n",ctrlreg,ctrlreg);
  LOG4CXX_INFO(_logLdaq," Threshold changed with "<<difid<<" ctr "<<ctrlreg1<<" B0 "<<B0<<" B1 "<<B1<<" B2 "<<B2<<" et "<<request.get("CTRLREG","0").c_str());
  //response["STATUS"]="TRY";
  //return;

  if (B0==0 || B1==0 || B2==0|| ctrlreg1==0)
    {
      LOG4CXX_ERROR(_logLdaq," Invalid parameters dif "<<difid<<" ctr "<<ctrlreg1<<" B0 "<<B0<<" B1 "<<B1<<" B2 "<<B2);
      response["STATUS"]="Invalid params ";
      return;
    }
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  Json::Value array_slc;
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");

	  response["STATUS"]="DIFID not found ";
	  return;

	}
      itd->second->setThreshold(B0,B1,B2);
      itd->second->configure(ctrlreg1);
      Json::Value ds;
      ds["id"]=itd->first;
      ds["slc"]=itd->second->status()->slc;
      array_slc.append(ds);
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->setThreshold(B0,B1,B2);
	  it->second->configure(ctrlreg1);
	  Json::Value ds;
	  ds["id"]=it->first;
	  ds["slc"]=it->second->status()->slc;
	  array_slc.append(ds);


	}
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;
      return;
    }
}


void lydaq::LDIFServer::cmdStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  uint32_t difid=atoi(request.get("difid","0").c_str());
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  Json::Value array_slc;
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  response["STATUS"]="DIFID not found ";
	  return;
	}
      

      Json::Value ds;
      ds["detid"]=itd->second->detectorId();
      ds["state"]=itd->second->state();
      ds["id"]=itd->second->status()->id;
      ds["status"]=itd->second->status()->status;
      ds["slc"]=itd->second->status()->slc;
      ds["gtc"]=itd->second->status()->gtc;
      ds["bcid"]=(Json::Value::UInt64)itd->second->status()->bcid;
      ds["bytes"]=(Json::Value::UInt64)itd->second->status()->bytes;
      ds["host"]=itd->second->status()->host;
      array_slc.append(ds);
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{

	  Json::Value ds;
	  ds["detid"]=it->second->detectorId();
	  ds["state"]=it->second->state();
	  ds["id"]=it->second->status()->id;
	  ds["status"]=it->second->status()->status;
	  ds["slc"]=it->second->status()->slc;
	  ds["gtc"]=it->second->status()->gtc;
	  ds["bcid"]=(Json::Value::UInt64) it->second->status()->bcid;
	  ds["bytes"]=(Json::Value::UInt64)it->second->status()->bytes;
	  ds["host"]=it->second->status()->host;
	  array_slc.append(ds);



	}
      response["STATUS"]="DONE";
      response["DIFLIST"]=array_slc;


      return;
    }
}

void lydaq::LDIFServer::status(zdaq::fsmmessage* m)
{

  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  
  uint32_t difid=m->content()["difid"].asInt();
  
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  Json::Value array_slc;
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}
      

      Json::Value ds;
      ds["detid"]=itd->second->detectorId();
      ds["state"]=itd->second->state();
      ds["id"]=itd->second->status()->id;
      ds["status"]=itd->second->status()->status;
      ds["slc"]=itd->second->status()->slc;
      ds["gtc"]=itd->second->status()->gtc;
      ds["bcid"]=(Json::Value::UInt64)itd->second->status()->bcid;
      ds["bytes"]=(Json::Value::UInt64)itd->second->status()->bytes;
      ds["host"]=itd->second->status()->host;
      array_slc.append(ds);
      m->setAnswer(array_slc);
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{

	  Json::Value ds;
	  ds["detid"]=it->second->detectorId();
	  ds["state"]=it->second->state();
	  ds["id"]=it->second->status()->id;
	  ds["status"]=it->second->status()->status;
	  ds["slc"]=it->second->status()->slc;
	  ds["gtc"]=it->second->status()->gtc;
	  ds["bcid"]=(Json::Value::UInt64) it->second->status()->bcid;
	  ds["bytes"]=(Json::Value::UInt64)it->second->status()->bytes;
	  ds["host"]=it->second->status()->host;
	  array_slc.append(ds);



	}
      m->setAnswer(array_slc);
      return;
    }
}


void lydaq::LDIFServer::configure(zdaq::fsmmessage* m)
{

  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();
  uint32_t ctrlreg=m->content()["ctrlreg"].asUInt();
  LOG4CXX_INFO(_logLdaq," Configuring with "<<difid<<" ctr "<<ctrlreg<<" cont "<<m->content());
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  Json::Value array_slc;
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}
      
      itd->second->configure(ctrlreg);
      Json::Value ds;
      ds["id"]=itd->first;
      ds["slc"]=itd->second->status()->slc;
      array_slc.append(ds);
      m->setAnswer(array_slc);
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->configure(ctrlreg);
	  Json::Value ds;
	  ds["id"]=it->first;
	  ds["slc"]=it->second->status()->slc;
	  array_slc.append(ds);


	}
      m->setAnswer(array_slc);
      return;
    }
}

void lydaq::LDIFServer::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}
      this->startDIFThread(itd->second);
      itd->second->start();
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  this->startDIFThread(it->second);
	  it->second->start();
	}

      return;
    }
}
void lydaq::LDIFServer::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");
	  rc=-1;
	  return;
	}

      itd->second->stop();
     
      return;
    }
  else
    {

      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  printf("Stopping thread of DIF %d \n",it->first);
	  it->second->stop();
	}
      
      return;
    }
}
void lydaq::LDIFServer::destroy(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  uint32_t difid=m->content()["difid"].asInt();
  int32_t rc=1;
  std::map<uint32_t,LDIF*> dm=this->getDIFMap();
  if (difid>0 )
    {
      std::map<uint32_t,LDIF*>::iterator itd=dm.find(difid);
      if (itd==dm.end())
	{
	  LOG4CXX_ERROR(_logLdaq," please do Scan devices first the dif  "<<difid<<"is not registered");

	  return;
	}

      itd->second->destroy();

      return;
    }
  else
    {

      bool running=false;
      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  running=running ||it->second->readoutStarted();
	}
      if (running)
	{
	  for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	    {
	      it->second->setReadoutStarted(false);
	    }

	  this->joinThreads();
	}
      for ( std::map<uint32_t,LDIF*>::iterator it=dm.begin();it!=dm.end();it++)
	{
	  it->second->destroy();
	}


      return;
    }
}






lydaq::LDIFServer::LDIFServer(std::string name)  : zdaq::baseApplication(name)
{

  //_fsm=new zdaq::fsm(name);
  _fsm=this->fsm();
  // Zmq transport
  _context = new zmq::context_t (1);
  // Register state
  _fsm->addState("SCANNED");
  _fsm->addState("INITIALISED");
  _fsm->addState("DBREGISTERED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  _fsm->addState("STOPPED");
  _fsm->addState("THRESHOLDSET");
  _fsm->addState("GAINSET");
  _fsm->addTransition("SCAN","CREATED","SCANNED",boost::bind(&lydaq::LDIFServer::scan, this,_1));
  _fsm->addTransition("INITIALISE","SCANNED","INITIALISED",boost::bind(&lydaq::LDIFServer::initialise, this,_1));
  _fsm->addTransition("REGISTERDB","INITIALISED","DBREGISTERED",boost::bind(&lydaq::LDIFServer::registerdb, this,_1));
  _fsm->addTransition("REGISTERDB","DBREGISTERED","DBREGISTERED",boost::bind(&lydaq::LDIFServer::registerdb, this,_1));
  _fsm->addTransition("REGISTERDB","CONFIGURED","DBREGISTERED",boost::bind(&lydaq::LDIFServer::registerdb, this,_1));
  _fsm->addTransition("CONFIGURE","DBREGISTERED","CONFIGURED",boost::bind(&lydaq::LDIFServer::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::LDIFServer::configure, this,_1));
  _fsm->addTransition("CONFIGURE","STOPPED","CONFIGURED",boost::bind(&lydaq::LDIFServer::configure, this,_1));
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::LDIFServer::start, this,_1));
  _fsm->addTransition("START","STOPPED","RUNNING",boost::bind(&lydaq::LDIFServer::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","STOPPED",boost::bind(&lydaq::LDIFServer::stop, this,_1));
  _fsm->addTransition("DESTROY","STOPPED","CREATED",boost::bind(&lydaq::LDIFServer::destroy, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::LDIFServer::destroy, this,_1));

  _fsm->addTransition("STATUS","SCANNED","SCANNED",boost::bind(&lydaq::LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","INITIALISED","INITIALISED",boost::bind(&lydaq::LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","DBREGISTERED","DBREGISTERED",boost::bind(&lydaq::LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","CONFIGURED","CONFIGURED",boost::bind(&lydaq::LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","RUNNING","RUNNING",boost::bind(&lydaq::LDIFServer::status, this,_1));
  _fsm->addTransition("STATUS","STOPPED","STOPPED",boost::bind(&lydaq::LDIFServer::status, this,_1));

  _fsm->addCommand("STATUS",boost::bind(&lydaq::LDIFServer::cmdStatus,this,_1,_2));
  _fsm->addCommand("SETTHRESHOLD",boost::bind(&lydaq::LDIFServer::setThreshold,this,_1,_2));
  _fsm->addCommand("SETGAIN",boost::bind(&lydaq::LDIFServer::setGain,this,_1,_2));



  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    _fsm->start(atoi(wp));
    }
}







void lydaq::LDIFServer::prepareDevices()
{
  for ( std::map<uint32_t,FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    if (it->second!=NULL) delete it->second;
  theFtdiDeviceInfoMap_.clear();
  for ( std::map<uint32_t,LDIF*>::iterator it=theDIFMap_.begin();it!=theDIFMap_.end();it++)
    if (it->second!=NULL) delete it->second;
  theDIFMap_.clear();
  system("/bin/rm /var/log/pi/ftdi_devices");
  system("/opt/dhcal/bin/ListDevices.py");
  std::string line;
  std::ifstream myfile ("/var/log/pi/ftdi_devices");
  std::stringstream diflist;



  if (myfile.is_open())
    {
      while ( myfile.good() )
	{
	  getline (myfile,line);
	  FtdiDeviceInfo* difi=new FtdiDeviceInfo();
	  memset(difi,0,sizeof(FtdiDeviceInfo));
	  sscanf(line.c_str(),"%x %x %s",&difi->vendorid,&difi->productid,difi->name);
	  if (strncmp(difi->name,"FT101",5)==0)
	    {
	      sscanf(difi->name,"FT101%d",&difi->id); 
	      difi->type=0;
	      std::pair<uint32_t,FtdiDeviceInfo*> p(difi->id,difi);
	      theFtdiDeviceInfoMap_.insert(p);
	    }
	  if (strncmp(difi->name,"DCCCCC",6)==0)
	    {sscanf(difi->name,"DCCCCC%d",&difi->id);difi->type=0x10;}


	}
      myfile.close();
    }
  else 
    {
      std::cout << "Unable to open file"<<std::endl; 
      LOG4CXX_FATAL(_logLdaq," Unable to open /var/log/pi/ftdi_devices");
    }

  for (std::map<uint32_t,FtdiDeviceInfo*>::iterator it=theFtdiDeviceInfoMap_.begin();it!=theFtdiDeviceInfoMap_.end();it++)
    printf("Device found and register: %d with info %d %d %s type %d \n", it->first,it->second->vendorid,it->second->productid,it->second->name,it->second->type);
}



void lydaq::LDIFServer::startDIFThread(LDIF* d)
{
  if (d->readoutStarted()) return;
  d->setReadoutStarted(true);	

  g_d.create_thread(boost::bind(&LDIF::readout,d));
  
}



