#ifndef _LDIF_h

#define _LDIF_h
#include <iostream>

#include <string.h>
#include<stdio.h>
#include "zmPusher.hh"
#include "DIFReadout.hh"
#include "DIFReadoutConstant.hh"
using namespace std;
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>
#include "ReadoutLogger.hh"
typedef struct 
{
  uint32_t vendorid;
  uint32_t productid;
  char name[12];
  uint32_t id;
  uint32_t type;
} FtdiDeviceInfo;
namespace lydaq {
class LDIF
{
public:
  LDIF(FtdiDeviceInfo *ftd);
  ~LDIF();
  // registration to Dim Info and creation of DimServices 
  void registration();
  void setTransport(zdaq::zmPusher* p);
  // initialise
  void initialise(zdaq::zmPusher* p=NULL);
  // configure
  //void difConfigure(uint32_t ctrl);
  void difConfigure(uint32_t ctrlreg,uint32_t p2pa=0x4e,uint32_t pa2pd=0x3e6,uint32_t pd2daq=0x4e,uint32_t daq2dr=0x4e,uint32_t d2ar=0x4e);


  
  void chipConfigure();
  void configure(uint32_t ctrl,uint32_t l1=0x4e,uint32_t l2=0x3e6,uint32_t l3=0x4e,uint32_t l4=0x4e,uint32_t l5=0x4e);
  //void configure(uint32_t ctrl,uint32_t l1=0,uint32_t l2=0,uint32_t l3=0,uint32_t l4=0,uint32_t l5=0);
  // Start Stop
  void start();
  void readout();
  void stop();
  // destroy
  void destroy();
  // register access
  void writeRegister(uint32_t adr,uint32_t reg);
  void readRegister(uint32_t adr,uint32_t &reg);
  // Getter and setters
  inline DIFStatus* status() const {return _status;}
  inline lydaq::DIFReadout* rd() const {return _rd;}
  inline DIFDbInfo* dbdif() const {return _dbdif;}
  void setState(std::string s){_state.assign(s);}
  inline std::string state() const {return _state;}
  inline uint32_t* data()  {return (uint32_t*) _dsData->buffer()->ptr();}
  
  // run control
  inline void setReadoutStarted(bool t){_readoutStarted=t;}
  inline bool readoutStarted() const { return _readoutStarted;}
  inline bool running() const { return _running;}
  inline uint32_t detectorId() const {return _detid;}
  inline void publishState(std::string s){setState(s);}
  //Threshold and gain
  void setThreshold(uint32_t B0,uint32_t B1,uint32_t B2,SingleHardrocV2ConfigurationFrame& ConfigHR2);
  void setGain(uint32_t gain,SingleHardrocV2ConfigurationFrame& ConfigHR2);
  void setThreshold(uint32_t B0,uint32_t B1,uint32_t B2);
  void setGain(uint32_t gain);

  static uint32_t getBufferDIF(unsigned char* cb,uint32_t idx=0);
  static uint32_t getBufferDTC(unsigned char* cb,uint32_t idx=0);
  static uint32_t getBufferGTC(unsigned char* cb,uint32_t idx=0);
  static unsigned long long getBufferABCID(unsigned char* cb,uint32_t idx=0);
private:
  FtdiDeviceInfo _ftd;
  DIFStatus* _status;
  lydaq::DIFReadout* _rd;
  std::string _state;
  DIFDbInfo* _dbdif;
  uint32_t _data[32768];

  zdaq::zmPusher* _dsData;
  uint32_t _detid;
  bool _running,_readoutStarted,_readoutCompleted;
};
};
#endif
