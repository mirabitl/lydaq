#include "Febv1Manager.hh"
using namespace mpi;
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


lydaq::Febv1Manager::Febv1Manager(std::string name) : zdaq::baseApplication(name),_context(NULL),_tca(NULL),_mpi(NULL)
{
  _fsm=this->fsm();
  // Register state

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");
  
  _fsm->addTransition("INITIALISE","CREATED","INITIALISED",boost::bind(&lydaq::Febv1Manager::initialise, this,_1));
  _fsm->addTransition("CONFIGURE","INITIALISED","CONFIGURED",boost::bind(&lydaq::Febv1Manager::configure, this,_1));
  _fsm->addTransition("CONFIGURE","CONFIGURED","CONFIGURED",boost::bind(&lydaq::Febv1Manager::configure, this,_1));
  
  _fsm->addTransition("START","CONFIGURED","RUNNING",boost::bind(&lydaq::Febv1Manager::start, this,_1));
  _fsm->addTransition("STOP","RUNNING","CONFIGURED",boost::bind(&lydaq::Febv1Manager::stop, this,_1));
  _fsm->addTransition("DESTROY","CONFIGURED","CREATED",boost::bind(&lydaq::Febv1Manager::destroy, this,_1));
  _fsm->addTransition("DESTROY","INITIALISED","CREATED",boost::bind(&lydaq::Febv1Manager::destroy, this,_1));
  
  

  _fsm->addCommand("STATUS", boost::bind(&lydaq::Febv1Manager::c_status, this, _1, _2));
  _fsm->addCommand("DIFLIST", boost::bind(&lydaq::Febv1Manager::c_diflist, this, _1, _2));
  _fsm->addCommand("SET6BDAC", boost::bind(&lydaq::Febv1Manager::c_set6bdac, this, _1, _2));
  _fsm->addCommand("CAL6BDAC", boost::bind(&lydaq::Febv1Manager::c_cal6bdac, this, _1, _2));
  _fsm->addCommand("SETVTHTIME", boost::bind(&lydaq::Febv1Manager::c_setvthtime, this, _1, _2));
  _fsm->addCommand("SETONEVTHTIME", boost::bind(&lydaq::Febv1Manager::c_set1vthtime, this, _1, _2));
  _fsm->addCommand("SETMASK", boost::bind(&lydaq::Febv1Manager::c_setMask, this, _1, _2));
  _fsm->addCommand("DOWNLOADDB", boost::bind(&lydaq::Febv1Manager::c_downloadDB, this, _1, _2));
  _fsm->addCommand("ASICS", boost::bind(&lydaq::Febv1Manager::c_asics, this, _1, _2));

  _fsm->addCommand("SETMODE", boost::bind(&lydaq::Febv1Manager::c_setMode, this, _1, _2));
  _fsm->addCommand("SETDELAY", boost::bind(&lydaq::Febv1Manager::c_setDelay, this, _1, _2));
  _fsm->addCommand("SETDURATION", boost::bind(&lydaq::Febv1Manager::c_setDuration, this, _1, _2));
  _fsm->addCommand("GETLUT", boost::bind(&lydaq::Febv1Manager::c_getLUT, this, _1, _2));
  _fsm->addCommand("CALIBSTATUS", boost::bind(&lydaq::Febv1Manager::c_getCalibrationStatus, this, _1, _2));
  _fsm->addCommand("CALIBMASK", boost::bind(&lydaq::Febv1Manager::c_setCalibrationMask, this, _1, _2));
  _fsm->addCommand("TESTMASK", boost::bind(&lydaq::Febv1Manager::c_setMeasurementMask, this, _1, _2));
  _fsm->addCommand("SCURVE", boost::bind(&lydaq::Febv1Manager::c_scurve, this, _1, _2));

  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;
 
  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Service "<<name<<" is starting on "<<atoi(wp));

      
      _fsm->start(atoi(wp));
    }
    
  
 
  // Initialise NetLink


}
void lydaq::Febv1Manager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb, "Status CMD called ");
  response["STATUS"] = "DONE";

  Json::Value jl;
  for (auto x : _mpi->boards())
  {

    Json::Value jt;
    jt["detid"] = x.second->data()->detectorId();
    jt["sourceid"] = x.second->data()->difId();
    jt["gtc"] = x.second->data()->gtc();
    jt["abcid"] = (Json::Value::UInt64)x.second->data()->abcid();
    jt["event"] = x.second->data()->event();
    jt["triggers"] = x.second->data()->triggers();
    jl.append(jt);
  }
  response["TDCSTATUS"] = jl;
}
void lydaq::Febv1Manager::c_diflist(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb, "List of source id CMD called ");
  response["STATUS"] = "DONE";
  response["DIFLIST"] = "EMPTY";

  Json::Value jl;
  for (auto x : _mpi->boards())
  {
    if (x.second == NULL)
      continue;
    Json::Value jt;
    jt["detid"] = x.second->data()->detectorId();
    jt["sourceid"] = x.second->data()->difId();
    jl.append(jt);
  }
  response["DIFLIST"] = jl;
}

