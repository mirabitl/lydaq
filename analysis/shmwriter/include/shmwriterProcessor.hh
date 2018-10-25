#ifndef _shmwriterProcessor_h_
#define _shmwriterProcessor_h_
#include "zmBuffer.hh"
#include "zmMerger.hh"
#include "json/json.h"
#include <stdint.h>
class DHCalEventReader;
class shmwriterProcessor : public zdaq::zmprocessor
{
public:
  shmwriterProcessor();

  virtual void start(uint32_t run);
  virtual void stop();
  virtual  void processEvent(uint32_t key,std::vector<zdaq::buffer*> dss);
  virtual  void processRunHeader(std::vector<uint32_t> header);
  virtual void loadParameters(Json::Value params);
  void store(uint32_t detid,uint32_t sourceid,uint32_t eventid,uint64_t bxid,void* ptr,uint32_t size,std::string destdir);
  static void ls(std::string sourcedir,std::vector<std::string>& res);

 private:
  std::string _filepath;
  uint32_t _run;
  bool _started;
};
#endif
