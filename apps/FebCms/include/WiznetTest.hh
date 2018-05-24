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

namespace lydaq
{

  class WiznetTest 
{
public:
  WiznetTest(std::string address,uint16_t portslc,uint16_t porttdc);
  void processBuffer(uint16_t l,char* b);
  void processPacket();   
  void initialise();
  void start();
  void stop();
  void endOfBuffer();
  inline bool isStart(uint16_t *b) {return (*b)==0XCAFE;}
  inline bool isEnd(uint16_t *b) {return (*b)==0XEFAC;}
private:
  std::string _address;
  uint16_t _portslc;
  uint16_t _porttdc;
  lydaq::WiznetInterface* _wiznet;
  lydaq::WiznetMessage* _msg;

  uint8_t _buf[32*1024];
  uint8_t* _payload;
  uint16_t _idx;
  uint16_t _currentLength,_packetNb;
  
  uint16_t* _sBuf;
  uint32_t* _lBuf;
};
};
#endif
