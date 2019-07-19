#include "WiznetManager.hh"
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

lydaq::WiznetManager::WiznetManager(std::string name) : zdaq::baseApplication(name), _context(NULL)
{
  _fsm = this->fsm();
  // Register state machine

  _fsm->addState("INITIALISED");
  _fsm->addState("CONFIGURED");
  _fsm->addState("RUNNING");

  _fsm->addTransition("INITIALISE", "CREATED", "INITIALISED", boost::bind(&lydaq::WiznetManager::initialise, this, _1));
  _fsm->addTransition("CONFIGURE", "INITIALISED", "CONFIGURED", boost::bind(&lydaq::WiznetManager::configure, this, _1));
  _fsm->addTransition("CONFIGURE", "CONFIGURED", "CONFIGURED", boost::bind(&lydaq::WiznetManager::configure, this, _1));

  _fsm->addTransition("START", "CONFIGURED", "RUNNING", boost::bind(&lydaq::WiznetManager::start, this, _1));
  _fsm->addTransition("STOP", "RUNNING", "CONFIGURED", boost::bind(&lydaq::WiznetManager::stop, this, _1));
  _fsm->addTransition("DESTROY", "CONFIGURED", "CREATED", boost::bind(&lydaq::WiznetManager::destroy, this, _1));
  _fsm->addTransition("DESTROY", "INITIALISED", "CREATED", boost::bind(&lydaq::WiznetManager::destroy, this, _1));

  //_fsm->addCommand("JOBLOG",boost::bind(&lydaq::WiznetManager::c_joblog,this,_1,_2));
  _fsm->addCommand("STATUS", boost::bind(&lydaq::WiznetManager::c_status, this, _1, _2));
  _fsm->addCommand("DIFLIST", boost::bind(&lydaq::WiznetManager::c_diflist, this, _1, _2));
  _fsm->addCommand("SET6BDAC", boost::bind(&lydaq::WiznetManager::c_set6bdac, this, _1, _2));
  _fsm->addCommand("SETVTHTIME", boost::bind(&lydaq::WiznetManager::c_setvthtime, this, _1, _2));
  _fsm->addCommand("SETONEVTHTIME", boost::bind(&lydaq::WiznetManager::c_set1vthtime, this, _1, _2));
  _fsm->addCommand("SETMASK", boost::bind(&lydaq::WiznetManager::c_setMask, this, _1, _2));
  _fsm->addCommand("DOWNLOADDB", boost::bind(&lydaq::WiznetManager::c_downloadDB, this, _1, _2));
  _fsm->addCommand("ASICS", boost::bind(&lydaq::WiznetManager::c_asics, this, _1, _2));

  _fsm->addCommand("SETMODE", boost::bind(&lydaq::WiznetManager::c_setMode, this, _1, _2));
  _fsm->addCommand("SETDELAY", boost::bind(&lydaq::WiznetManager::c_setDelay, this, _1, _2));
  _fsm->addCommand("SETDURATION", boost::bind(&lydaq::WiznetManager::c_setDuration, this, _1, _2));
  _fsm->addCommand("GETLUT", boost::bind(&lydaq::WiznetManager::c_getLUT, this, _1, _2));
  _fsm->addCommand("CALIBSTATUS", boost::bind(&lydaq::WiznetManager::c_getCalibrationStatus, this, _1, _2));
  _fsm->addCommand("CALIBMASK", boost::bind(&lydaq::WiznetManager::c_setCalibrationMask, this, _1, _2));
  _fsm->addCommand("TESTMASK", boost::bind(&lydaq::WiznetManager::c_setMeasurementMask, this, _1, _2));

  //std::cout<<"Service "<<name<<" started on port "<<port<<std::endl;

  char *wp = getenv("WEBPORT");
  if (wp != NULL)
  {
    LOG4CXX_INFO(_logFeb, __PRETTY_FUNCTION__ << " Service " << name << " is starting on " << atoi(wp));

    _fsm->start(atoi(wp));
  }

  // Initialise NetLink
  _wiznet = new lydaq::WiznetInterface();
  _msg = new lydaq::WiznetMessage();
}
void lydaq::WiznetManager::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb, "Status CMD called ");
  response["STATUS"] = "DONE";

  Json::Value jl;
  for (auto x : _vTdc)
  {

    Json::Value jt;
    jt["detid"] = x->detectorId();
    jt["sourceid"] = x->difId();
    jt["gtc"] = x->gtc();
    jt["abcid"] = (Json::Value::UInt64)x->abcid();
    jt["event"] = x->event();
    jt["triggers"] = x->triggers();
    jl.append(jt);
  }
  response["TDCSTATUS"] = jl;
}
void lydaq::WiznetManager::c_diflist(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb, "List of source id CMD called ");
  response["STATUS"] = "DONE";
  response["DIFLIST"] = "EMPTY";

  Json::Value jl;
  for (auto x : _vTdc)
  {
    if (x == NULL)
      continue;
    Json::Value jt;
    jt["detid"] = x->detectorId();
    jt["sourceid"] = x->difId();
    jl.append(jt);
  }
  response["DIFLIST"] = jl;
}

