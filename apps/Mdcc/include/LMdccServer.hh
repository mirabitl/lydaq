#ifndef _LMdccServer_h

#define _LMdccServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"

#include "MDCCHandler.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"

namespace lydaq
{
  class LMdccServer : public zdaq::baseApplication
  {
  public:
    LMdccServer(std::string name);
    void open(zdaq::fsmmessage* m);
    void close(zdaq::fsmmessage* m);
    void pause(zdaq::fsmmessage* m);
    void resume(zdaq::fsmmessage* m);
    void ecalpause(zdaq::fsmmessage* m);
    void ecalresume(zdaq::fsmmessage* m);
    void reset(zdaq::fsmmessage* m);
    void cmd(zdaq::fsmmessage* m);
    void doOpen(std::string s);

    MDCCHandler* getMDCCHandler(){  //std::cout<<" get Ptr "<<_mdcc<<std::endl;
      return _mdcc;}
    void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_pause(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_resume(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_ecalpause(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_ecalresume(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_reset(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_readreg(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_writereg(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_spillon(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_spilloff(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_beamon(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setcalibcount(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_reloadcalib(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_calibon(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_caliboff(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_resettdc(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setspillregister(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_settrigext(Mongoose::Request &request, Mongoose::JsonResponse &response);
  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::MDCCHandler* _mdcc;
  };
};
#endif