void lydaq::Febv1Manager::c_set6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t nc = atol(request.get("value", "31").c_str());
  LOG4CXX_INFO(_logFeb, "Set6bdac called with dac=" << nc);

  this->set6bDac(nc & 0xFF);
  response["6BDAC"] = _jControl;
}
void lydaq::Febv1Manager::c_cal6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t mask = atol(request.get("mask", "4294967295").c_str());
  int32_t shift = atol(request.get("shift", "0").c_str());
  LOG4CXX_INFO(_logFeb, "cal6bdac called with mask=" << mask << " Shift:" << shift);

  this->cal6bDac(mask, shift);
  response["6BDAC"] = _jControl;
}
void lydaq::Febv1Manager::c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t nc = atol(request.get("value", "380").c_str());

  LOG4CXX_INFO(_logFeb, "set VThTime called with value=" << nc);

  this->setVthTime(nc);
  response["VTHTIME"] = _jControl;
}
void lydaq::Febv1Manager::c_set1vthtime(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t vth = atol(request.get("vth", "550").c_str());
  uint32_t feb = atol(request.get("feb", "5").c_str());
  uint32_t asic = atol(request.get("asic", "1").c_str());
  LOG4CXX_INFO(_logFeb, " SetOneVthTime called with vth " << vth << " feb " << feb << " asic " << asic);
  this->setSingleVthTime(vth, feb, asic);
  response["VTH"] = vth;
  response["FEB"] = feb;
  response["ASIC"] = asic;
  response["1VTH"] = _jControl;
}
void lydaq::Febv1Manager::c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb, __PRETTY_FUNCTION__ << "SetMask called ");
  response["STATUS"] = "DONE";

  //uint32_t nc=atol(request.get("value","4294967295").c_str());
  uint32_t nc;
  sscanf(request.get("value", "4294967295").c_str(), "%u", &nc);

  uint32_t asic = atol(request.get("asic", "255").c_str());

  LOG4CXX_INFO(_logFeb, "SetMask called  with mask" << std::hex << nc << std::dec << " and asic mask " << asic);
  this->setMask(nc, asic & 0xFF);
  response["MASK"] = _jControl;
}

void lydaq::Febv1Manager::c_setCalibrationMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb, __PRETTY_FUNCTION__ << "SetCalibrationMask called ");
  response["STATUS"] = "DONE";
  uint64_t mask = 0;
  sscanf(request.get("value", "0x0").c_str(), "%llx", &mask);
  LOG4CXX_INFO(_logFeb, "SetCalibrationMask called  with mask" << std::hex << mask << std::dec);
  this->setCalibrationMask(mask);
  response["CMASK"] = (Json::Value::UInt64)mask;
}
void lydaq::Febv1Manager::c_setMeasurementMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb, __PRETTY_FUNCTION__ << "c_setMeasurementMask called ");
  response["STATUS"] = "DONE";
  uint64_t mask = 0;
  sscanf(request.get("value", "0x0").c_str(), "%llx", &mask);
  uint32_t feb = atol(request.get("feb", "255").c_str());
  LOG4CXX_INFO(_logFeb, "c_setMeasurementMask called  with mask" << std::hex << mask << std::dec<<" On FEB "<<feb);
  this->setMeasurementMask(mask,feb);
  response["MMASK"] = (Json::Value::UInt64)mask;
}
void lydaq::Febv1Manager::c_setDelay(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t delay = atol(request.get("value", "255").c_str());
  _delay = delay;
  this->setDelay();
  LOG4CXX_INFO(_logFeb, "SetDelay called with " << delay << " " << _delay);
  response["DELAY"] = _delay;
}
void lydaq::Febv1Manager::c_getLUT(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t chan = atol(request.get("value", "0").c_str());
  this->getLUT(chan);
  LOG4CXX_INFO(_logFeb, "GETLUT called for " << chan);
  response["LUT"] = _jControl;
}
void lydaq::Febv1Manager::c_getCalibrationStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  this->getCalibrationStatus();
  LOG4CXX_INFO(_logFeb, "GetCalibrationStatus called ");
  response["CALIBRATION"] = _jControl;
}
void lydaq::Febv1Manager::c_setDuration(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t duration = atol(request.get("value", "255").c_str());
  _duration = duration;
  this->setDuration();
  LOG4CXX_INFO(_logFeb, "Setduration called with " << duration << " " << _duration);
  response["DURATION"] = _duration;
}

