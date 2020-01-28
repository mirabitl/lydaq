#ifndef _PmrDriver_H
#define _PmrDriver_H


#include <iomanip>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdint.h>

// hardware access
#include <ftdi.h>
#include <string.h>
#include "ReadoutLogger.hh"


#define PMR_TEST_REG 0x00
#define PMR_ID_REG 0x01
#define PMR_NBASIC_REG 0x02
#define PMR_SLC_DATA_REG 0x10
#define PMR_SLC_CONTROL_REG 0x11
#define PMR_SLC_STATUS_REG 0x12
#define PMR_SLC_SIZE_REG 0x13
#define PMR_RO_DATA_FORMAT_REG 0x31
#define PMR_RO_CONTROL_REG 0x30
#define PMR_RO_RESET_REG 0x32
#define PMR_PP_AN_2_DIG_REG 0x21
#define PMR_PP_DIG_2_ACQ_REG 0x22
#define PMR_PP_ACQ_2_DIG_REG 0x23
#define PMR_PP_DIG_2_AN_REG 0x24
#define PMR_PP_CONTROL_REG 0x25

namespace lydaq
{
class PmrDriver {
public:
  PmrDriver(char * deviceIdentifier ,uint32_t productid=0x6001);
  ~PmrDriver();
  int32_t open(char * deviceIdentifier, uint32_t productid );
  int32_t writeNBytes(unsigned char  *cdata, uint32_t nb);
  int32_t readNBytes(unsigned char  *cdata, uint32_t nb);
  int32_t registerWrite(uint32_t address, uint32_t data);
  int32_t registerRead(uint32_t address, uint32_t *data);
  int32_t setup();
  int32_t loadSLC(unsigned char* SLC,uint32_t size);
  int32_t setPowerPulsing(bool enable=false,uint32_t an2d=0,uint32_t d2ac=0,uint32_t ac2d=0,uint32_t d2an=0);
  int32_t setAcquisitionMode(bool active=true,bool autoreset=true);
  int32_t resetFSM();
  int32_t readData(unsigned char* tro,uint32_t size);
protected:
  struct ftdi_context theFtdi;
  uint32_t _productId;
  char _deviceId[12];
};
};
#endif