void lydaq::WiznetManager::c_set6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t nc = atol(request.get("value", "31").c_str());
  LOG4CXX_INFO(_logFeb, "Set6bdac called with dac=" << nc);

  this->set6bDac(nc & 0xFF);
  response["6BDAC"] = _jControl;
}
void lydaq::WiznetManager::c_cal6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t mask = atol(request.get("mask", "4294967295").c_str());
  int32_t shift = atol(request.get("shift", "0").c_str());
  LOG4CXX_INFO(_logFeb, "cal6bdac called with mask=" << mask<<" Shift:"<<shift);

  this->cal6bDac(mask,shift);
  response["6BDAC"] = _jControl;
}
void lydaq::WiznetManager::c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t nc = atol(request.get("value", "380").c_str());

  LOG4CXX_INFO(_logFeb, "set VThTime called with value=" << nc);

  this->setVthTime(nc);
  response["VTHTIME"] = _jControl;
}
void lydaq::WiznetManager::c_set1vthtime(Mongoose::Request &request, Mongoose::JsonResponse &response)
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
  response["1VTH"]=_jControl;
}
void lydaq::WiznetManager::c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
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

void lydaq::WiznetManager::c_setCalibrationMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb, __PRETTY_FUNCTION__ << "SetCalibrationMask called ");
  response["STATUS"] = "DONE";
  uint64_t mask = 0;
  sscanf(request.get("value", "0x0").c_str(), "%llx", &mask);
  LOG4CXX_INFO(_logFeb, "SetCalibrationMask called  with mask" << std::hex << mask << std::dec);
  this->setCalibrationMask(mask);
  response["CMASK"] = (Json::Value::UInt64)mask;
}
void lydaq::WiznetManager::c_setMeasurementMask(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logFeb, __PRETTY_FUNCTION__ << "c_setMeasurementMask called ");
  response["STATUS"] = "DONE";
  uint64_t mask = 0;
  sscanf(request.get("value", "0x0").c_str(), "%llx", &mask);
  LOG4CXX_INFO(_logFeb, "c_setMeasurementMask called  with mask" << std::hex << mask << std::dec);
  this->setMeasurementMask(mask);
  response["MMASK"] = (Json::Value::UInt64)mask;
}
void lydaq::WiznetManager::c_setDelay(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint8_t delay = atol(request.get("value", "255").c_str());
  _delay = delay;
  this->setDelay();
  LOG4CXX_INFO(_logFeb, "SetDelay called with " << delay << " " << _delay);
  response["DELAY"] =_delay;
}
void lydaq::WiznetManager::c_getLUT(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t chan = atol(request.get("value", "0").c_str());
  this->getLUT(chan);
  LOG4CXX_INFO(_logFeb, "GETLUT called for " << chan);
  response["LUT"] = _jControl;
}
void lydaq::WiznetManager::c_getCalibrationStatus(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  this->getCalibrationStatus();
  LOG4CXX_INFO(_logFeb, "GetCalibrationStatus called ");
  response["CALIBRATION"] = _jControl;
}
void lydaq::WiznetManager::c_setDuration(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint8_t duration = atol(request.get("value", "255").c_str());
  _duration = duration;
  this->setDuration();
  LOG4CXX_INFO(_logFeb, "Setduration called with " << duration << " " << _duration);
  response["DURATION"] = _duration;
}