void lydaq::Febv1Manager::c_setMode(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t mode = atol(request.get("value", "2").c_str());
  if (mode != 2)
    _type = mode;
  LOG4CXX_INFO(_logFeb, "SetMode called with Mode " << mode << "  Type " << _type);
  response["MODE"] = _type;
}
void lydaq::Febv1Manager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"downloadDB called ");
  response["STATUS"]="DONE";


  
  std::string dbstate=request.get("state","NONE");
  uint32_t version=atol(request.get("version","0").c_str());
  Json::Value jTDC=this->parameters()["febv1"];
  if (jTDC.isMember("db"))
    {
      Json::Value jTDCdb=jTDC["db"];
      _tca->clear();

      if (jTDCdb["mode"].asString().compare("mongo")!=0)
	_tca->parseDb(dbstate,jTDCdb["mode"].asString());
      else
	_tca->parseMongoDb(dbstate,version);

	 
    }
  response["DBSTATE"]=dbstate;
}



void lydaq::Febv1Manager::c_scurve(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t first = atol(request.get("first", "420").c_str());
  uint32_t last = atol(request.get("last", "520").c_str());
  uint32_t step = atol(request.get("step", "2").c_str());
  uint32_t mode = atol(request.get("channel", "255").c_str());
  //  LOG4CXX_INFO(_logFeb, " SetOneVthTime called with vth " << vth << " feb " << feb << " asic " << asic);
  
  //this->Scurve(mode,first,last,step);

  _sc_mode=mode;
  _sc_thmin=first;
  _sc_thmax=last;
  _sc_step=step;
  if (_sc_running)
    {
      response["SCURVE"] ="ALREADY_RUNNING";
      return;
    }
  boost::thread_group g;
  g.create_thread(boost::bind(&lydaq::Febv1Manager::thrd_scurve, this));
  response["SCURVE"] ="RUNNING";
}

void lydaq::Febv1Manager::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"****** CMD: "<<m->command());
  //  std::cout<<"m= "<<m->command()<<std::endl<<m->content()<<std::endl;
 
  Json::Value jtype=this->parameters()["type"];
  _type=jtype.asInt();
  printf ("_type =%d\n",_type); 

  // Need a FEBV1 tag
  if (m->content().isMember("febv1"))
    {
      printf ("found febv1/n");
      this->parameters()["febv1"]=m->content()["febv1"];
    }
  if (!this->parameters().isMember("febv1"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No febv1 tag found ");
      return;
    }
  // Now create the Message handler
  if (_mpi==NULL)
    _mpi= new lydaq::febv1::Interface();
  _mpi->initialise();

   
  Json::Value jFEBV1=this->parameters()["febv1"];
  //_msh =new lydaq::MpiMessageHandler("/dev/shm");
  if (!jFEBV1.isMember("network"))
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No febv1:network tag found ");
      return;
    }
  // Scan the network
  std::map<uint32_t,std::string> diflist=mpi::MpiMessageHandler::scanNetwork(jFEBV1["network"].asString());
  // Download the configuration
  if (_tca==NULL)
    {
      std::cout<< "Create config acccess"<<std::endl;
      _tca=new lydaq::TdcConfigAccess();
      _tca->clear();
    }
  std::cout<< " jFEBV1 "<<jFEBV1<<std::endl;
  if (jFEBV1.isMember("json"))
    {
      Json::Value jFEBV1json=jFEBV1["json"];
      if (jFEBV1json.isMember("file"))
	{
	  _tca->parseJsonFile(jFEBV1json["file"].asString());
	}
      else
	if (jFEBV1json.isMember("url"))
	  {
	    _tca->parseJsonUrl(jFEBV1json["url"].asString());
	  }
    }
  if (jFEBV1.isMember("db"))
    {
      Json::Value jFEBV1db=jFEBV1["db"];
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<"Parsing:"<<jFEBV1db["state"].asString()<<jFEBV1db["mode"].asString());

              
      if (jFEBV1db["mode"].asString().compare("mongo")!=0)	
	_tca->parseDb(jFEBV1db["state"].asString(),jFEBV1db["mode"].asString());
      else
	_tca->parseMongoDb(jFEBV1db["state"].asString(),jFEBV1db["version"].asUInt());
      
    }
  if (_tca->asicMap().size()==0)
    {
      LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" No ASIC found in the configuration ");
      return;
    }
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"ASIC found in the configuration "<<_tca->asicMap().size() );
  // Initialise the network
  std::vector<uint32_t> vint;
  vint.clear();
  for (auto x:_tca->asicMap())
    {
      uint32_t eip= ((x.first)>>32)&0XFFFFFFFF;
      std::map<uint32_t,std::string>::iterator idif=diflist.find(eip);
      if (idif==diflist.end()) continue;
      if ( std::find(vint.begin(), vint.end(), eip) != vint.end() ) continue;
      
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" New FEBV1 found in db "<<std::hex<<eip<<std::dec<<" IP address "<<idif->second);
      vint.push_back(eip);
      _mpi->addDevice(idif->second);
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Registration done for "<<eip);
    }
  //std::string network=
  // Connect to the event builder
  if (_context==NULL)
    _context= new zmq::context_t(1);

  if (m->content().isMember("publish"))
    {
      this->parameters()["publish"]=m->content()["publish"];
    }

  
  for (auto x:_mpi->boards())
    x.second->data()->autoRegister(_context,this->configuration(),"BUILDER","collectingPort");
  //x->connect(_context,this->parameters()["publish"].asString());

  // Listen All Febv1 sockets
  _mpi->listen();

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Init done  "); 
}

