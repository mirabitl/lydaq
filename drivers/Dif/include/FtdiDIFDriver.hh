#ifndef _FtdiDIFDriver_H
#define _FtdiDIFDriver_H

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
#include "ReadoutLogger.hh"

namespace lydaq
{
  class FtdiDIFDriver : public lydaq::FtdiUsbDriver
  {
  public:
    FtdiDIFDriver(char * deviceIdentifier,uint32_t productid=0x6001)     ;//throw (LocalHardwareException)
    int32_t NbAsicsWrite(uint32_t tnumber,uint32_t l1,uint32_t l2,uint32_t l3,uint32_t l4 )    ;//throw (LocalHardwareException)
    int32_t SetControlRegister(int32_t tvalue)    ;//throw (LocalHardwareException)
    int32_t GetControlRegister(uint32_t *tvalue)    ;//throw (LocalHardwareException)
    int32_t  HardrocGetTimerHoldRegister(uint32_t *thold)    ;//throw (LocalHardwareException)
    int32_t HardrocSetNumericalReadoutMode(int32_t tmode)    ;//throw (LocalHardwareException)
    int32_t HardrocGetNumericalReadoutMode(int32_t *tmode)    ;//throw (LocalHardwareException)
    int32_t HardrocSetNumericalReadoutStartMode(int32_t tmode)    ;//throw (LocalHardwareException)
    int32_t HardrocGetNumericalReadoutStartMode(int32_t *tmode)    ;//throw (LocalHardwareException)
    int32_t HardrocSetSCOverVoltageDefault(void) ;//throw (LocalHardwareException)
    int32_t HardrocGetSCOverVoltage(int32_t *tmode) ;//throw (LocalHardwareException)
    int32_t HardrocSetTestAllAsicsDefault(void)    ;//throw (LocalHardwareException)
    int32_t HardrocGetTestAllAsics(int32_t *tmode)    ;//throw (LocalHardwareException)
    int32_t HardrocSetEnablePowerPulsing(int32_t tmode)    ;//throw (LocalHardwareException)
    int32_t HardrocGetEnablePowerPulsing(int32_t *tmode)    ;//throw (LocalHardwareException)
    int32_t HardrocSetEnableTimeoutDigitalReadout(int32_t tmode)    ;//throw (LocalHardwareException)
    int32_t HardrocGetEnableTimeoutDigitalReadout(int32_t *tmode)    ;//throw (LocalHardwareException)
    int32_t HardrocGetStatusRegister(uint32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t HardrocGetMemFull(uint32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t HardrocGetRamFullCpt(uint32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t HardrocSetSCDebugRegister(int32_t tvalue)    ;//throw (LocalHardwareException)
    int32_t HardrocGetSCDebugRegister(uint32_t *tvalue)    ;//throw (LocalHardwareException)
    int32_t SetChipTypeRegister(int32_t tvalue)    ;//throw (LocalHardwareException)
    int32_t GetChipTypeRegister(uint32_t *tvalue)    ;//throw (LocalHardwareException)
    int32_t SetResetCounter(int32_t tvalue)    ;//throw (LocalHardwareException)
    int32_t GetResetCounter(uint32_t *tvalue)    ;//throw (LocalHardwareException)

    int32_t SetEventsBetweenTemperatureReadout(uint32_t tdata)    ;//throw (LocalHardwareException)
    int32_t GetEventsBetweenTemperatureReadout(uint32_t *tdata)    ;//throw (LocalHardwareException)
    int32_t SetAnalogConfigureRegister(uint32_t tdata)    ;//throw (LocalHardwareException)

    int32_t  SetPwrToPwrARegister(uint32_t tnumber)    ;//throw (LocalHardwareException)
    int32_t  SetPwrAToPwrDRegister(uint32_t tnumber)    ;//throw (LocalHardwareException)
    int32_t  SetPwrDToDAQRegister(uint32_t tnumber)    ;//throw (LocalHardwareException)
    int32_t  SetDAQToPwrDRegister(uint32_t tnumber)    ;//throw (LocalHardwareException)
    int32_t  SetPwrDToPwrARegister(uint32_t tnumber)    ;//throw (LocalHardwareException)
    int32_t  HardrocCommandAskDifTemperature(void)    ;//throw (LocalHardwareException)
    int32_t  HardrocCommandAskAsuTemperature(void)    ;//throw (LocalHardwareException)
    int32_t  GetDIFTemperature(uint32_t *tvalue)    		 ;//throw (LocalHardwareException)
    int32_t  SetTemperatureReadoutToAuto(uint32_t tvalue)    	 ;//throw (LocalHardwareException)
    int32_t  GetTemperatureReadoutAutoStatus(uint32_t *tvalue)    	 ;//throw (LocalHardwareException)
    int32_t  GetASUTemperature(uint32_t *tvalue1,uint32_t *tvalue2)    						;//throw (LocalHardwareException)
    int32_t UsbSetDIFID(uint32_t tnumber)    ;//throw (LocalHardwareException)
    int32_t GetDIFID(uint32_t *tnumber)    ;//throw (LocalHardwareException)
    int32_t HardrocSetGeneratorDivision(uint32_t tnumber)    ;//throw (LocalHardwareException)
    int32_t NbAsicsRead(uint32_t *tnumber)    ;//throw (LocalHardwareException)
    int32_t HardrocPwonDacDelayRead(uint32_t *tnumber)    ;//throw (LocalHardwareException)
    int32_t HardrocPwonDacDelayWrite(uint32_t tnumber)    ;//throw (LocalHardwareException)
    int32_t HardrocPwonAEndDelayRead(uint32_t *tnumber)    ;//throw (LocalHardwareException)
    int32_t HardrocPwonAEndDelayWrite(uint32_t tnumber)    ;//throw (LocalHardwareException)
    int32_t HardrocSLCStatusRead(uint32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t HardrocSLCCRCStatusRead(void)    ;//throw (LocalHardwareException)
    int32_t HardrocSLCLoadStatusRead(void)    ;//throw (LocalHardwareException)
    int32_t DIFMonitoringEnable(int32_t status)   ;//throw (LocalHardwareException)
    int32_t DIFMonitoringSetDIFGain (int32_t gain)   ;//throw (LocalHardwareException)
    int32_t DIFMonitoringSetSlabGain(int32_t gain)   ;//throw (LocalHardwareException)
    int32_t DIFMonitoringSetSequencer(int32_t status)   ;//throw (LocalHardwareException)
    int32_t DIFMonitoringSetAVDDshdn (int32_t status)   ;//throw (LocalHardwareException)
    int32_t DIFMonitoringSetDVDDshdn (int32_t status)   ;//throw (LocalHardwareException)
    int32_t DIFMonitoringSetConvertedChannels (int32_t channel)   ;//throw (LocalHardwareException)
    int32_t DIFMonitoringGetConfigRegister(uint32_t *status)   ;//throw (LocalHardwareException)
    int32_t DIFMonitoringGetDIFCurrent(uint32_t *DIFCurrent)    ;//throw (LocalHardwareException)
    int32_t DIFMonitoringGetSlabCurrent(uint32_t *SlabCurrent)    ;//throw (LocalHardwareException)
    int32_t DIFMonitoringGetChannel4Monitoring(uint32_t *Ch4Value)    ;//throw (LocalHardwareException)
    int32_t HardrocCommandSLCWrite(void)    ;//throw (LocalHardwareException)
    int32_t HardrocCommandSLCWriteLocal(void)    ;//throw (LocalHardwareException)
    int32_t HardrocCommandSLCWriteByte(unsigned char  tbyte)    ;//throw (LocalHardwareException)
    int32_t HardrocCommandSLCWriteCRC(unsigned char  *tbyte)    ;//throw (LocalHardwareException)
    int32_t HardrocCommandLemoPulse(void) ;//throw (LocalHardwareException)
    int32_t FT245Reset(void) ;//throw (LocalHardwareException)
    int32_t FPGAReset(void) ;//throw (LocalHardwareException)
    int32_t HardrocReset(void) ;//throw (LocalHardwareException)
    int32_t BCIDReset(void)   ;//throw (LocalHardwareException)
    int32_t SCReset(void) ;//throw (LocalHardwareException)
    int32_t SCSRReset(void) ;//throw (LocalHardwareException)
    int32_t SRReset(void) ;//throw (LocalHardwareException)
    int32_t SCReportReset(void) ;//throw (LocalHardwareException)
    int32_t DIFCptReset(void) ;//throw (LocalHardwareException)
    int32_t SetPowerAnalog(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetPowerAnalog(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetPowerADC(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetPowerADC(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetPowerSS(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetPowerSS(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetPowerDigital(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetPowerDigital(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetPowerDAC(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetPowerDAC(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t ResetCounter(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t ClearAnalogSR(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t SetSCChoice(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetSCChoice(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetCalibrationMode(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetCalibrationMode(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetSetupWithCCC(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetSetupWithCCC(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetSetupWithDCC(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetSetupWithDCC(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetAcqTest(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetAcqTest(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t Set4VforSC(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t Get4VforSC(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetMode4VforSC(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetMode4VforSC(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetModeDCCCCC(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetModeDCCCCC(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetHold(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetHold(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetTimeoutDigitalReadout(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetTimeoutDigitalReadout(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetPowerPulsing(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetPowerPulsing(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetRealPowerPulsing(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetRealPowerPulsing(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetDIFCommandsONOFF(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetDIFCommandsONOFF(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetDROBtMode(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetDROBtMode(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetClockFrequency(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetClockFrequency(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t HardrocSetPowerPulsing(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t HardrocGetPowerPulsing(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t SetSCClockFrequency(int32_t tstatus)    ;//throw (LocalHardwareException)
    int32_t GetSCClockFrequency(int32_t *tstatus)    ;//throw (LocalHardwareException)
    int32_t HardrocStartDigitalAcquisitionCommand(void)    ;//throw (LocalHardwareException)
    int32_t HardrocStopDigitalAcquisitionCommand(void)    ;//throw (LocalHardwareException)
    int32_t HardrocStartDigitalReadoutCommand(void)    ;//throw (LocalHardwareException)
    int32_t HardrocSendRamfullExtCommand(void)    ;//throw (LocalHardwareException)
    int32_t HardrocSendExternalTriggerCommand(void)    ;//throw (LocalHardwareException)
    int32_t HardrocSendMezzanine11Command(void)    ;//throw (LocalHardwareException)
    int32_t  HardrocSetTimerHoldRegister(int32_t thold)    ;//throw (LocalHardwareException)
    int32_t  HardrocStartAnalogAcq(void)    ;//throw (LocalHardwareException)
    int32_t  HardrocSoftwareTriggerAnalogAcq(void)    ;//throw (LocalHardwareException)
    int32_t  DIFMonitoringGetTemperature(uint32_t *Temperature) ;//throw (LocalHardwareException)
    int32_t CommandSLCWriteSingleSLCFrame(unsigned char  *tbyte,uint32_t n) ;//throw (LocalHardwareException)
    int32_t HardrocFlushDigitalFIFO(void);
    int32_t HardrocFastFlushDigitalFIFO(void);
  };
};
#endif

