#ifndef _LBmpServer_h

#define _LBmpServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"

#include "BMP183.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class LBmpServer : public zdaq::baseApplication
  {
  public:
    LBmpServer(std::string name);
    // Transition
    void open(zdaq::fsmmessage* m);
    void close(zdaq::fsmmessage* m);
    void start(zdaq::fsmmessage* m);
    void monitor();
    void stop(zdaq::fsmmessage* m);
    // Access to the interface
    lydaq::BMP183* getBmpInterface(){  //std::cout<<" get Ptr "<<_bmp<<std::endl;
      return _bmp;}
    // Status
    Json::Value status();
    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::BMP183* _bmp;
    boost::thread_group g_store;
    bool _running;
    uint32_t _period;
    zmq::context_t* _context;
    zmq::socket_t *_publisher;
  };
};
#endif

