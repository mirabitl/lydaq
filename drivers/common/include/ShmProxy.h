#ifndef _ShmProxy_H
#define _ShmProxy_H
#include <stdint.h>
#include <iostream>

#include <stdint.h>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>

#include <typeinfo>
#include <string>
#include <sstream>
//#include <map>
//#include <ext/hash_map>
//using namespace __gnu_cxx;
#include <map>
#include <time.h>
#include <stdio.h>

#include "DIFWritterInterface.h"
#ifdef USE_DIM
#include "dis.hxx"
#endif
#include "CommonLogger.h"

class ShmProxy
 #ifdef USE_DIM
: public DimServer
#endif
{
 public:
  ShmProxy(uint32_t nbdif=1,bool save=false,DIFWritterInterface* w=NULL);
  uint32_t getNumberOfDIF();
  void setNumberOfDIF(int32_t t);
  void setDIFWritter(DIFWritterInterface* w){theWritter_=w;theSave_=true;}
  void Initialise(bool purge=true);
  void Configure();
  void Start(uint32_t run=0,std::string dir="/tmp",uint32_t nd=0);
  void Stop();
  static void purgeShm(std::string memory_dir="/dev/shm");
  static void save2DevShm(unsigned char* cbuf,uint32_t size_buf,uint32_t dif_shift,std::string memory_dir);
  
  static void run2DevShm(uint32_t &run,std::string memory_dir);

  static void transferToFile(unsigned char* cbuf,uint32_t size_buf,uint64_t bcid,uint32_t detector_event,uint32_t global_event,uint32_t id,std::string memory_dir="/dev/shm");

	static uint32_t getBufferDIF(unsigned char* cb,uint32_t idx=0);
  static uint32_t getBufferDTC(unsigned char* cb,uint32_t idx=0);
  static uint32_t getBufferGTC(unsigned char* cb,uint32_t idx=0);
  static unsigned long long getBufferABCID(unsigned char* cb,uint32_t idx=0);
	static uint32_t getBufferDIFTemp(unsigned char* cb,uint32_t idx=0);
  static unsigned char*  packDIFData(unsigned char* cbuf,uint32_t size,uint32_t gtc,uint32_t dtc,uint32_t id);
  bool performReadFiles();
  virtual bool performWrite();
  void svc();
#ifdef OLDSTUFF
  void openFile(uint32_t run=0);
#endif
  inline bool RunIsStopped(){return theRunIsStopped_;}
  inline  std::map<uint64_t,std::vector<unsigned char*> >& getBufferMap(){return theBufferMap_;}
  
  uint32_t getRunNumber();
   uint32_t getEventNumber();
  void setSetupName(std::string s);
  void setDirectoryName(std::string s);
private:
  std::string theSetupName_;
  std::string theDirectoryName_;

    // work loops for transitional states
 
	boost::thread    theThread_;  

  bool theSave_;
  bool theRunIsStopped_;
  int fdOut_;
  boost::interprocess::interprocess_mutex theSync_;
  std::map<uint64_t,std::vector<unsigned char*> > theBufferMap_;  
  uint32_t theNumberOfDIF_;

  uint32_t theRunNumber_;
  uint32_t theEventNumber_;
  uint64_t lastGTCWrite_;
  uint32_t theTotalSize_;
  time_t theTime_;
  DIFWritterInterface* theWritter_;

#ifdef USE_DIM
  bool serveEvents_;
  DimCommand* theUpdateCmd_;
  std::map<uint32_t,DimService*> theDimServiceMap_; 
  std::map<uint32_t,uint32_t*> theDimBufferMap_; 
 public:
  void configureDimServer();
  void updateDimServices();
  std::map<uint32_t,uint32_t*>::iterator findDimBufferIterator(uint32_t difid);
#endif

};
#endif
