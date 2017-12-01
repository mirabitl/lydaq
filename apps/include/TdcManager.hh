#ifndef _TDC_MANAGER_HH
#define _TDC_MANAGER_HH
#include "TdcMessageHandler.hh"
#include "TdcConfigAccess.hh"
#include "baseApplication.hh"
#include "PRSlow.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"

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
  void c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
  
  //void parseConfig(std::string name);
  void writeRamAvm(NL::Socket* sctrl,uint16_t* sa,uint16_t* sb,uint32_t sby);
  void queryCRC(NL::Socket* sctrl);
  void startAcquisition( NL::Socket* sctrl,bool start);
  void listen();
  void dolisten();
  void set6bDac(uint8_t dac);
  void setMask(uint32_t mask);
  void sendTrigger(uint32_t nt);
  void setVthTime(uint32_t dac);
private:
  lydaq::TdcConfigAccess* _tca;
  std::vector<NL::Socket*> _vsCtrl;
  std::vector<NL::Socket*> _vsTdc;

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
  uint32_t _run,_type;
  std::string _directory;
  int32_t _fdOut;
  struct evs _eventStruct;
  uint32_t _t0;
  bool _running;


  bool _loop;
  zmq::context_t* _context;
};
};
#endif
