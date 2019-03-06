#include "HR2ConfigAccess.hh"
#include "fsmwebCaller.hh"
#include "MpiMessageHandler.hh"
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
#include <ILCConfDB.h>


using namespace lydaq;
lydaq::HR2ConfigAccess::HR2ConfigAccess()
{
 
  _jall=Json::Value::null;
#ifndef NO_DB
  std::cout<<"On initialise Oracle "<<std::endl;
  try {
    DBInit::init();
  }
  catch(...)
    {
      LOG4CXX_FATAL(_logLdaq,"Cannot initialise Oracle");
      return;
    }

#endif
}
void lydaq::HR2ConfigAccess::parseJsonFile(std::string jsf)
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
void lydaq::HR2ConfigAccess::parseJson()
{
  const Json::Value& difs = _jall["DIF"];
  
  for (Json::ValueConstIterator itd = difs.begin(); itd != difs.end(); ++itd)
    {
      const Json::Value& dif = *itd;
      //uint8_t difid=dif["NUM"].asUInt()>>8;
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
	  uint8_t header=asic["HEADER"].asUInt();
	  lydaq::HR2Slow prs;prs.setJson(asic);
	  uint64_t eid=((uint64_t) lydaq::MpiMessageHandler::convertIP(ipadr))<<32|header;
	  _asicMap.insert(std::pair<uint64_t,lydaq::HR2Slow>(eid,prs));
	}
    }
}

void lydaq::HR2ConfigAccess::parseJsonUrl(std::string jsf)
{
  std::string jsconf=fsmwebCaller::curlQuery(jsf);
  std::cout<<jsconf<<std::endl;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(jsconf,_jall);
      
  this->parseJson();
}
uint8_t* lydaq::HR2ConfigAccess::slcBuffer(){return _slcBuffer;}
uint32_t  lydaq::HR2ConfigAccess::slcBytes(){return _slcBytes;}
std::map<uint64_t,lydaq::HR2Slow>&  lydaq::HR2ConfigAccess::asicMap(){return _asicMap;}

void  lydaq::HR2ConfigAccess::prepareSlowControl(std::string ipadr)
{
  // Initialise
  _slcBytes=0;
  uint64_t eid=((uint64_t) lydaq::MpiMessageHandler::convertIP(ipadr))<<32;
  // Loop on 4 Asic maximum
  for (int ias=24;ias>=1;ias--)
    {
      uint64_t eisearch= eid|ias;
      std::map<uint64_t,lydaq::HR2Slow>::iterator im=_asicMap.find(eisearch);
      if (im==_asicMap.end()) continue;
      printf("DIF %x ,Asic %d Found\n",eid>>32,ias); 
      memcpy(&_slcBuffer[_slcBytes],im->second.ucPtr(),109);
      _slcBytes+=109;
    }
}


