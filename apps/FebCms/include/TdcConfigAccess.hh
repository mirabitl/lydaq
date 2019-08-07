#ifndef _TDC_CONFIG_ACCESS_HH
#define _TDC_CONFIG_ACCESS_HH
/*!
* \file TdcConfigAccess.hh
 * \brief Method to fill PRSLow object from file or DB
 * \author L.Mirabito
 * \version 1.0
*/
#define USE_PR2B
//#define USE_PR2A

#ifdef USE_PR2A
#include "PRSlow.hh"
#define PR2 PRSlow
#else
#include "PRBSlow.hh"
#define PR2 PRBSlow
#endif
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <map>
#include "ReadoutLogger.hh"
namespace lydaq
{
/**
   * \brief Main class to access (DB or file) the PETIROC2A parameters
   * */
class TdcConfigAccess
{
public:
  /// Constructor
  TdcConfigAccess();
  /// Destructor
  ~TdcConfigAccess() { ; }
  /// Parse a JSON file
  void parseJsonFile(std::string jsf);
  /// Fill buffers with JSON parsed data
  void parseJson();
  /// Parse a JSON url
  void parseJsonUrl(std::string jsf);
  /// Download and parse MongoDb version
  void parseMongoDb(std::string state,uint32_t version);

  /// FEB specific slow control buffers
  uint16_t *slcBuffer();
  uint16_t *slcAddr();
  uint32_t slcBytes();
  /// map id - ASIC where id = (IP<<32| asic  header)
  std::map<uint64_t, lydaq::PR2> &asicMap();
  void clear();
  void dumpMap();
  /// Fill FEB buffer for one FEB (ip address)
  void prepareSlowControl(std::string ipadr);
  /// Fill map from DB
  void parseDb(std::string state, std::string mode);
  /// Write Slow control buffer in /dev/shm
  void dumpToShm(std::string path);
  /// Not implemented
  void connect();
  /// Not implemented
  void publish();

private:
  uint16_t _slcBuffer[0x1000];
  uint16_t _slcAddr[0x1000];
  uint32_t _slcBytes;
  std::map<uint64_t, lydaq::PR2> _asicMap;


  Json::Value _jall, _jasic;
};
}; // namespace lydaq
#endif
