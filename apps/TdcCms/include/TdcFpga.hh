#ifndef _TDCFPGA_HH
#define _TDCFPGA_HH
#include "zmPusher.hh"
#define MAX_EVENT_SIZE 65535
#define MAX_TDC_NB 255
#define TDC_VERSION 120
#include <iostream>
#include <list>
#include <vector>
#include <stdint.h>
#include "TdcChannel.hh"
using namespace std;
namespace lydaq {

class TdcFpga
{
public:
  TdcFpga(uint32_t m,uint32_t adr);
  virtual void processEventTdc();
  inline void setStorage(std::string sdir) {_sdir=sdir;}
  inline void setTriggerId(uint8_t i) {_triggerId=i;}
  void addChannels(uint8_t* buf,uint32_t sizeb);
  uint32_t detectorId(){return _detid;}
  uint32_t difId(){return _id;}
  uint64_t abcid(){return _abcid;}
  uint32_t gtc(){return _gtc;}
  uint32_t event(){return _event;}
  uint32_t triggers(){return _ntrg;}
  void clear();
  void connect(zmq::context_t* c,std::string dest);
private:
  uint32_t _mezzanine,_adr,_startIdx;
  uint8_t _buf[0x100000];
  vector<TdcChannel> _channels;
  uint64_t _abcid;
  uint32_t _gtc,_lastGtc,_event,_detid,_id,_ntrg;
  uint32_t _nBuffers;
  zdaq::zmPusher* _dsData;
  std::string _sdir;
  uint8_t _triggerId;

};
};

#endif
