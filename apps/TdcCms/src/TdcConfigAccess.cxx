#include "TdcConfigAccess.hh"
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <boost/format.hpp>
#include "fsmwebCaller.hh"
#include "TdcMessageHandler.hh"

using namespace zdaq;
using namespace lydaq;
lydaq::TdcConfigAccess::TdcConfigAccess()
{
  _slcBytes=0;
  memset(_slcBuffer,0,0x1000*sizeof(uint16_t));
  memset(_slcAddr,0,0x1000*sizeof(uint16_t));
  _jall=Json::Value::null;
}
void lydaq::TdcConfigAccess::parseJsonFile(std::string jsf)
{
  Json::Reader reader;
  std::ifstream ifs (jsf.c_str(), std::ifstream::in);
  //      Json::Value _jall;
  bool parsingSuccessful = reader.parse(ifs,_jall,false);
  if (!_jall.isMember("DIF"))
    {
      std::cout<<" No DIF tag found "<<std::endl;
      return;
    }
  this->parseJson();
}
void lydaq::TdcConfigAccess::parseJson()
{
  const Json::Value& difs = _jall["DIF"];
  
  for (Json::ValueConstIterator itd = difs.begin(); itd != difs.end(); ++itd)
	{
	  const Json::Value& dif = *itd;
	  uint8_t difid=dif["NUM"].asUInt();
	  std::string ipadr=dif["IPADDRESS"].asString();
	  if (!dif.isMember("ASICS"))
	    {
	      std::cout<<" No ASICS tag found "<<std::endl;
	      return;
	    }
	  const Json::Value& asics = dif["ASICS"];
  
	  for (Json::ValueConstIterator ita = asics.begin(); ita != asics.end(); ++ita)
	    {
	      const Json::Value& asic = *ita;
	      uint8_t header=asic["header"].asUInt();
	      lydaq::PRSlow prs;prs.setJson(asic);
	      uint64_t eid=((uint64_t) lydaq::TdcMessageHandler::convertIP(ipadr))<<32|header;
	      _asicMap.insert(std::pair<uint64_t,lydaq::PRSlow>(eid,prs));
	    }
	}
}

void lydaq::TdcConfigAccess::parseJsonUrl(std::string jsf)
{
  std::string jsconf=fsmwebCaller::curlQuery(jsf);
  std::cout<<jsconf<<std::endl;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(jsconf,_jall);
      
  this->parseJson();
}
uint16_t* lydaq::TdcConfigAccess::slcBuffer(){return _slcBuffer;}
uint16_t* lydaq::TdcConfigAccess::slcAddr(){return _slcAddr;}
uint32_t  lydaq::TdcConfigAccess::slcBytes(){return _slcBytes;}
std::map<uint64_t,lydaq::PRSlow>&  lydaq::TdcConfigAccess::asicMap(){return _asicMap;}
void  lydaq::TdcConfigAccess::prepareSlowControl(std::string ipadr)
{
  // Initialise
  _slcBytes=0;
  uint64_t eid=((uint64_t) lydaq::TdcMessageHandler::convertIP(ipadr))<<32;
  // Loop on 4 Asic maximum
  for (int ias=4;ias>=1;ias--)
    {
      uint64_t eisearch= eid|ias;
      std::map<uint64_t,lydaq::PRSlow>::iterator im=_asicMap.find(eisearch);
      if (im==_asicMap.end()) continue;
      printf("DIF %x ,Asic %d Found\n",eid>>32,ias); 
      im->second.prepare4Tdc(_slcAddr,_slcBuffer,_slcBytes);
      _slcBytes+=80;
    }
  if (_slcBytes>=80)
    {
      _slcBuffer[_slcBytes]=0x3;
      _slcAddr[_slcBytes]=0x201;
      _slcBytes++;
    }
}
void lydaq::TdcConfigAccess::clear()
{
  _asicMap.clear();
}
void lydaq::TdcConfigAccess::dumpMap()
{
  for (auto x:_asicMap)
    {
      uint32_t ip=(x.first)>>32&0XFFFFFFFF;
      uint8_t as=(x.first)&0xFF;
      std::cout<<" DIF "<<std::hex<<ip<<std::dec<<" ASIC "<<(int) as<<std::endl;
      x.second.dumpJson();
    }
}
void lydaq::TdcConfigAccess::parseDb(std::string state,std::string mode)
{
}
void lydaq::TdcConfigAccess::dumpToShm(std::string path)
{
}
void lydaq::TdcConfigAccess::connect()
{
}
void lydaq::TdcConfigAccess::publish()
{
}
