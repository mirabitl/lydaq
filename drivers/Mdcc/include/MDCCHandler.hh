#ifndef _MDCCHandler_h_
#define _MDCCHandler_h_

#include "LocalHardwareException.hh"

#define USE_FTDI
#ifdef USE_FTDI

#include "FtdiUsbDriver.hh"

#endif
#include "ReadoutLogger.hh"
namespace lydaq
{
  class MDCCHandler 
  {
  public:
    MDCCHandler (std::string name,uint32_t productid=0x6001);
    ~MDCCHandler();
    void open();
    void close();
    void writeRegister(uint32_t addr,uint32_t data);
    uint32_t readRegister(uint32_t addr);
    void maskTrigger();
    void unmaskTrigger();
    void maskEcal();
    void unmaskEcal();
    void resetCounter();
    void resetTDC(uint8_t b);
    uint32_t mask();
    uint32_t ecalmask();
    uint32_t spillCount();
    uint32_t busyCount(uint8_t b);
    uint32_t spillOn();
    uint32_t spillOff();
    void setSpillOn(uint32_t nc);
    void setSpillOff(uint32_t nc);
    uint32_t beam();
    void setBeam(uint32_t nc);

    uint32_t version();
    uint32_t id();

    void calibOn();
    void calibOff();
    void reloadCalibCount();
    uint32_t calibCount();
    void setCalibCount(uint32_t nc);


    void setSpillRegister(uint32_t nc);
    uint32_t spillRegister();
    void useSPSSpill(bool t);
    void useTrigExt(bool t);
    void setTriggerDelay(uint32_t nc);
    uint32_t triggerDelay();
    void setTriggerBusy(uint32_t nc);
    uint32_t triggerBusy();

  
  private : 
    lydaq::FtdiUsbDriver* _driver;
    std::string _name;
    uint32_t _productid,_version,_id;

  };
};
#endif
