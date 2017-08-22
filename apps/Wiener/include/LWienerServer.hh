#ifndef _LWienerServer_h

#define _LWienerServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"

#include "WienerSnmp.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class LWienerServer : public zdaq::baseApplication
  {
  public:
    LWienerServer(std::string name);
    // Transition
    void open(zdaq::fsmmessage* m);
    void close(zdaq::fsmmessage* m);
    void start(zdaq::fsmmessage* m);
    void monitor();
    void stop(zdaq::fsmmessage* m);
    // Access to the interface
    lydaq::WienerSnmp* getHVWienerInterface(){  //std::cout<<" get Ptr "<<_hv<<std::endl;
      return _hv;}
    // Status
    Json::Value status();
    Json::Value channelStatus(uint32_t channel);
    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_vset(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_iset(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_rampup(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_on(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_off(Mongoose::Request &request, Mongoose::JsonResponse &response);
  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::WienerSnmp* _hv;
    boost::thread_group g_store;
    bool _running;
    uint32_t _period;
    zmq::context_t* _context;
    zmq::socket_t *_publisher;
  };
};
#endif

