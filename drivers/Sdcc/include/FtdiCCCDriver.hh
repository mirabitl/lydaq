/************************************************************************/
/* ILC test beam daq							*/
/* interface for DIF with hardroc chip V1					*/
/* C. Combaret								*/
/* V 1.0								*/
/* first release : 30-01-2008						*/
/* revs : 								*/
/***********************************************************************/


#ifndef _FtdiCCCDriver_H
#define _FtdiCCCDriver_H


#include <iomanip>

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <stdint.h>

// hardware access
#include "LocalHardwareException.hh"
#include "FtdiUsbDriver.hh"

#include <ftdi.h>
#include <string.h>



#define FLUSH_TO_FILE 		// when flush function is sed , data are sent to a file

namespace lydaq
{
  class FtdiCCCDriver : public lydaq::FtdiUsbDriver
  {
  public:
    
    FtdiCCCDriver(	char * deviceIdentifier)     throw (LocalHardwareException);
    
    virtual ~FtdiCCCDriver()     throw (LocalHardwareException);
    
    
    // register access
    
    
    int32_t CCCCommandDIFReset(void)    																				throw (LocalHardwareException);
    int32_t CCCCommandBCIDReset(void)     																			throw (LocalHardwareException);
    int32_t CCCCommandStartAcquisitionAuto(void)     														throw (LocalHardwareException);
    int32_t CCCCommandRamfullExt(void)     																			throw (LocalHardwareException);
    int32_t CCCCommandTriggerExt(void)     																			throw (LocalHardwareException);
    int32_t CCCCommandStopAcquisition(void)     																throw (LocalHardwareException);
    int32_t CCCCommandDigitalReadout(void)    																	throw (LocalHardwareException);
    int32_t CCCCommandTrigger(void)    																					throw (LocalHardwareException);
    int32_t CCCCommandClearMemory(void)    																			throw (LocalHardwareException);
    int32_t CCCCommandStartSingleAcquisition(void)    													throw (LocalHardwareException);
    int32_t CCCCommandPulseLemo(void)    																			 	throw (LocalHardwareException);
    int32_t CCCCommandRazChannel(void)  																		   	throw (LocalHardwareException);
    //		int32_t CCCCommandNoCommand(void)   																				throw (LocalHardwareException);
    int32_t CCCCommandResetCCC(void)    																		 		throw (LocalHardwareException);
    int32_t CCCCommandSpillOn(void)    																					throw (LocalHardwareException);
    int32_t CCCCommandSpillOff(void)    																					throw (LocalHardwareException);
	
  protected:
    struct ftdi_context theFtdi;
    uint32_t timeOut;
	
				
  };
};
#endif