void lydaq::WiznetManager::c_setMode(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  uint32_t mode = atol(request.get("value", "2").c_str());
  if (mode != 2)
    _type = mode;
  LOG4CXX_INFO(_logFeb, "SetMode called with Mode " << mode << "  Type " << _type);
  response["MODE"] = _type;
}
void lydaq::WiznetManager::c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";

  std::string dbstate = request.get("state", "NONE");
  Json::Value jTDC = this->parameters()["tdc"];
  if (jTDC.isMember("db"))
  {
    Json::Value jTDCdb = jTDC["db"];
    _tca->clear();
    _tca->parseDb(dbstate, jTDCdb["mode"].asString());
  }
  LOG4CXX_INFO(_logFeb, "DownloadDB called  for " << dbstate);

  response["DBSTATE"] = dbstate;
}

void lydaq::WiznetManager::initialise(zdaq::fsmmessage *m)
{
  LOG4CXX_INFO(_logFeb, "INITIALISE transition call");

  Json::Value jtype = this->parameters()["type"];
  _type = jtype.asInt();
  ///printf ("_type =%d\n",_type);

  // Need a TDC tag
  if (m->content().isMember("tdc"))
  {
    this->parameters()["tdc"] = m->content()["tdc"];
  }
  if (!this->parameters().isMember("tdc"))
  {
    LOG4CXX_FATAL(_logFeb, " No tdc tag found ");
    return;
  }
  // Now create the Message handler
  _wiznet->initialise();

  Json::Value jTDC = this->parameters()["tdc"];

  if (!jTDC.isMember("network"))
  {
    LOG4CXX_FATAL(_logFeb, " No tdc:network tag found ");
    return;
  }
  // Scan the network
  std::map<uint32_t, std::string> diflist = lydaq::WiznetMessageHandler::scanNetwork(jTDC["network"].asString());
  // Download the configuration
  _tca = new lydaq::TdcConfigAccess();
  if (jTDC.isMember("json"))
  {
    Json::Value jTDCjson = jTDC["json"];
    if (jTDCjson.isMember("file"))
    {
      _tca->parseJsonFile(jTDCjson["file"].asString());
    }
    else if (jTDCjson.isMember("url"))
    {
      _tca->parseJsonUrl(jTDCjson["url"].asString());
    }
  }
  if (jTDC.isMember("db"))
  {
    Json::Value jTDCdb = jTDC["db"];
    _tca->parseDb(jTDCdb["state"].asString(), jTDCdb["mode"].asString());
  }
  if (_tca->asicMap().size() == 0)
  {
    LOG4CXX_FATAL(_logFeb, " No ASIC found in the configuration ");
    return;
  }
  LOG4CXX_INFO(_logFeb, "ASIC found in the configuration  #=" << _tca->asicMap().size());
  // Initialise the network
  std::vector<uint32_t> vint;
  vint.clear();
  for (auto x : _tca->asicMap())
  {
    uint32_t eip = ((x.first) >> 32) & 0XFFFFFFFF;
    std::map<uint32_t, std::string>::iterator idif = diflist.find(eip);
    if (idif == diflist.end())
      continue;
    if (std::find(vint.begin(), vint.end(), eip) != vint.end())
      continue;

    LOG4CXX_INFO(_logFeb, __PRETTY_FUNCTION__ << " New DIF " << eip);
    vint.push_back(eip);
    lydaq::TdcWiznet *_tdc = new lydaq::TdcWiznet(eip);
    // Slow control
    _wiznet->addCommunication(idif->second, 10001);
    _wiznet->registerDataHandler(idif->second, 10001, boost::bind(&lydaq::TdcWiznet::processSlc, _tdc, _1, _2, _3));

    // TDC
    _wiznet->addDataTransfer(idif->second, 10002);
    _wiznet->registerDataHandler(idif->second, 10002, boost::bind(&lydaq::TdcWiznet::processBuffer, _tdc, _1, _2, _3));

    _vTdc.push_back(_tdc);
    LOG4CXX_INFO(_logFeb, __PRETTY_FUNCTION__ << " Registration done for " << eip);
  }
  // Connect to the event builder
  if (_context == NULL)
    _context = new zmq::context_t(1);

  if (m->content().isMember("publish"))
  {
    this->parameters()["publish"] = m->content()["publish"];
  }
  if (!this->parameters().isMember("publish"))
  {

    LOG4CXX_ERROR(_logFeb, __PRETTY_FUNCTION__ << " No publish tag found ");
    return;
  }
  else
    for (auto x : _vTdc)
      x->connect(_context, this->parameters()["publish"].asString());

  // Listen All Wiznet sockets
  _wiznet->listen();

  LOG4CXX_INFO(_logFeb, " Initialisation completed");
}

