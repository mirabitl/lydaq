#ifndef _CCCReadout_h_
#define _CCCReadout_h_
//#include "UsbCCCDriver.h"
#include "LocalHardwareException.hh"

#define USE_FTDI
#undef USE_FTD2XX


#ifdef USE_FTDI
#undef USE_FTD2XX
#undef SERIAL_FTDI
#include "FtdiCCCDriver.hh"
#define USBDRIVER lydaq::FtdiCCCDriver
#endif
namespace lydaq
{
  class CCCReadout 
  {
  public:
    CCCReadout (std::string name);
    virtual int open() ;//throw( LocalHardwareException ) ;
    virtual int close() ;//throw( LocalHardwareException );

  private : 
    USBDRIVER* theDriver_;
    std::string theName_;
  public : 
    virtual void DoSendDIFReset();
    virtual void DoSendBCIDReset();
    virtual void DoSendStartAcquisitionAuto();
    virtual void DoSendRamfullExt();
    virtual void DoSendTrigExt();
    virtual void DoSendStopAcquisition();
    virtual void DoSendDigitalReadout();
    virtual void DoSendClearMemory();
    virtual void DoSendStartSingleAcquisition();
    virtual void DoSendPulseLemo();
    virtual void DoSendRazChannel();
    virtual void DoSendTrigger();
    virtual void DoSendCCCReset();
    virtual void DoSendSpillOn();
    virtual void DoSendSpillOff();
    virtual void DoWriteRegister(uint32_t addr,uint32_t data);
    virtual uint32_t DoReadRegister(uint32_t addr);
    virtual void DoSendPauseTrigger();
    virtual void DoSendResumeTrigger();
  };
};
#endif
