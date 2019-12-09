#ifndef _DIFManager_h

#define _DIFManager_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"
#include "DIFInterface.hh"
#include "DIFReadoutConstant.hh"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "ReadoutLogger.hh"
namespace lydaq
{
  class DIFManager  : zdaq::baseApplication
  {
  
  public:
    DIFManager(std::string name);
    void registerdb(zdaq::fsmmessage* m);
    void dbcache(std::string server,std::vector<uint32_t> vids);
    void scan(zdaq::fsmmessage* m);
    void initialise(zdaq::fsmmessage* m);
    void configure(zdaq::fsmmessage* m);
    void start(zdaq::fsmmessage* m);
    void stop(zdaq::fsmmessage* m);
    void destroy(zdaq::fsmmessage* m);
    void status(zdaq::fsmmessage* m);

    void cmdStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void setThreshold(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void setGain(Mongoose::Request &request, Mongoose::JsonResponse &response);

    void prepareDevices();
    void startDIFThread(DIFInterface* d);
    // DimRpc interface
    std::map<uint32_t,FtdiDeviceInfo*>& getFtdiMap(){ return theFtdiDeviceInfoMap_;}
    std::map<uint32_t,DIFInterface*>& getDIFMap(){ return _DIFInterfaceMap;}
      
    FtdiDeviceInfo* getFtdiDeviceInfo(uint32_t i) { if ( theFtdiDeviceInfoMap_.find(i)!=theFtdiDeviceInfoMap_.end()) return theFtdiDeviceInfoMap_[i]; else return NULL;}

    void joinThreads(){g_d.join_all();}

  private:
    std::map<uint32_t,FtdiDeviceInfo*> theFtdiDeviceInfoMap_;	
    std::map<uint32_t,lydaq::DIFInterface*> _DIFInterfaceMap;
    std::string _dbstate;
    //zdaq::fsm* _fsm;
    zdaq::fsmweb* _fsm;
    boost::thread_group g_d,g_db;
    zmq::context_t* _context;
    bool _dbcacheRunning;

  };
};
#endif

