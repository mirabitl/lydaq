#ifndef _LMdccEcalServer_h

#define _LMdccEcalServer_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"

#include "MDCCReadout.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"

namespace lydaq
{
  class LMdccEcalServer : public zdaq::baseApplication
  {
  public:
    LMdccEcalServer(std::string name);
    void open(zdaq::fsmmessage* m);
    void close(zdaq::fsmmessage* m);
    void pause(zdaq::fsmmessage* m);
    void resume(zdaq::fsmmessage* m);
    void ecalpause(zdaq::fsmmessage* m);
    void ecalresume(zdaq::fsmmessage* m);
    void reset(zdaq::fsmmessage* m);
    void cmd(zdaq::fsmmessage* m);
    void doOpen(std::string s);

    MDCCReadout* getMDCCReadout(){  //std::cout<<" get Ptr "<<_mdcc<<std::endl;
      return _mdcc;}
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
    void c_setregister(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_getregister(Mongoose::Request &request, Mongoose::JsonResponse &response);
    
  private:
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
 
    lydaq::MDCCReadout* _mdcc;
  };
};
#endif

