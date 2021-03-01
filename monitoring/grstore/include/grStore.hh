#ifndef _mgstore_h

#define _mgstore_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "zmonStore.hh"
#include <mongoc.h>
#include <json/json.h>


using namespace std;
#include <sstream>
#include "ReadoutLogger.hh"

namespace lydaq
{
  class grStore : public zdaq::zmonStore
  {
  public:
    grStore();
    virtual void connect();
    virtual void store(std::string loc,std::string hw,uint32_t ti,Json::Value status);
    virtual  void loadParameters(Json::Value params);
    // Access to the interface
  private:
    //zdaq::fsm* _fsm;
    Json::Value _params;
    std::string _graphite_host,_graphite_port;

  };
};
#endif

