#ifndef _TDC_MANAGER_HH
#define _TDC_MANAGER_HH
#include "TdcMessageHandler.hh"
#include "baseApplication.hh"
#include "PRSlow.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
namespace lydaq
{
struct evs {
  uint64_t abcid;
  double ltdc,rtdc;
  uint32_t bcid,idx,event,time,mezzanine,strip;
    
};

class TdcManager : public zdaq::baseApplication
{
public:
  TdcManager(std::string name);
  ~TdcManager(){;}
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
  void c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response);
  
  void parseConfig(std::string name);
  void writeRamAvm();
  void queryCRC();
  void startAcquisition( bool start);
  void listen();
  void dolisten();
  void set6bDac(uint8_t dac);
  void setMask(uint32_t mask);
  void sendTrigger(uint32_t nt);
  void setVthTime(uint32_t dac);
private:
  NL::Socket* _sCtrl;
  NL::Socket* _sTDC1;
  NL::Socket* _sTDC2;
  NL::SocketGroup* _group;
 
  lydaq::TdcMessageHandler* _msh;
  lytdc::OnRead* _onRead;
  lytdc::OnAccept* _onAccept;
  lytdc::OnClientDisconnect* _onClientDisconnect;
  lytdc::OnDisconnect* _onDisconnect;
  boost::thread_group g_store;
  boost::thread_group g_run;


  uint32_t disconnected_;
  zdaq::fsmweb* _fsm;
  uint16_t _slcBuffer[0x1000];
  uint16_t _slcAddr[0x1000];    
  uint32_t _slcBytes;
  uint16_t _slcAck[0x1000];
  uint32_t _slcAckLength;
  PRSlow _s1,_s2;
  uint32_t _run,_type;
  std::string _directory;
  int32_t _fdOut;
  struct evs _eventStruct;
  uint32_t _t0;
  bool _running;

  uint32_t _firstEvent[2],_currentGTC[2];
  uint64_t _currentABCID[2];
  bool _loop;
  zmq::context_t* _context;
};
};
#endif
