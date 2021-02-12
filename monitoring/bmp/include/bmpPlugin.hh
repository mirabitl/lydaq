#ifndef _bmpPlugin_h

#define _bmpPlugin_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "zmonPlugin.hh"
#undef BMP183
#ifdef BMP183
#include "BMP183.hh"
#else
#include "BMP280.hh"
#endif
using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class bmpPlugin : public zdaq::zmonPlugin
  {
  public:
    bmpPlugin();
    // Transition
    virtual void open(zdaq::fsmmessage* m);
    virtual void close(zdaq::fsmmessage* m);
    // Access to the interface
#ifdef BMP183
    lydaq::BMP183* getBmpInterface(){return _bmp;}
#else
    lydaq::BMP280* getBmpInterface(){return _bmp;}
#endif
    // Status
    virtual Json::Value status();
    virtual std::string hardware(){return "BMP";}
    virtual void registerCommands(zdaq::fsmweb* f);
    virtual  void loadParameters(Json::Value params);

    // Commande
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  private:
    //zdaq::fsm* _fsm;
    Json::Value _params;
#ifdef BMP183 
    lydaq::BMP183* _bmp;
#else
    lydaq::BMP280* _bmp;
#endif
  };
};
#endif