void lydaq::WiznetManager::writeAddress(std::string host, uint32_t port, uint16_t addr, uint16_t val)
{
  /// Encode one command for FEB+wiznet
  _msg->_address = ((uint64_t)lydaq::WiznetMessageHandler::convertIP(host) << 32) | port;
  _msg->_length = 4;
  _msg->_buf[0] = htons(0xFF00);
  _msg->_buf[1] = htons(1);
  _msg->_buf[2] = htons(addr);
  _msg->_buf[3] = htons(val);
  _wiznet->sendMessage(_msg);
}

void lydaq::WiznetManager::writeLongWord(std::string host, uint32_t port, uint16_t addr, uint64_t val)
{
  /// Encode one command for FEB+wiznet
  _msg->_address = ((uint64_t)lydaq::WiznetMessageHandler::convertIP(host) << 32) | port;
  _msg->_length = 10;
  _msg->_buf[0] = htons(0xFF00);
  _msg->_buf[1] = htons(4);
  _msg->_buf[2] = htons(addr);
  _msg->_buf[3] = htons(val & 0XFFFF);
  _msg->_buf[4] = htons(addr + 1);
  _msg->_buf[5] = htons((val >> 16) & 0XFFFF);
  _msg->_buf[6] = htons(addr + 2);
  _msg->_buf[7] = htons((val >> 32) & 0xFFFF);
  _msg->_buf[8] = htons(addr + 3);
  _msg->_buf[9] = htons((val >> 48) & 0xFFFF);
  _wiznet->sendMessage(_msg);
}
void lydaq::WiznetManager::configure(zdaq::fsmmessage *m)
{
  LOG4CXX_INFO(_logFeb, "CONFIGURE transition");

  // Now loop on slowcontrol socket

  for (auto x : _wiznet->controlSockets())
  {
    this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x220, 0); //Stop acquisition

    _tca->prepareSlowControl(x.second->hostTo());

    _wiznet->writeRamAvm(x.second, _tca->slcAddr(), _tca->slcBuffer(), _tca->slcBytes());
    this->readShm(x.second->hostTo(), x.second->portTo());
  }
}

