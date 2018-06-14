#ifndef _TDCWIZNET_HH
#define _TDCWIZNET_HH
#include "zmPusher.hh"
#define TDC_VERSION 130
#include <iostream>
#include <list>
#include <vector>
#include <stdint.h>
using namespace std;
namespace lydaq {

class TdcWiznet
{
public:
  TdcWiznet(uint32_t adr);
  void processBuffer(uint64_t id, uint16_t l,char* b);
  void processSlc(uint64_t id, uint16_t l,char* b);
  bool processPacket();
  void purgeBuffer();

  void processEventTdc();

  int16_t checkBuffer(uint8_t* b,uint32_t maxidx);
  inline void setTriggerId(uint8_t i) {_triggerId=i;}
  inline uint32_t detectorId(){return _detid;}
  inline uint32_t difId(){return _id;}
  inline uint64_t abcid(){return _lastABCID;}
  inline uint32_t gtc(){return _lastGTC;}
  inline uint32_t packets(){return _nProcessed;}
  inline uint32_t event(){return _event;}
  inline uint32_t triggers(){return _ntrg;}
  void clear();
  void connect(zmq::context_t* c,std::string dest);
private:
  uint32_t _adr,_idx,_chlines;
  uint8_t _buf[0x1000000];
  uint8_t _linesbuf[0x1000000];

  uint64_t _lastABCID;
  uint32_t _lastGTC,_event,_detid,_id,_ntrg,_expectedLength;
  uint32_t _nProcessed;
  zdaq::zmPusher* _dsData;
  uint8_t _triggerId;

};
};

#endif