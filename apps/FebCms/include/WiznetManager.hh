#ifndef _WIZNET_MANAGER_HH
#define _WIZNET_MANAGER_HH
#include "WiznetInterface.hh"
#include "TdcWiznet.hh"
#include "TdcConfigAccess.hh"
#include "baseApplication.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"

namespace lydaq
{

class WiznetManager : public zdaq::baseApplication
{
public:
  WiznetManager(std::string name);
  ~WiznetManager(){;}
  void initialise(zdaq::fsmmessage* m);
  void configure(zdaq::fsmmessage* m);
  void start(zdaq::fsmmessage* m);
  void stop(zdaq::fsmmessage* m);
  void destroy(zdaq::fsmmessage* m);
  void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_diflist(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_set6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setMode(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setDelay(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setDuration(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_set1vthtime(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
  
  // FEB access
  void writeAddress(std::string host,uint32_t port,uint16_t addr,uint16_t val);
  void set6bDac(uint8_t dac);
  void setMask(uint32_t mask,uint8_t asic=255);
  void sendTrigger(uint32_t nt);
  void setVthTime(uint32_t dac);
  void setSingleVthTime(uint32_t vth,uint32_t feb,uint32_t asic);
  void setDelay();
  void setDuration();
private:
  lydaq::TdcConfigAccess* _tca;
  lydaq::WiznetInterface* _wiznet;
  lydaq::WiznetMessage* _msg;

  std::vector<lydaq::TdcWiznet*> _vTdc;

  zdaq::fsmweb* _fsm;
  uint32_t _run,_type;
  uint8_t _delay;
  uint8_t _duration;

  zmq::context_t* _context;
};
};
#endif
