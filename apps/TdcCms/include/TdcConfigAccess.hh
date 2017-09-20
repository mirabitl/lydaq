#ifndef _TDC_CONFIG_ACCESS_HH
#define _TDC_CONFIG_ACCESS_HH
#include "PRSlow.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include <map>
namespace lydaq
{
class TdcConfigAccess 
{
public:
  TdcConfigAccess();
  ~TdcConfigAccess(){;}
  void parseJsonFile(std::string jsf);
  void parseJson();
  void parseJsonUrl(std::string jsf);
  uint16_t* slcBuffer();
  uint16_t* slcAddr();
  uint32_t  slcBytes();
  std::map<uint64_t,lydaq::PRSlow>& asicMap();
  void clear();
  void dumpMap();
  void  prepareSlowControl(std::string ipadr);
  void parseDb(std::string state,std::string mode);
  void dumpToShm(std::string path);
  void connect();
  void publish();
private:
  uint16_t _slcBuffer[0x1000];
  uint16_t _slcAddr[0x1000];    
  uint32_t _slcBytes;
  std::map<uint64_t,lydaq::PRSlow> _asicMap;
  Json::Value _jall,_jasic;
};
};
#endif
