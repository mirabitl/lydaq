#ifndef _TDCFPGA_HH
#define _TDCFPGA_HH
#include "zmPusher.hh"
#define MAX_EVENT_SIZE 65535

#include <iostream>
#include <list>
#include <vector>
#include <stdint.h>
using namespace std;
namespace lydaq {
class TdcChannel
{
public:
  TdcChannel(uint8_t*  b) :_fr(b),_used(false) {;}
  inline uint8_t channel() {return  (_fr[0]&0XFF);}
  inline uint8_t length(){return 8;}
  inline uint64_t coarse() const {return ((uint64_t)_fr[6])|((uint64_t)_fr[5]<<8)|((uint64_t)_fr[4]<<16)|((uint64_t)_fr[3]<<24);}
  inline uint8_t fine() const {return _fr[7];}
  #ifdef BCIDFROMCOARSE
  inline uint16_t bcid(){return (uint16_t) (coarse()*2.5/200);}
  #else
  inline const uint16_t bcid(){return (_fr[2]|(_fr[1]<<8));}
  #endif
  inline  double tdcTime() const { return (coarse()*2.5+fine()*0.009765625);}
  inline uint8_t* frame(){ return _fr;}
  inline bool used(){return _used;}
  inline void setUsed(bool t){_used=t;}
  bool operator<(const TdcChannel &ipaddr){
    if( coarse() < ipaddr.coarse())
    return true;
  else
    return false;
}
private:
  uint8_t* _fr;
  bool _used;
};

class TdcFpga
{
public:
  TdcFpga(uint32_t m,uint32_t adr);
  virtual void processEventTdc();
  inline void setStorage(std::string sdir) {_sdir=sdir;}
  void addChannels(uint8_t* buf,uint32_t sizeb);
  uint32_t detectorId(){return _detid;}
  uint32_t difId(){return _id;}
  uint64_t abcid(){return _abcid;}
  uint32_t gtc(){return _gtc;}
  uint32_t event(){return _event;}
  void clear();
  void connect(zmq::context_t* c,std::string dest);
private:
  uint32_t _mezzanine,_adr,_startIdx;
  uint8_t _buf[0x100000];
  vector<TdcChannel> _channels;
  uint64_t _abcid;
  uint32_t _gtc,_lastGtc,_event,_detid,_id;
  uint32_t _nBuffers;
  zdaq::zmPusher* _dsData;
  std::string _sdir;

};
};

#endif