void lydaq::Febv1Manager::configurePR2()
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" COnfigure the chips ");

  
  fprintf(stderr,"Loop on socket for Sending slow control \n");
  for (auto x:_mpi->boards())
    {
      _tca->prepareSlowControl(x.second->ipAddress());
      x.second->reg()->writeRam(_tca->slcAddr(), _tca->slcBuffer(), _tca->slcBytes());
      x.second->reg()->dumpAnswer(0);
    }
}
void lydaq::Febv1Manager::configure(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());

  // Now loop on slowcontrol socket


  this->configurePR2();

}

void lydaq::Febv1Manager::set6bDac(uint8_t dac)
{

  //::sleep(1);

  // Modify ASIC SLC
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    for (int i = 0; i < 32; i++)
    {
      it->second.set6bDac(i, dac);
    }
  }
  // Now loop on slowcontrol socket and send packet
  this->configurePR2();
}
void lydaq::Febv1Manager::cal6bDac(uint32_t mask, int32_t dacShift)
{
  LOG4CXX_INFO(_logFeb, "CAL6BDAC called " << mask << " Shift " << dacShift);
  //::usleep(50000);
  std::map<uint64_t, uint16_t *> ascopy;
  // Modify ASIC SLC
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {

    if (ascopy.find(it->first) == ascopy.end())
    {
      uint16_t *b = new uint16_t[32];
      std::pair<uint64_t, uint16_t *> p(it->first, b);
      ascopy.insert(p);
    }
    auto ic = ascopy.find(it->first);
    for (int i = 0; i < 32; i++)
    {
      ic->second[i] = it->second.get6bDac(i);
      if ((mask >> i) & 1)
      {
        uint32_t dac = it->second.get6bDac(i);
        int32_t ndac = dac + dacShift;
        if (ndac < 0)
          ndac = 0;
        if (ndac > 63)
          ndac = 63;
        std::cout << "channel " << i << " DAC " << dac << " shifted to " << ndac << std::endl;

        it->second.set6bDac(i, ndac);
      }
    }
  }
  // Now loop on slowcontrol socket and send packet
  this->configurePR2();

  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {

    auto ic = ascopy.find(it->first);
    for (int i = 0; i < 32; i++)

      it->second.set6bDac(i, ic->second[i]);
  }
  for (auto it = ascopy.begin(); it != ascopy.end(); it++)
    delete it->second;

 
}
void lydaq::Febv1Manager::c_asics(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";
  Json::Value jlist;
  ;
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    Json::Value jasic;
    uint32_t iasic = it->first & 0xFF;
    jasic["num"] = iasic;
    uint32_t difid = ((it->first) >> 32 & 0xFFFFFFFF);
    jasic["dif"] = difid;
    it->second.toJson();
    jasic["slc"] = it->second.getJson();
    jlist.append(jasic);
  }
  response["asics"] = jlist;
}

