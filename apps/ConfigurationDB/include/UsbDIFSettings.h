#ifndef _UsbDIFSettings_H
#define _UsbDIFSettings_H
#include <stdint.h>
#include <stdio.h>
#include <iostream>
/**
 * \class UsbSettings
 * \author L.Mirabito
 * \date 2012-02-01 
 * \brief Contains Slow Control DIF settings. SInce no methods is implemented all data are public 
 */
class UsbDIFSettings
{
 public:
 UsbDIFSettings(int32_t difid) :   AVDDShdn(1),
  CHAMBER_ID(0),
  DCC_CHANNEL(0),
  DIFIMonGain(50),
  DVDDShdn(1),
  EnableMonitoring(0),
  FIRMWARE("Rev0"),
  HARDWARE("Rev0"),
  HW_TYPE(2),
  ID(0),
  LDA_ADDRESS("ff:ff:ff:ff:ff:ff"),
  LDA_CHANNEL(0),
  MASK(0),
  MemoryDisplayLimitMax(10),
  MemoryDisplayLimitMin(0),
  MemoryEfficiencyLimitMax(10),
  MemoryEfficiencyLimitMin(0),
  MonitorSequencer(1),
  MonitoredChannel(3),
  NAME("NONE"),
  NUM(difid),
  NumericalReadoutMode(1),
  NumericalReadoutStartMode(0),
  PowerADC(1),
  PowerAnalog(1),
  PowerDAC(1),
  PowerDigital(1),
  PowerSS(1),
  SETUP_ID(0),
  SlabIMonGain(50),
    TimerHoldRegister(5),
    Masked(false)

    {
      char name[256];
      sprintf(name,"FT101%.3d",difid);
      NAME.assign(name);
      std::cout<<"DIF created "<<NAME<<std::endl;
    }




  int32_t AVDDShdn;
  int32_t CHAMBER_ID;
  int32_t DCC_CHANNEL;
  int32_t DIFIMonGain;
  int32_t DVDDShdn;
  int32_t EnableMonitoring;
  std::string FIRMWARE;
  std::string HARDWARE;
  int32_t HW_TYPE;
  int32_t ID;
  std::string LDA_ADDRESS;
  int32_t LDA_CHANNEL;
  int32_t MASK;
  int32_t MemoryDisplayLimitMax;
  int32_t MemoryDisplayLimitMin;
  int32_t MemoryEfficiencyLimitMax;
  int32_t MemoryEfficiencyLimitMin;
  int32_t MonitorSequencer;
  int32_t MonitoredChannel;
  std::string NAME;
  int32_t NUM;
  int32_t NumericalReadoutMode;
  int32_t NumericalReadoutStartMode;
  int32_t PowerADC;
  int32_t PowerAnalog;
  int32_t PowerDAC;
  int32_t PowerDigital;
  int32_t PowerSS;
  int32_t SETUP_ID;
  int32_t SlabIMonGain;
  int32_t TimerHoldRegister;
  bool Masked;


};
#endif  
  
