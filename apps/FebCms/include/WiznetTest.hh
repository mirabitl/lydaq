#ifndef _WIZNET_TEST_HH
#define _WIZNET_TEST_HH

#include "WiznetInterface.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"
#include "baseApplication.hh"

namespace lydaq
{

  class WiznetTest : public zdaq::baseApplication
{
public:
  WiznetTest(std::string address,uint16_t portslc,uint16_t porttdc);
  void c_configure(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_mode(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_start(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_stop(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_packet(Mongoose::Request &request, Mongoose::JsonResponse &response);

  void processBuffer(uint64_t id, uint16_t l,char* b);
  void processSlc(uint64_t id, uint16_t l,char* b);
  bool processPacket();   
  void initialise();
  void start(uint16_t nc=1);
  void mode(uint16_t nc=0);
  void stop();
  void configure(std::string name);
  inline bool isStart(uint16_t *b) {return (*b)==0XCAFE;}
  inline bool isEnd(uint16_t *b) {return (*b)==0XEFAC;}
private:
  std::string _address;
  uint16_t _portslc;
  uint16_t _porttdc;
  lydaq::WiznetInterface* _wiznet;
  lydaq::WiznetMessage* _msg;

  int32_t _cpos;
  uint8_t _buf[128*1024],_cpacket[128*1024];
  uint16_t _idx;
  uint32_t _expectedLength,_packetNb,_chlines;
  
  uint16_t* _sBuf;
  uint32_t* _lBuf;
  uint32_t _nProcessed,_lastGTC,_event;
  uint64_t _lastABCID;
};
};
#endif
