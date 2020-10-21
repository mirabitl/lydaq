#ifndef _GRICV0_MANAGER_HH
#define _GRICV0_MANAGER_HH
#include "Gricv0Interface.hh"
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

class Gricv0Manager : public zdaq::baseApplication
{
public:
  Gricv0Manager(std::string name);
  ~Gricv0Manager(){;}

  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_startacq(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_stopacq(Mongoose::Request &request, Mongoose::JsonResponse &response);

  void c_storesc(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_loadsc(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_readsc(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_lastabcid(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_lastgtc(Mongoose::Request &request, Mongoose::JsonResponse &response);


  void c_pulse(Mongoose::Request &request, Mongoose::JsonResponse &response);

  void initialise(zdaq::fsmmessage* m);
  void configureHR2();
  void configure(zdaq::fsmmessage* m);
  void setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif=0);
  void setGain(uint16_t gain);
  void setMask(uint32_t level,uint64_t mask);
  void setChannelMask(uint16_t level,uint16_t channel,uint16_t val);
  void start(zdaq::fsmmessage* m);
  void stop(zdaq::fsmmessage* m);
  void destroy(zdaq::fsmmessage* m);



private:
  lydaq::HR2ConfigAccess* _hca;
  lydaq::gricv0::Interface* _mpi;



  zdaq::fsmweb* _fsm;
  uint32_t _run,_type;
  uint8_t _delay;
  uint8_t _duration;

  zmq::context_t* _context;
};
};
#endif
