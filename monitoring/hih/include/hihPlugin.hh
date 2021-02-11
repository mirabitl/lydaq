#ifndef _hihPlugin_h

#define _hihPlugin_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "zmonPlugin.hh"
#include "hih8000.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class hihPlugin : public zdaq::zmonPlugin
  {
  public:
    hihPlugin();
    // Transition
    virtual void open(zdaq::fsmmessage* m);
    virtual void close(zdaq::fsmmessage* m);
    // Access to the interface
    // Access to the interface
    lydaq::hih8000* getHih8000Interface(){  //std::cout<<" get Ptr "<<_hih8000<<std::endl;
      return _hih;}
    // Status
    virtual Json::Value status();
    virtual std::string hardware(){return "HIH";}
    virtual void registerCommands(zdaq::fsmweb* f);
    virtual  void loadParameters(Json::Value params);

    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  private:
    //zdaq::fsm* _fsm;
    Json::Value _params;
    lydaq::hih8000* _hih;
  };
};
#endif