void lydaq::HR2ConfigAccess::clear()
{
  _asicMap.clear();
}
void lydaq::HR2ConfigAccess::dumpMap()
{
  for (auto x:_asicMap)
    {
      uint32_t ip=(x.first)>>32&0XFFFFFFFF;
      uint8_t as=(x.first)&0xFF;
      std::cout<<" DIF "<<std::hex<<ip<<std::dec<<" ASIC "<<(int) as<<std::endl;
      x.second.dumpJson();
    }
}
void lydaq::HR2ConfigAccess::parseDb(std::string stateName,std::string mode)
{
  State* _state=NULL;
#ifndef NO_DB
  Setup* theOracleSetup_=NULL;
#endif
  if (mode.compare("DB")==0)
    {
#ifndef NO_DB
      try {
	theOracleSetup_=Setup::getSetup(stateName); 
	std::cout<<"On initialise Oracle "<<(long)theOracleSetup_<< std::endl;
	LOG4CXX_INFO(_logLdaq,"Downloading "<<stateName);
       
      }
      catch(...)
	{
	  LOG4CXX_FATAL(_logLdaq,"Setup initialisation failed");
	}
      _state=theOracleSetup_->getStates()[0];
#else
      LOG4CXX_FATAL(_logLdaq,"No Oracle DB access compiled");
      return;
#endif
      
    }
  if (mode.compare("WEB")==0)
    {
      try
	{
	  _state = State::getState_WebServer(stateName);
	}
      catch (ILCException::Exception e)
	{
	  LOG4CXX_ERROR(_logLdaq," Error in Web access"<<e.getMessage());
	  return;
	}

    }
  if (mode.compare("FILE")==0)
    {
      
      try
	{
	  std::string fullname=stateName;
	  std::string basen(basename((char*)stateName.c_str()));
	  size_t lastindex = basen.find_last_of("."); 
	  stateName = basen.substr(0, lastindex);
	  _state = State::createStateFromXML_NODB(stateName,fullname);
	}
      catch (ILCException::Exception e)
	{
	  LOG4CXX_ERROR(_logLdaq," Error in File access"<<e.getMessage());
	  return;
	}

    }
  // Loop on Asic and create asicMap
  std::vector<ConfigObject*> dim=_state->getDifConfiguration()->getVector();
  std::vector<ConfigObject*> asic_vector_=_state->getAsicConfiguration()->getVector();
  for (std::vector<ConfigObject*>::iterator itMRp=asic_vector_.begin();itMRp!=asic_vector_.end();itMRp++)
    {
      Asic* itMR=(Asic*) (*itMRp);
      if (itMR->getInt("ENABLED")==0) continue;
      uint32_t difid=itMR->getInt("DIF_ID");
      uint64_t eid=0;
      for (std::vector<ConfigObject*>::iterator itDIFp=dim.begin();itDIFp!=dim.end();itDIFp++)
	{
	  Dif* itDIF= (Dif*) (*itDIFp);

	  //      std::cout<<"DIF found "<<itDIF->getInt("ID")<<std::endl;
	  if (itDIF->getInt("ID")!=difid) continue;
	  string ipadr = itDIF->getString("IP_ADDRESS");
	  eid=(((uint64_t) lydaq::MpiMessageHandler::convertIP(ipadr))<<32) |itMR->getInt("HEADER");
	  break;
	}


      if (_asicMap.find(eid)!=_asicMap.end()) continue;
      lydaq::HR2Slow prs;
      // Fill it


      prs.setEN_OTAQ(itMR->getInt("EN_OTAQ"));
      prs.setDACSW(itMR->getInt("DACSW"));
      prs.setSEL0(itMR->getInt("SEL0"));
      prs.setCMDB2FSB2(itMR->getInt("CMDB2FSB2"));
      prs.setCMDB0FSB2(itMR->getInt("CMDB0FSB2"));
      prs.setENOCTRANSMITON1B(itMR->getInt("ENOCTRANSMITON1B"));
      prs.setSEL1(itMR->getInt("SEL1"));
      prs.setCMDB2FSB1(itMR->getInt("CMDB2FSB1"));
      prs.setOTABGSW(itMR->getInt("OTABGSW"));
      prs.setSW50F0(itMR->getInt("SW50F0"));
      prs.setSELRAZ1(itMR->getInt("SELRAZ1"));
      prs.setENOCDOUT2B(itMR->getInt("ENOCDOUT2B"));
      prs.setSW50K0(itMR->getInt("SW50K0"));
      prs.setSMALLDAC(itMR->getInt("SMALLDAC"));
      prs.setSELENDREADOUT(itMR->getInt("SELENDREADOUT"));
      prs.setCMDB3FSB1(itMR->getInt("CMDB3FSB1"));
      prs.setSWSSC(itMR->getInt("SWSSC"));
      prs.setPWRONBUFF(itMR->getInt("PWRONBUFF"));
      prs.setENOCDOUT1B(itMR->getInt("ENOCDOUT1B"));
      prs.setSELRAZ0(itMR->getInt("SELRAZ0"));
      prs.setSW50K2(itMR->getInt("SW50K2"));
      prs.setCMDB2SS(itMR->getInt("CMDB2SS"));
      prs.setTRIG1B(itMR->getInt("TRIG1B"));
      prs.setCMDB1SS(itMR->getInt("CMDB1SS"));
      prs.setPWRONPA(itMR->getInt("PWRONPA"));
      prs.setPWRONSS(itMR->getInt("PWRONSS"));
      prs.setRAZCHNINTVAL(itMR->getInt("RAZCHNINTVAL"));
      prs.setSW100K1(itMR->getInt("SW100K1"));
      prs.setCKMUX(itMR->getInt("CKMUX"));
      prs.setSW50F1(itMR->getInt("SW50F1"));
      prs.setPWRONFSB0(itMR->getInt("PWRONFSB0"));
      prs.setENOCTRANSMITON2B(itMR->getInt("ENOCTRANSMITON2B"));
      prs.setENOCCHIPSATB(itMR->getInt("ENOCCHIPSATB"));
      prs.setCMDB3SS(itMR->getInt("CMDB3SS"));
      prs.setDISCRI1(itMR->getInt("DISCRI1"));
      prs.setSW50F2(itMR->getInt("SW50F2"));
      prs.setSW100K0(itMR->getInt("SW100K0"));
      prs.setCMDB3FSB2(itMR->getInt("CMDB3FSB2"));
      prs.setSCON(itMR->getInt("SCON"));
      prs.setTRIG2B(itMR->getInt("TRIG2B"));
      prs.setSW100K2(itMR->getInt("SW100K2"));
      prs.setSW100F1(itMR->getInt("SW100F1"));
      prs.setTRIGEXTVAL(itMR->getInt("TRIGEXTVAL"));
      prs.setPWRONFSB2(itMR->getInt("PWRONFSB2"));
      prs.setRS_OR_DISCRI(itMR->getInt("RS_OR_DISCRI"));
      prs.setTRIG0B(itMR->getInt("TRIG0B"));
      prs.setPWRONFSB1(itMR->getInt("PWRONFSB1"));
      prs.setSW100F2(itMR->getInt("SW100F2"));
      prs.setPWRONW(itMR->getInt("PWRONW"));
      prs.setRAZCHNEXTVAL(itMR->getInt("RAZCHNEXTVAL"));
      prs.setDISCRI2(itMR->getInt("DISCRI2"));
      prs.setSELSTARTREADOUT(itMR->getInt("SELSTARTREADOUT"));
      prs.setSW50K1(itMR->getInt("SW50K1"));
      prs.setCMDB1FSB1(itMR->getInt("CMDB1FSB1"));
      prs.setENTRIGOUT(itMR->getInt("ENTRIGOUT"));
      prs.setCMDB0SS(itMR->getInt("CMDB0SS"));
      prs.setSW100F0(itMR->getInt("SW100F0"));
      prs.setCMDB1FSB2(itMR->getInt("CMDB1FSB2"));
      prs.setOTAQ_PWRADC(itMR->getInt("OTAQ_PWRADC"));
      prs.setCMDB0FSB1(itMR->getInt("CMDB0FSB1"));
      prs.setDISCOROR(itMR->getInt("DISCOROR"));
      prs.setDISCRI0(itMR->getInt("DISCRI0"));
      prs.setB0(itMR->getInt("B0"));
      prs.setB0(itMR->getInt("B1"));
      prs.setB0(itMR->getInt("B2"));

      std::vector<int> PAGAIN=itMR->getIntVector("PAGAIN");
      std::vector<int> CTEST=itMR->getIntVector("CTEST");
      std::vector<int> M0=itMR->getIntVector("MASK0");
      std::vector<int> M1=itMR->getIntVector("MASK1");
      std::vector<int> M2=itMR->getIntVector("MASK2");

      for (uint8_t ch=0;ch<64;ch++)
	{
	  prs.setPAGAIN(ch,PAGAIN[ch]);
	  prs.setCTEST(ch,CTEST[ch]==1);
	  prs.setMASKChannel(0,ch,M0[ch]==1);
	  prs.setMASKChannel(1,ch,M1[ch]==1);
	  prs.setMASKChannel(2,ch,M2[ch]==1);

	}
      prs.toJson();
      // Update map
      _asicMap.insert(std::pair<uint64_t,lydaq::HR2Slow>(eid,prs));
    }

#ifndef NO_DB
  if (theOracleSetup_!=NULL)
    {
      delete  theOracleSetup_;
      _state=NULL;
    }
#else
  if (_state!=NULL)
    delete _state;
  _state=NULL;
#endif


}
