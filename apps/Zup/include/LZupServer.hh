#ifndef _LZupServer_h

#define _LZupServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "monitorApplication.hh"

#include "Zup.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class LZupServer : public zdaq::monitorApplication
  {
  public:
    LZupServer(std::string name);
    // Transition
    virtual void open(zdaq::fsmmessage* m);
    virtual void close(zdaq::fsmmessage* m);
    // Access to the interface
    lydaq::Zup* getLVZupInterface(){  //std::cout<<" get Ptr "<<_lv<<std::endl;
      return _lv;}
    // Status
    virtual Json::Value status();
    virtual std::string hardware(){return "ZUP";}

    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_on(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_off(Mongoose::Request &request, Mongoose::JsonResponse &response);
  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::Zup* _lv;
    
  };
};
#endif

