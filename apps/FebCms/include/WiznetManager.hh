#ifndef _WIZNET_MANAGER_HH
#define _WIZNET_MANAGER_HH
#include "WiznetInterface.hh"
#include "TdcWiznet.hh"
#include "TdcConfigAccess.hh"
#include "baseApplication.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"

/*!
* \file WiznetManager.hh
 * \brief Main Zdaq baseAPplication to interavt with FEBS
 * \author L.Mirabito
 * \version 1.0
*/

namespace lydaq
{
/**
   * \class WiznetManager
   * \brief zdaq::baseApplication to interact with FEBs
   * 
   *  State Machine Implementation
   *  VOID,CREATED,INITIALISED,CONFIGURED,RUNNING
   * */
class WiznetManager : public zdaq::baseApplication
{
public:
  ///Constructor
  WiznetManager(std::string name);
  /// Empty destructor
  ~WiznetManager() { ; }
  /// INITIALISE  handler
  void initialise(zdaq::fsmmessage *m);
  /// CONFIGURE  handler
  void configure(zdaq::fsmmessage *m);
  /// START  handler
  void start(zdaq::fsmmessage *m);
  /// STOP  handler
  void stop(zdaq::fsmmessage *m);
  /// DESTROY  handler
  void destroy(zdaq::fsmmessage *m);
  /// job log command  (obsolete)
  void c_joblog(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// STATUS Command handler
  void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// DIFLIST Command handler
  void c_diflist(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SET6BDAC Command handler
  void c_set6bdac(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETMASK Command handler
  void c_setMask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETMODE Command handler
  void c_setMode(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETDELAY Command handler
  void c_setDelay(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETDURATION Command handler
  void c_setDuration(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETVTHTIME Command Handler
  void c_setvthtime(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// SETONEVTHTIME Command handler
  void c_set1vthtime(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// DOWNLOADDB Command handler
  void c_downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// GETLUT command handler
  void c_getLUT(Mongoose::Request &request, Mongoose::JsonResponse &response);
  /// TDC Calibration  command handlers
  void c_getCalibrationStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setCalibrationMask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void c_setMeasurementMask(Mongoose::Request &request, Mongoose::JsonResponse &response);

  /// FEB register access
  void writeAddress(std::string host, uint32_t port, uint16_t addr, uint16_t val);

  void writeLongWord(std::string host, uint32_t port, uint16_t addr, uint64_t val);

  /// Change 6BDAC (all FEBs,all Asics)
  void set6bDac(uint8_t dac);
  /// Change Mask (all febs, ASIC mask)
  void setMask(uint32_t mask, uint8_t asic = 255);
  /// Sof trigger (obsolete)
  void sendTrigger(uint32_t nt);
  /// Change VTHTIME (all FEBS)
  void setVthTime(uint32_t dac);
  /// Change VTHTIME (FEB and asic specified)
  void setSingleVthTime(uint32_t vth, uint32_t feb, uint32_t asic);
  /// Change Dead time
  void setDelay();
  /// Change active time
  void setDuration();
  /// Reuqires LUT of one TDC channel
  void getLUT(int chan);
  /// Get calibration status bits
  void getCalibrationStatus();
  /// Set Calibration Mask
  void setCalibrationMask(uint64_t mask);
  /// Set measurement mask
  void setMeasurementMask(uint64_t mask);
  /// Shm read
  void readShm(std::string host,uint32_t port);
  /// controlSize
  int32_t getControlSize(){return _controlSize;}
  inline void setControlSize(int32_t s){_controlSize=s;}
  /// controlData
  inline uint8_t* getControlData(){return _controlData;}

private:
  lydaq::TdcConfigAccess *_tca;
  lydaq::WiznetInterface *_wiznet;
  lydaq::WiznetMessage *_msg;

  std::vector<lydaq::TdcWiznet *> _vTdc;

  zdaq::fsmweb *_fsm;
  uint32_t _run, _type;
  uint8_t _delay;
  uint8_t _duration;

  zmq::context_t *_context;

  int32_t _controlSize;
  uint8_t _controlData[0x40000];
  Json::Value _jControl;
};
}; // namespace lydaq
#endif
