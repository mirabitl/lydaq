#ifndef _LZupServer_h

#define _LZupServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"

#include "Zup.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class LZupServer : public zdaq::baseApplication
  {
  public:
    LZupServer(std::string name);
    // Transition
    void open(zdaq::fsmmessage* m);
    void close(zdaq::fsmmessage* m);
    void start(zdaq::fsmmessage* m);
    void monitor();
    void stop(zdaq::fsmmessage* m);
    // Access to the interface
    lydaq::Zup* getLVZupInterface(){  //std::cout<<" get Ptr "<<_lv<<std::endl;
      return _lv;}
    // Status
    Json::Value status();
    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_on(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_off(Mongoose::Request &request, Mongoose::JsonResponse &response);
  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::Zup* _lv;
    boost::thread_group g_store;
    bool _running;
    uint32_t _period;
    zmq::context_t* _context;
    zmq::socket_t *_publisher;
  };
};
#endif