void lydaq::WiznetManager::set6bDac(uint8_t dac)
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
  for (auto x : _wiznet->controlSockets())
  {

    _tca->prepareSlowControl(x.second->hostTo());

    _wiznet->writeRamAvm(x.second, _tca->slcAddr(), _tca->slcBuffer(), _tca->slcBytes());
    this->readShm(x.second->hostTo(), x.second->portTo());
  }

  ::usleep(50000);
}
void lydaq::WiznetManager::cal6bDac(uint32_t mask,int32_t dacShift)
{

  //::usleep(50000);

  // Modify ASIC SLC
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {

    for (int i = 0; i < 32; i++)
    {
      if ((mask>>i)&1)
	{
	  uint32_t dac=it->second.get6bDac(i);
	  if (dac+dacShift>0)
	    it->second.set6bDac(i, dac+dacShift);
	}
    }
  }
  // Now loop on slowcontrol socket and send packet
  for (auto x : _wiznet->controlSockets())
  {

    _tca->prepareSlowControl(x.second->hostTo());

    _wiznet->writeRamAvm(x.second, _tca->slcAddr(), _tca->slcBuffer(), _tca->slcBytes());
    this->readShm(x.second->hostTo(), x.second->portTo());
  }

  ::usleep(50000);
}
void lydaq::WiznetManager::c_asics(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  response["STATUS"] = "DONE";
  Json::Value jlist;;
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    Json::Value jasic;
    uint32_t iasic=it->first & 0xFF;
    jasic["num"] = iasic;
    uint32_t difid= ((it->first)>>32 & 0xFFFFFFFF);
    jasic["dif"] = difid;
    it->second.toJson();
    jasic["slc"]=it->second.getJson();
    jlist.append(jasic);
  }
  response["asics"]=jlist;
}


void lydaq::WiznetManager::setMask(uint32_t mask, uint8_t asic)
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
  for (auto x : _wiznet->controlSockets())
  {

    _tca->prepareSlowControl(x.second->hostTo());

    _wiznet->writeRamAvm(x.second, _tca->slcAddr(), _tca->slcBuffer(), _tca->slcBytes());
    this->readShm(x.second->hostTo(), x.second->portTo());
  }

  ::usleep(50000);
}

void lydaq::WiznetManager::setVthTime(uint32_t vth)
{

  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Debut ");
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    int iasic = it->first & 0xFF;

    it->second.setVthTime(vth);
    //it->second.Print();
    // 1 seul ASIC break;
  }

  // Now loop on slowcontrol socket
  for (auto x : _wiznet->controlSockets())
  {
    LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Data send to " << x.second->hostTo());
    _tca->prepareSlowControl(x.second->hostTo());

    _wiznet->writeRamAvm(x.second, _tca->slcAddr(), _tca->slcBuffer(), _tca->slcBytes());
    this->readShm(x.second->hostTo(), x.second->portTo());
  }
  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Fin ");
}

void lydaq::WiznetManager::setSingleVthTime(uint32_t vth, uint32_t feb, uint32_t asic)
{
  // Encode IP
  std::stringstream ip;
  ip << "192.168.10." << feb;

  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Debut ");
  for (auto it = _tca->asicMap().begin(); it != _tca->asicMap().end(); it++)
  {
    //  Change VTH time only on specified ASIC
    uint64_t eid = (((uint64_t)lydaq::WiznetMessageHandler::convertIP(ip.str())) << 32) | asic;
    if (eid != it->first)
      continue;
    it->second.setVthTime(vth);
  }

  // Now loop on slowcontrol socket
  for (auto x : _wiznet->controlSockets())
  {
    LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Data send to " << x.second->hostTo());
    _tca->prepareSlowControl(x.second->hostTo());

    _wiznet->writeRamAvm(x.second, _tca->slcAddr(), _tca->slcBuffer(), _tca->slcBytes());
     this->readShm(x.second->hostTo(), x.second->portTo());
  }
  LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << " Fin ");
}

