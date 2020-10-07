#ifndef _C3I_MANAGER_HH
#define _C3I_MANAGER_HH
#include "MpiInterface.hh"
#include "C3iMpi.hh"
#include "HR2ConfigAccess.hh"
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

class C3iManager : public zdaq::baseApplication
{
public:
  C3iManager(std::string name);
  ~C3iManager(){;}

  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_readreg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_writereg(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_readbme(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void initialise(zdaq::fsmmessage* m);
  void writeRegister(std::string host,uint32_t port,uint16_t address,uint32_t value);
  uint32_t readRegister(std::string host,uint32_t port,uint16_t address);
  void sendSlowControl(std::string host,uint32_t port,uint8_t* slc);
  void configureHR2();
  void configure(zdaq::fsmmessage* m);
  void setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif=0);
  void setGain(uint16_t gain);
  void setMask(uint32_t level,uint64_t mask);
  void setChannelMask(uint16_t level,uint16_t channel,uint16_t val);
  void start(zdaq::fsmmessage* m);
  void stop(zdaq::fsmmessage* m);
  void destroy(zdaq::fsmmessage* m);

  void processReply(uint32_t adr,uint32_t tr,uint8_t command,uint32_t* rep);

private:
  lydaq::HR2ConfigAccess* _hca;
  lydaq::c3i::MpiInterface* _mpi;
  lydaq::c3i::MpiMessage* _msg;

  std::vector<lydaq::C3iMpi*> _vC3i;
  std::map<uint32_t,lydaq::C3iMpi*> _mC3i;

  zdaq::fsmweb* _fsm;
  uint32_t _run,_type;
  uint8_t _delay;
  uint8_t _duration;

  zmq::context_t* _context;
};
};
#endif
