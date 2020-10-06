#ifndef _C3IMPI_HH
#define _C3IMPI_HH
#include "zmSender.hh"
#define C3I_VERSION 145
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

class C3iMpi
{
public:
  C3iMpi(uint32_t adr);
  void processBuffer(uint64_t id, uint16_t l,char* b);
  bool processPacket();
  void purgeBuffer();

  void processEventData();
  void processSlcData();
  void processRegisterData();

  bool isHardrocData();
  bool isSlcData();
  bool isRegisterData();
  

  int16_t checkBuffer(uint8_t* b,uint32_t maxidx);

  inline void setTriggerId(uint8_t i) {_triggerId=i;}
  inline void setSlcStatus(uint32_t i) {_slcStatus=i;}
  inline uint32_t detectorId(){return _detid;}
  inline uint32_t difId(){return _id;}
  inline uint64_t abcid(){return _lastABCID;}
  inline uint32_t gtc(){return _lastGTC;}
  inline uint32_t packets(){return _nProcessed;}
  inline uint32_t event(){return _event;}
  inline uint32_t triggers(){return _ntrg;}
  inline uint8_t* answer(uint8_t tr){return _answ[tr];}
  inline uint32_t address(){return _adr;}
  inline uint32_t slcStatus(){return _slcStatus;}
  void clear();
  void connect(zmq::context_t* c,std::string dest);
  void autoRegister(zmq::context_t* c,Json::Value config,std::string appname,std::string portname);
private:
  uint32_t _adr,_idx,_chlines;
  uint8_t _buf[MBSIZE];
  

  uint64_t _lastABCID;
  uint32_t _lastGTC,_event,_detid,_id,_ntrg,_expectedLength;
  uint32_t _slcStatus;
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
