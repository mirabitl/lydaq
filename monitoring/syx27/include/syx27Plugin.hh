#ifndef _syx27Plugin_h

#define _syx27Plugin_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "zmonPlugin.hh"

#include "HVCaenInterface.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class syx27Plugin : public zdaq::zmonPlugin
  {
  public:
    syx27Plugin();
    // Transition
    virtual void open(zdaq::fsmmessage* m);
    virtual void close(zdaq::fsmmessage* m);
    virtual void registerCommands(zdaq::fsmweb* f);
    virtual  void loadParameters(Json::Value params);
    // Access to the interface
    lydaq::HVCaenInterface* getHVCaenInterface(){  //std::cout<<" get Ptr "<<_hv<<std::endl;
      return _hv;}
    // Status
    virtual Json::Value status();
    Json::Value status(int32_t f,int32_t l);
    virtual std::string hardware(){return "SYX27";}
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
    Json::Value _params;
    lydaq::HVCaenInterface* _hv;
  };
};
#endif

