#ifndef _LGPIOServer_h

#define _LGPIOServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitorApplication.hh"

#include "GPIO.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class LGPIOServer : public zdaq::monitorApplication
  {
  public:
    LGPIOServer(std::string name);
    // Transition
    virtual void open(zdaq::fsmmessage* m);
    virtual void close(zdaq::fsmmessage* m);
    // Access to the interface
    lydaq::GPIO* getGPIOInterface(){  //std::cout<<" get Ptr "<<_GPIO<<std::endl;
      return _GPIO;}
    // Status
    virtual Json::Value status();
    virtual std::string hardware(){return "GPIO";}
    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_vmeon(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_vmeoff(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_difon(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_difoff(Mongoose::Request &request, Mongoose::JsonResponse &response);

  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::GPIO* _GPIO;
  };
};
#endif

