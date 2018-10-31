#ifndef _monitorProcessor_h_
#define _monitorProcessor_h_
#include "zmBuffer.hh"
#include "zmMerger.hh"
#include "json/json.h"
#include <stdint.h>
class DHCalEventReader;
class monitorProcessor : public zdaq::zmprocessor
{
public:
  monitorProcessor();

  virtual void start(uint32_t run);
  virtual void stop();
  virtual  void processEvent(uint32_t key,std::vector<zdaq::buffer*> dss);
  virtual  void processRunHeader(std::vector<uint32_t> header);
  virtual void loadParameters(Json::Value params);
  void store(uint32_t detid,uint32_t sourceid,uint32_t eventid,uint64_t bxid,void* ptr,uint32_t size,std::string destdir);
  void ls(std::string sourcedir,std::vector<std::string>& res);

 private:
  std::string _filepath;
  uint32_t _run,_nheader;
  bool _started;
};
#endif
