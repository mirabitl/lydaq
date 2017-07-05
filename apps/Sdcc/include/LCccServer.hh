#ifndef _LCccServer_h

#define _LCccServer_h
#include <iostream>

#include <string.h>
#include <stdio.h>


#include "CCCManager.hh"

using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"
#include "baseApplication.hh"
namespace lydaq {
  class LCccServer : public zdaq::baseApplication
  {
  public:
    LCccServer(std::string name);
    void open(zdaq::fsmmessage* m);
    void initialise(zdaq::fsmmessage* m);
    void configure(zdaq::fsmmessage* m);
    void start(zdaq::fsmmessage* m);
    void stop(zdaq::fsmmessage* m);
    void cmd(zdaq::fsmmessage* m);
    void Open(std::string s);

    // getters

    lydaq::CCCManager* getManager(){return _manager;}
    // Commands
    void pause(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void resume(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void difreset(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void cccreset(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void readreg(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void writereg(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);

  private:
    std::string _state;
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
    lydaq::CCCManager* _manager;
  };
};
#endif

