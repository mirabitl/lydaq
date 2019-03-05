#ifndef _HR2_CONFIG_ACCESS_HH
#define _HR2_CONFIG_ACCESS_HH
#include "HR2Slow.hh"
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
class HR2ConfigAccess 
{
public:
  HR2ConfigAccess();
  ~HR2ConfigAccess(){;}
  void parseJsonFile(std::string jsf);
  void parseJson();
  void parseJsonUrl(std::string jsf);
  std::map<uint64_t,lydaq::HR2Slow>& asicMap();
  void clear();
  void dumpMap();
  void parseDb(std::string state,std::string mode);
  void  prepareSlowControl(std::string ipadr);
private:
  std::map<uint64_t,lydaq::HR2Slow> _asicMap;
  Json::Value _jall,_jasic;
  uint8_t _slcBuffer[65536];
  uint32_t _slcBytes;
};
};
#endif
