#ifndef _genesysPlugin_h

#define _genesysPlugin_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "zmonPlugin.hh"

#include "Genesys.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class genesysPlugin : public zdaq::zmonPlugin
  {
  public:
    genesysPlugin();
    // Transition
    virtual void open(zdaq::fsmmessage* m);
    virtual void close(zdaq::fsmmessage* m);
    virtual void registerCommands(zdaq::fsmweb* f);
    virtual  void loadParameters(Json::Value params);
    // Access to the interface
    lydaq::Genesys* getGenesysInterface(){  //std::cout<<" get Ptr "<<_hv<<std::endl;
      return _lv;}
    // Status
    virtual Json::Value status();
    virtual std::string hardware(){return "GENESYS";}

    
    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_on(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_off(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setdevice(Mongoose::Request &request, Mongoose::JsonResponse &response);


  private:
    //zdaq::fsm* _fsm;
    Json::Value _params;
    lydaq::Genesys* _lv;
  };
};
#endif

