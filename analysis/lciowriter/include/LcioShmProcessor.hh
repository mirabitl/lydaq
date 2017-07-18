#ifndef _LcioShmProcessor_h_
#define _LcioShmProcessor_h_
#include "zmBuffer.hh"
#include "zmMerger.hh"
#include "json/json.h"
#include <stdint.h>
class DHCalEventReader;
class LcioShmProcessor : public zdaq::zmprocessor
{
public:
  LcioShmProcessor(std::string dir="/tmp",std::string setup="default");

  virtual void start(uint32_t run);
  virtual void stop();
  virtual  void processEvent(uint32_t key,std::vector<zdaq::buffer*> dss);
  virtual  void processRunHeader(std::vector<uint32_t> header);
  virtual void loadParameters(Json::Value params);
  std::string getOutputFileName(uint32_t run,uint32_t seq);
  void close();
 private:
  std::string _filepath,_setup;
  uint32_t theEventNumber_,theRunNumber_,theSequence_;
  DHCalEventReader* _der;
  bool _started;
};
#endif
