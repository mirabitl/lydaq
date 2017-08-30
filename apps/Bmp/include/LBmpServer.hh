#ifndef _LBmpServer_h

#define _LBmpServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitorApplication.hh"

#include "BMP183.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class LBmpServer : public zdaq::monitorApplication
  {
  public:
    LBmpServer(std::string name);
    // Transition
    virtual void open(zdaq::fsmmessage* m);
    virtual void close(zdaq::fsmmessage* m);
    // Access to the interface
    lydaq::BMP183* getBmpInterface(){  //std::cout<<" get Ptr "<<_bmp<<std::endl;
      return _bmp;}
    // Status
    virtual Json::Value status();
    virtual std::string hardware(){return "BMP";}
    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::BMP183* _bmp;
  };
};
#endif

