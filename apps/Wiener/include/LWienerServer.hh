#ifndef _LWienerServer_h

#define _LWienerServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitorApplication.hh"

#include "WienerSnmp.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class LWienerServer : public zdaq::monitorApplication
  {
  public:
    LWienerServer(std::string name);
    // Transition
    virtual void open(zdaq::fsmmessage* m);
    virtual void close(zdaq::fsmmessage* m);
    // Access to the interface
    lydaq::WienerSnmp* getHVWienerInterface(){  //std::cout<<" get Ptr "<<_hv<<std::endl;
      return _hv;}
    // Status
    virtual Json::Value status();
    Json::Value status(int32_t f,int32_t l);
    virtual std::string hardware(){return "ISEG";}
    Json::Value channelStatus(uint32_t channel);
    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_vset(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_iset(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_rampup(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_on(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_off(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_clearalarm(Mongoose::Request &request, Mongoose::JsonResponse &response);

  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::WienerSnmp* _hv;
  };
};
#endif

