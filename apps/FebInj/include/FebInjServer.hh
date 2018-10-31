#ifndef _FebInjServer_h

#define _FebInjServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"

#include "FebInj.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include <zmq.hpp>

namespace lydaq
{
  class FebInjServer : public zdaq::baseApplication
  {
  public:
    FebInjServer(std::string name);
    // Transition
    void configure(zdaq::fsmmessage* m);
    void destroy(zdaq::fsmmessage* m);
    // Commande
    void c_set_mask(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_set_trigger_source(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_software_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_internal_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_pause_external_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_resume_external_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_set_number_of_trigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_set_delay(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_set_duration(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_set_pulse_height(Mongoose::Request &request, Mongoose::JsonResponse &response);
    
  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::FebInj* _inj;
  };
};
#endif

