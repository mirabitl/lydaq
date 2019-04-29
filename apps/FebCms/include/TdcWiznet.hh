#ifndef _TDCWIZNET_HH
#define _TDCWIZNET_HH
#include "zmPusher.hh"
#define TDC_VERSION 130
#include <iostream>
#include <list>
#include <vector>
#include <stdint.h>
#include "debug.hh"
#include "ReadoutLogger.hh"
/*!
* \file TdcWiznet.hh
 * \brief Buffer processing and interface to the event builder
 * \author L.Mirabito
 * \version 1.0
*/

using namespace std;
namespace lydaq
{
/**
   * \class TdcWiznet
   * \brief Process incomming buffer and forward to event builder pre-formatted data
   * */
class TdcWiznet
{
public:
  /// Constructor (adr=ip address in integer)
  TdcWiznet(uint32_t adr);
  /// FEBFunctor handler
  void processBuffer(uint64_t id, uint16_t l, char *b);
  /// Slow control processing
  void processSlc(uint64_t id, uint16_t l, char *b);
  /// Data packet processing
  bool processPacket();
  /// Obsolete , remove unused packet
  void purgeBuffer();
  /// Send FEB buffer as lines
  void processEventTdc();
  /// Check buffer type
  int16_t checkBuffer(uint8_t *b, uint32_t maxidx);
  /// Set Trigger Type (130 for FEB)
  inline void setTriggerId(uint8_t i) { _triggerId = i; }
  /// get detector id
  inline uint32_t detectorId() { return _detid; }
  /// get source id
  inline uint32_t difId() { return _id; }
  /// get Absolute BCID
  inline uint64_t abcid() { return _lastABCID; }
  /// Get Global Trigger Counter
  inline uint32_t gtc() { return _lastGTC; }
  /// Get number of packets processed
  inline uint32_t packets() { return _nProcessed; }
  /// Event number
  inline uint32_t event() { return _event; }
  /// Triggers
  inline uint32_t triggers() { return _ntrg; }
  /// clear buffers
  void clear();
  /// create and connect zdaq:zmPusher
  void connect(zmq::context_t *c, std::string dest);

private:
  uint32_t _adr, _idx, _chlines;
  uint8_t _buf[0x1000000];
  uint8_t _linesbuf[0x1000000];

  uint64_t _lastABCID;
  uint32_t _lastGTC, _event, _detid, _id, _ntrg, _expectedLength;
  uint32_t _nProcessed;
  zdaq::zmPusher *_dsData;
  uint8_t _triggerId;
};
}; // namespace lydaq

#endif
