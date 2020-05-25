#ifndef _GRICMPI_HH
#define _GRICMPI_HH
#include "zmSender.hh"
#define GRIC_VERSION 140
#include <iostream>
#include <list>
#include <vector>
#include <map>
#include <stdint.h>
#include "debug.hh"
#include "ReadoutLogger.hh"
using namespace std;
#define MBSIZE 0x100000
namespace lydaq {

class GricMpi
{
public:
  GricMpi(uint32_t adr);
  void processBuffer(uint64_t id, uint16_t l,char* b);
  bool processPacket();
  void purgeBuffer();

  void processEventGric();
  void processSensorGric();
  void processCommandGric();

  bool isHardrocData();
  bool isSensorData();
  bool isCommandData();
  

  int16_t checkBuffer(uint8_t* b,uint32_t maxidx);

  inline void setTriggerId(uint8_t i) {_triggerId=i;}
  inline uint32_t detectorId(){return _detid;}
  inline uint32_t difId(){return _id;}
  inline uint64_t abcid(){return _lastABCID;}
  inline uint32_t gtc(){return _lastGTC;}
  inline uint32_t packets(){return _nProcessed;}
  inline uint32_t event(){return _event;}
  inline uint32_t triggers(){return _ntrg;}
  inline uint8_t* answer(uint8_t tr){return _answ[tr];}
  inline uint32_t address(){return _adr;}
  void clear();
  void connect(zmq::context_t* c,std::string dest);
  void autoRegister(zmq::context_t* c,Json::Value config,std::string appname,std::string portname);
private:
  uint32_t _adr,_idx,_chlines;
  uint8_t _buf[MBSIZE];
  

  uint64_t _lastABCID;
  uint32_t _lastGTC,_event,_detid,_id,_ntrg,_expectedLength;
  uint32_t _nProcessed;
  zdaq::zmSender* _dsData;
  uint8_t _triggerId;
  // temporary buffer
  uint8_t _b[0x1000000];
  // Command answers
  std::map<uint8_t,uint8_t*> _answ;

  
};
};

#endif