void lydaq::Febv1Manager::setMask(uint32_t mask, uint8_t asic)
{

  //::sleep(1);
  // Change all Asics VthTime
  uint32_t umask;
  uint32_t asica = asic;
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    uint32_t iasic = it->first & 0xFF;
    fprintf(stderr, "ASIC in map %d ASIC asked %d \n", iasic, asica);
    if ((iasic & asica) == 0)
    {
      fprintf(stderr, "Skipping asic %d by masking all channels\n", iasic);
      umask = 0;
    }
    else
      umask = mask;

    for (int i = 0; i < 32; i++)
    {
      if ((umask >> i) & 1)
      {
        it->second.setMaskDiscriTime(i, 0);
      }
      else
      {
        it->second.setMaskDiscriTime(i, 1);
      }
    }
    //std::cout << "ASIC " << (int)iasic << "==========================" << std::endl;
    //it->second.Print();
  }

  // Now loop on slowcontrol socket
  this->configurePR2();
}

void lydaq::Febv1Manager::setVthTime(uint32_t vth)
{

  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Debut ");
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    int iasic = it->first & 0xFF;

    it->second.setVthTime(vth);
    //it->second.Print();
    // 1 seul ASIC break;
  }
  this->configurePR2();
 
  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Fin ");
}

void lydaq::Febv1Manager::setSingleVthTime(uint32_t vth, uint32_t feb, uint32_t asic)
{
  // Encode IP
  std::stringstream ip;
  ip << "192.168.10." << feb;

  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Debut ");
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    //  Change VTH time only on specified ASIC
    uint64_t eid = (((uint64_t) mpi::MpiMessageHandler::convertIP(ip.str())) << 32) | asic;
    if (eid != it->first)
      continue;
    it->second.setVthTime(vth);
  }
  this->configurePR2();

  
  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Fin ");
}

void lydaq::Febv1Manager::setDelay()
{
  LOG4CXX_INFO(_logFeb, "Setting active time " << (int) _delay);
  for (auto x : _mpi->boards())
    x.second->reg()->writeAddress(0x222, _delay);
 
}
void lydaq::Febv1Manager::setDuration()
{
  LOG4CXX_INFO(_logFeb, " Setting Dead time duration " << (int) _duration);
  for (auto x : _mpi->boards())
    x.second->reg()->writeAddress(0x223, _duration);
 
}
void lydaq::Febv1Manager::getLUT(int chan)
{
  LOG4CXX_INFO(_logFeb, " get LUT for " << chan << " on all FEBS");
  for (auto x : _mpi->boards())
    {
      x.second->reg()->writeAddress(0x224,chan,true);
      x.second->reg()->dumpAnswer(0);
    }
}
void lydaq::Febv1Manager::getCalibrationStatus()
{
  LOG4CXX_INFO(_logFeb, " get Calibration Status for on all FEBS");
  for (auto x : _mpi->boards())
    {
      x.second->reg()->writeAddress(0x225,0,true);
      x.second->reg()->dumpAnswer(0);
    }
}
void lydaq::Febv1Manager::setCalibrationMask(uint64_t mask)
{
  LOG4CXX_INFO(_logFeb, " setCalibrationMask " << std::hex << mask << std::dec << " on all FEBS");
  for (auto x : _mpi->boards())
    x.second->reg()->writeLongWord(0x226, mask);
}
void lydaq::Febv1Manager::setMeasurementMask(uint64_t mask,uint32_t feb)
{
  LOG4CXX_INFO(_logFeb, " setMeasurementMask " << std::hex << mask << std::dec << " on  FEBS"<<feb);
  

  
  for (auto x :  _mpi->boards())
  {
    if (feb!=255)
      {
	std::stringstream ip;
	ip <<this->parameters()["febv1"]["network"].asString()<< feb;
	if (ip.str().compare(x.second->ipAddress())!=0)
	  {
	    LOG4CXX_INFO(_logFeb, " setMeasurementMask " <<x.second->ipAddress()<< " skipped for FEB "<<feb);
	    continue;
	  }
      }
     x.second->reg()->writeLongWord(0x230, mask);

  }
}


