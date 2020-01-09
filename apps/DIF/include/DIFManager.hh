#ifndef _DIFManager_h

#define _DIFManager_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "baseApplication.hh"
#include "DIFInterface.hh"
#include "DIFReadoutConstant.hh"
#include "HR2ConfigAccess.hh"

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
    void scan(zdaq::fsmmessage* m);
    void initialise(zdaq::fsmmessage* m);
    void configure(zdaq::fsmmessage* m);
    void start(zdaq::fsmmessage* m);
    void stop(zdaq::fsmmessage* m);
    void destroy(zdaq::fsmmessage* m);

    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setchannelmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setmask(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setthresholds(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setpagain(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_ctrlreg(Mongoose::Request &request, Mongoose::JsonResponse &response);
    /**
       Change the threshold on all Asics of the DIF
       @param b0 First threshold
       @param idif The dif ID
     */
    void setThresholds(uint16_t b0,uint16_t b1,uint16_t b2,uint32_t idif);
    void setGain(uint16_t gain);
    void setMask(uint32_t level,uint64_t mask);
    void setChannelMask(uint16_t level,uint16_t channel,uint16_t val);

    Json::Value configureHR2();

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
    std::vector<lydaq::DIFInterface*> _vDif;
    lydaq::HR2ConfigAccess* _hca;
    zdaq::fsmweb* _fsm;
    boost::thread_group g_d;
    zmq::context_t* _context;

  };
};
#endif