void lydaq::WiznetManager::setDelay()
{
  LOG4CXX_INFO(_logFeb, "Setting dead time " << _delay);
  for (auto x : _wiznet->controlSockets())
  {
    this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x222, _delay);
  }
}
void lydaq::WiznetManager::setDuration()
{
  LOG4CXX_INFO(_logFeb, " Setting  Active duration " << _duration);
  for (auto x : _wiznet->controlSockets())
  {
    this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x223, _duration);
  }
}
void lydaq::WiznetManager::getLUT(int chan)
{
  LOG4CXX_INFO(_logFeb, " get LUT for " << chan << " on all FEBS");
  for (auto x : _wiznet->controlSockets())
  {
    this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x224, chan);
    ::sleep(1);
    this->readShm(x.second->hostTo(), x.second->portTo());
  }
}
void lydaq::WiznetManager::getCalibrationStatus()
{
  LOG4CXX_INFO(_logFeb, " get Calibration Status for on all FEBS");
  for (auto x : _wiznet->controlSockets())
  {
    this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x225, 0);
    ::usleep(100000);
    this->readShm(x.second->hostTo(), x.second->portTo());
  }
}
void lydaq::WiznetManager::setCalibrationMask(uint64_t mask)
{
  LOG4CXX_INFO(_logFeb, " setCalibrationMask " << std::hex << mask << std::dec << " on all FEBS");
  for (auto x : _wiznet->controlSockets())
  {
    this->writeLongWord(x.second->hostTo(), x.second->portTo(), 0x226, mask);
  }
}
void lydaq::WiznetManager::setMeasurementMask(uint64_t mask)
{
  LOG4CXX_INFO(_logFeb, " setMeasurementMask " << std::hex << mask << std::dec << " on all FEBS");
  for (auto x : _wiznet->controlSockets())
  {
    this->writeLongWord(x.second->hostTo(), x.second->portTo(), 0x230, mask);
  }
}
void lydaq::WiznetManager::start(zdaq::fsmmessage *m)
{

  // Create run file
  Json::Value jc = m->content();
  _run = jc["run"].asInt();

  // Clear buffers
  for (auto x : _vTdc)
  {
    x->clear();
  }

  // Turn run type on

  switch (_type)
  {
  case 0: // ilc mode
  {
    LOG4CXX_INFO(_logFeb, " Starting ILC Mode " << _type << " run " << _run);
    for (auto x : _wiznet->controlSockets())
    {
      this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x219, 0); //ILC Mode
      this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x220, 1); //Sart acquisition
    }

    break;
  }
  case 1: // beamtest mode
  {
    LOG4CXX_INFO(_logFeb, "Starting beamtest Mode" << _type << "run " << _run);
    for (auto x : _wiznet->controlSockets())
    {
      this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x219, 1); // Beam test Mode
      this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x220, 1); //Sart acquisition
    }
    break;
  }
  }
}
void lydaq::WiznetManager::stop(zdaq::fsmmessage *m)
{
  LOG4CXX_INFO(_logFeb, "STOP transition");
  //std::cout<<m->command()<<std::endl<<m->content()<<std::endl;
  for (auto x : _wiznet->controlSockets())
  {

    this->writeAddress(x.second->hostTo(), x.second->portTo(), 0x220, 0); // Stop
  }

  ::sleep(2);
}
void lydaq::WiznetManager::destroy(zdaq::fsmmessage *m)
{

  LOG4CXX_INFO(_logFeb, "DESTROY  transition");

  for (auto x : _vTdc)
    delete x;
  LOG4CXX_INFO(_logFeb, " Data sockets deleted");
  _vTdc.clear();

  // To be done: _wiznet->clear();
}
void lydaq::WiznetManager::readShm(std::string host,uint32_t port)
{
  ::sleep(1);
  std::stringstream s;
  s<<"/dev/shm/"<<host<<"/"<<port<<"/data";

  int fd= ::open(s.str().c_str(),O_RDONLY);
  if (fd<0)
  {
    
    LOG4CXX_FATAL(_logFeb," Cannot open shm file "<<s.str());
    perror("No way to store to file :");
    _controlSize=fd;
    return;
  }
  _controlSize=65536;
  int32_t ier=::read(fd,_controlData,_controlSize);
  _controlSize=ier;
  ::close(fd);
  _jControl.clear();
  Json::Value jl;
  jl["size"]=_controlSize;
  if (_controlSize>0)
    {
      
      Json::Value jt;
      for (int i =0;i<_controlSize;i++)       
	jt.append((uint16_t) _controlData[i]);
      jl["content"]=jt;
    }
  _jControl[host]=jl;
}