/////////////////////////////////////////////////////////
void lydaq::Febv1Manager::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  // Create run file
  Json::Value jc=m->content();
  _run=jc["run"].asInt();

  // Clear buffers
  for (auto x:_mpi->boards())
    {
      x.second->data()->clear();
    }

  // Turn run type on
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , enabled (Bit 0 a 1)
      x.second->reg()->writeAddress(0x219,_type);
      x.second->reg()->writeAddress(0x220,1);
    }
}
void lydaq::Febv1Manager::stop(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  for (auto x:_mpi->boards())
    {
      // Automatic FSM (bit 1 a 0) , disabled (Bit 0 a 0)
      x.second->reg()->writeAddress(0x220,0);
    }
  ::sleep(2);


}
void lydaq::Febv1Manager::destroy(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" CMD: "<<m->command());
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"CLOSE called ");
  
  _mpi->close();
  for (auto x:_mpi->boards())
    delete x.second;
  _mpi->boards().clear();
  delete _mpi;
  _mpi=0;

  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Data sockets deleted");



  // To be done: _febv1->clear();
}






void lydaq::Febv1Manager::ScurveStep(fsmwebCaller* mdcc,fsmwebCaller* builder,int thmin,int thmax,int step)
{

  int ncon=2000,ncoff=100,ntrg=50;
  mdcc->sendCommand("PAUSE");
  Json::Value p;
  p.clear();p["nclock"]=ncon;  mdcc->sendCommand("SPILLON",p);
  p.clear();p["nclock"]=ncoff;  mdcc->sendCommand("SPILLOFF",p);
  printf("Clock On %d Off %d \n",ncon, ncoff);
  p.clear();p["value"]=4;  mdcc->sendCommand("SETSPILLREGISTER",p);
  mdcc->sendCommand("CALIBON");
  p.clear();p["nclock"]=ntrg;  mdcc->sendCommand("SETCALIBCOUNT",p);
  int thrange=(thmax-thmin+1)/step;
  for (int vth=0;vth<=thrange;vth++)
    {
      if (!_running) break;
      mdcc->sendCommand("PAUSE");
      this->setVthTime(thmax-vth*step);
      p.clear();
      Json::Value h;
      h.append(2);h.append(thmax-vth*step);
      p["header"]=h;builder->sendCommand("SETHEADER",p);
      int firstEvent=0;
      for (auto x : _mpi->boards())
	if (x.second->data()->event()>firstEvent) firstEvent=x.second->data()->event();
      mdcc->sendCommand("RELOADCALIB");
      mdcc->sendCommand("RESUME");
      int nloop=0,lastEvent=firstEvent;
      while (lastEvent < (firstEvent + ntrg - 20))
	{
	  ::usleep(100000);
	  for (auto x : _mpi->boards())
	    if (x.second->data()->event()>lastEvent) lastEvent=x.second->data()->event();
	  nloop++;if (nloop > 20 || !_running)  break;
	}
      printf("Step %d Th %d First %d Last %d \n",vth,thmax-vth*step,firstEvent,lastEvent);
      mdcc->sendCommand("PAUSE");
    }
  mdcc->sendCommand("CALIBOFF");
}


void lydaq::Febv1Manager::thrd_scurve()
{
  _sc_running=true;
  this->Scurve(_sc_mode,_sc_thmin,_sc_thmax,_sc_step);
  _sc_running=false;
}


void lydaq::Febv1Manager::Scurve(int mode,int thmin,int thmax,int step)
{
  fsmwebCaller* mdcc=findMDCC("MDCCSERVER");
  fsmwebCaller* builder=findMDCC("BUILDER");
  if (mdcc==NULL) return;
  if (builder==NULL) return;
  int firmware[]={0,2,4,6,
		  8,10,12,14,
		  16,18,20,22,
		  24,26,28,30};

  int mask=0;
  if (mode==255)
    {

      for (int i=0;i<16;i++) mask|=(1<<firmware[i]);
      this->setMask(mask,0xFF);
      this->ScurveStep(mdcc,builder,thmin,thmax,step);
      return;
      
    }
  if (mode==1023)
    {
      int mask=0;
      for (int i=0;i<16;i++)
	{
	  mask=(1<<firmware[i]);
	  std::cout<<"Step PR2 "<<i<<" channel "<<firmware[i]<<std::endl;
	  this->setMask(mask,0xFF);
	  this->ScurveStep(mdcc,builder,thmin,thmax,step);
	}
      return;
    }
  mask=(1<<mode);
  this->setMask(mask,0xFF);
  this->ScurveStep(mdcc,builder,thmin,thmax,step);

  
}

fsmwebCaller* lydaq::Febv1Manager::findMDCC(std::string appname)
{
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
	  if (p_name.compare(appname)==0)
	    {
	      
	      return  new fsmwebCaller(host,port); 
	    }
	}

    }
  
  return NULL;
  
}
