#ifndef _FtdiUsbDriver_H
#define _FtdiUsbDriver_H


#include <iomanip>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdint.h>

// hardware access
#include "LocalHardwareException.hh"
#include <ftdi.h>
#include <string.h>
#include "ReadoutLogger.hh"

namespace lydaq
{
class FtdiUsbDriver {
public:
  FtdiUsbDriver(char * deviceIdentifier ,uint32_t productid=0x6001)     throw (LocalHardwareException);
	~FtdiUsbDriver()     throw (LocalHardwareException);
	void checkReadWrite(uint32_t start,uint32_t count) throw (LocalHardwareException);
	void FT245Purge( void ) throw (LocalHardwareException);
	int32_t read( unsigned char  *resultPtr ) throw (LocalHardwareException);
	void readn( unsigned char  *resultPtr,int32_t nbbytes )throw (LocalHardwareException);
	void write( unsigned char  data) throw (LocalHardwareException);
	void MonWritenAmoi( unsigned char  *cdata, uint32_t nb) throw (LocalHardwareException);
	void readEEPROM( uint32_t address, 	uint32_t *resultPtr ) throw (LocalHardwareException);
	void writeEEPROM( 	uint32_t address,	uint32_t data) throw (LocalHardwareException);
	void resetEEPROM( 	void) throw (LocalHardwareException);
	void resetBus( ) throw (LocalHardwareException);
	void readStatus( uint32_t *RXQueue, uint32_t *TXQueue, uint32_t *Event) throw (LocalHardwareException); //Dummy 
	int32_t UsbGetFirmwareRevision(uint32_t *version)  throw (LocalHardwareException);
	int32_t SetTestRegister(int32_t tvalue)    throw (LocalHardwareException);
	int32_t GetTestRegister(uint32_t *tvalue)    throw (LocalHardwareException);
	int32_t UsbReadByte(unsigned char  *tbyte) throw (LocalHardwareException);
	int32_t UsbRead4Bytes(uint32_t *data) throw (LocalHardwareException);
	int32_t UsbRead16Bytes(unsigned char  *data) throw (LocalHardwareException);
	int32_t UsbRead22Bytes(unsigned char  *data) throw (LocalHardwareException);
	int32_t UsbReadnBytes(unsigned char  *data, int32_t nbbytes) throw (LocalHardwareException);
	int32_t UsbRead3Bytes(uint32_t *data) throw (LocalHardwareException);
	int32_t UsbRead2Bytes(uint32_t *data) throw (LocalHardwareException);
	int32_t UsbRegisterRead(uint32_t address, uint32_t *data) throw (LocalHardwareException);
	int32_t UsbRegisterRead2(uint32_t address, uint32_t *data) throw (LocalHardwareException);
	int32_t UsbRegisterWrite(uint32_t address, uint32_t data) throw (LocalHardwareException);
	int32_t UsbRegisterWrite2(uint32_t address, uint32_t data) throw (LocalHardwareException);
	int32_t UsbCommandWrite(uint32_t command) throw (LocalHardwareException);
	int32_t  FT245GetStatus(int32_t *RXQueue,int32_t *TXQueue ,int32_t *Event)    throw (LocalHardwareException);
	int32_t FT245Reset(void) throw (LocalHardwareException);

	static uint16_t CrcTable[256];
protected:
	struct ftdi_context theFtdi;
	uint32_t timeOut;
	uint32_t theProduct_;
        char theName_[12];
};
};
#endif
