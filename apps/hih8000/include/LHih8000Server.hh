#ifndef _LHih8000Server_h

#define _LHih8000Server_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitorApplication.hh"

#include "hih8000.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class LHih8000Server : public zdaq::monitorApplication
  {
  public:
    LHih8000Server(std::string name);
    // Transition
    virtual void open(zdaq::fsmmessage* m);
    virtual void close(zdaq::fsmmessage* m);
    // Access to the interface
    lydaq::hih8000* getHih8000Interface(){  //std::cout<<" get Ptr "<<_hih8000<<std::endl;
      return _hih8000;}
    // Status
    virtual Json::Value status();
    virtual std::string hardware(){return "HIH8000";}
    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::hih8000* _hih8000;
  };
};
#endif

