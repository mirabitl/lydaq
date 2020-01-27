#include "HR2ConfigAccess.hh"
#include "fsmwebCaller.hh"
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
#include <ILCConfDB.h>


using namespace lydaq;
lydaq::HR2ConfigAccess::HR2ConfigAccess()
{
 
  _jall=Json::Value::null;
  _jasic=Json::Value::null;
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
void lydaq::HR2ConfigAccess::parseMongoDb(std::string state,uint32_t version)
{
  std::stringstream scmd;
  scmd<<"/bin/bash -c 'mgroc --download --state="<<state<<" --version="<<version<<"'";
  system(scmd.str().c_str());
  std::stringstream sname;
  sname<<"/dev/shm/mgroc/"<<state<<"_"<<version<<".json";


  fprintf(stderr,"Parsing the file %s\n",sname.str().c_str());
  Json::Reader reader;

  std::ifstream ifs(sname.str().c_str(), std::ifstream::in);
  //      Json::Value _jall;
  fprintf(stderr,"Before Parsing the file %s\n",sname.str().c_str());
  bool parsingSuccessful = reader.parse(ifs, _jall, false);
  fprintf(stderr,"After Parsing the file %s done %d\n",sname.str().c_str(),parsingSuccessful);

  //std::cout<<"Before JALL "<<jall<<std::flush<<std::endl;
  //  _jall=jall;
  //std::cout<<"After JALL "<<_jall<<std::flush<<std::endl;
  if (!_jall.isMember("asics"))
  {
    std::cout << " No DIF tag found " << std::endl;
    return;
  }
 
  const Json::Value &asics = _jall["asics"];
  
  for (Json::ValueConstIterator ita = asics.begin(); ita != asics.end(); ++ita)
    {
      const Json::Value &asic = *ita;
      std::string ipadr = asic["address"].asString();
      uint8_t header = asic["num"].asUInt();
      fprintf(stderr,"Insering %s %d\n",sname.str().c_str(),header);
      lydaq::HR2Slow prs;
      prs.setJson(asic["slc"]);
      //std::cout<<asic["slc"]<<std::flush<<std::endl;
      uint64_t eid = ((uint64_t)  lydaq::TdcConfigAccess::convertIP(ipadr)) << 32 | header;
      _asicMap.insert(std::pair<uint64_t, lydaq::HR2Slow>(eid, prs));
      //prs.dumpBinary();
    }
  
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
	  uint64_t eid=((uint64_t) lydaq::TdcConfigAccess::convertIP(ipadr))<<32|header;
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

void  lydaq::HR2ConfigAccess::prepareSlowControl(std::string ipadr,bool inverted)
{
  // Initialise
  _slcBytes=0;
  uint64_t eid=((uint64_t) lydaq::TdcConfigAccess::convertIP(ipadr))<<32;
  // Loop on 48 Asic maximum
  for (int ias=1;ias<=48;ias++)
    {
      uint64_t eisearch= eid|ias;
      std::map<uint64_t,lydaq::HR2Slow>::iterator im=_asicMap.find(eisearch);
      if (im==_asicMap.end()) continue;
      if (!im->second.isEnabled())
	{
	  printf("\t ===> DIF %lx ,Asic %d disabled\n",eid>>32,ias);
	  continue;
	}
      printf("DIF %lx ,Asic %d Found\n",eid>>32,ias);
      if (!inverted)
	memcpy(&_slcBuffer[_slcBytes],im->second.ucPtr(),109);
      else
	{
	  uint8_t* dest=&_slcBuffer[_slcBytes];
	  uint8_t* sour=im->second.ucPtr();
	for(uint32_t ii=0;ii<109;ii++)
	  dest[108-ii]=sour[ii];
	}
      //memcpy(&_slcBuffer[_slcBytes],im->second.ucInvertedPtr(),109);
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
      LOG4CXX_INFO(_logLdaq,"Downloading "<<stateName);
      try
	{
	  _state = State::getState_WebServer(stateName);
	}
      catch (ILCException::Exception e)
	{
	  LOG4CXX_ERROR(_logLdaq," Error in Web access"<<e.getMessage());
	  return;
	}
      LOG4CXX_INFO(_logLdaq,"WEB Downloading done "<<stateName);
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
  LOG4CXX_INFO(_logLdaq," DIF size "<<dim.size());
  std::vector<ConfigObject*> asic_vector_=_state->getAsicConfiguration()->getVector();
  LOG4CXX_INFO(_logLdaq," ASIC size "<<asic_vector_.size());
  for (std::vector<ConfigObject*>::iterator itMRp=asic_vector_.begin();itMRp!=asic_vector_.end();itMRp++)
    {
      Asic* itMR=(Asic*) (*itMRp);
      if (itMR->getInt("ENABLED")==0) continue;
      uint32_t difid=itMR->getInt("DIF_ID");
      uint64_t eid=0;
      for (std::vector<ConfigObject*>::iterator itDIFp=dim.begin();itDIFp!=dim.end();itDIFp++)
	{
	  Dif* itDIF= (Dif*) (*itDIFp);

	  std::cout<<"DIF found "<<itDIF->getInt("ID")<<std::endl;
	  if (itDIF->getInt("ID")!=difid) continue;
	  string ipadr = itDIF->getString("IP_ADDRESS");
	  eid=(((uint64_t) lydaq::TdcConfigAccess::convertIP(ipadr))<<32) |itMR->getInt("HEADER");
	  std::cout<<"HEADER found "<<eid<<std::endl;
	  break;
	}


      if (_asicMap.find(eid)!=_asicMap.end()) continue;
      lydaq::HR2Slow prs;
       LOG4CXX_INFO(_logLdaq," HR2Slow created");
      // Fill it

      prs.setHEADER(itMR->getInt("HEADER"));
      prs.setEN_OTAQ(itMR->getInt("EN_OTAQ"));
      printf("%d\n",__LINE__);
      prs.setDACSW(itMR->getInt("DACSW"));
      prs.setSEL0(itMR->getInt("SEL0"));
      prs.setCMDB2FSB2(itMR->getInt("CMDB2FSB2"));
      prs.setCMDB0FSB2(itMR->getInt("CMDB0FSB2"));
      prs.setENOCTRANSMITON1B(itMR->getInt("ENOCTRANSMITON1B"));
      prs.setSEL1(itMR->getInt("SEL1"));
      printf("%d\n",__LINE__);
      prs.setCMDB2FSB1(itMR->getInt("CMDB2FSB1"));
      printf("%d\n",__LINE__);
      prs.setOTABGSW(itMR->getInt("OTABGSW"));
      printf("%d\n",__LINE__);
      prs.setSW50F0(itMR->getInt("SW50F0"));
      printf("%d\n",__LINE__);
      printf("%d\n",__LINE__);
      prs.setENOCDOUT2B(itMR->getInt("ENOCDOUT2B"));
      printf("%d\n",__LINE__);
      prs.setSW50K0(itMR->getInt("SW50K0"));
      printf("%d\n",__LINE__);
      prs.setSMALLDAC(itMR->getInt("SMALLDAC"));
      printf("%d\n",__LINE__);
      prs.setSELENDREADOUT(itMR->getInt("SELENDREADOUT"));
      prs.setCMDB3FSB1(itMR->getInt("CMDB3FSB1"));
      prs.setSWSSC(itMR->getInt("SWSSC"));
      prs.setPWRONBUFF(itMR->getInt("PWRONBUFF"));
      prs.setENOCDOUT1B(itMR->getInt("ENOCDOUT1B"));
      prs.setSW50K2(itMR->getInt("SW50K2"));
      prs.setCMDB2SS(itMR->getInt("CMDB2SS"));
      prs.setTRIG1B(itMR->getInt("TRIG1B"));
      prs.setCMDB1SS(itMR->getInt("CMDB1SS"));
      prs.setPWRONPA(itMR->getInt("PWRONPA"));
      prs.setPWRONSS(itMR->getInt("PWRONSS"));
      printf("%d\n",__LINE__);
      prs.setRAZCHNINTVAL(itMR->getInt("RAZCHNINTVAL"));
      printf("%d\n",__LINE__);
      prs.setSW100K1(itMR->getInt("SW100K1"));
      printf("%d\n",__LINE__);
      prs.setCLKMUX(itMR->getInt("CLKMUX"));
      printf("%d\n",__LINE__);
      prs.setSW50F1(itMR->getInt("SW50F1"));
      printf("%d\n",__LINE__);
      prs.setPWRONFSB0(itMR->getInt("PWRONFSB0"));
      printf("%d\n",__LINE__);
      prs.setENOCTRANSMITON2B(itMR->getInt("ENOCTRANSMITON2B"));
      printf("%d\n",__LINE__);
      prs.setENOCCHIPSATB(itMR->getInt("ENOCCHIPSATB"));
      printf("%d\n",__LINE__);
      prs.setCMDB3SS(itMR->getInt("CMDB3SS"));
      printf("%d\n",__LINE__);
      prs.setDISCRI1(itMR->getInt("DISCRI1"));
      printf("%d\n",__LINE__);
      prs.setSW50F2(itMR->getInt("SW50F2"));
      printf("%d\n",__LINE__);
      prs.setSW100K0(itMR->getInt("SW100K0"));
      printf("%d\n",__LINE__);
      prs.setCMDB3FSB2(itMR->getInt("CMDB3FSB2"));
      printf("%d\n",__LINE__);
      prs.setSCON(itMR->getInt("SCON"));
      printf("%d\n",__LINE__);
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
      printf("%d\n",__LINE__);
      prs.setSELSTARTREADOUT(itMR->getInt("SELSTARTREADOUT"));
      prs.setSW50K1(itMR->getInt("SW50K1"));
      prs.setCMDB1FSB1(itMR->getInt("CMDB1FSB1"));
      printf("%d\n",__LINE__);
      prs.setENTRIGOUT(itMR->getInt("ENTRIGOUT"));
      printf("%d\n",__LINE__);
      prs.setCMDB0SS(itMR->getInt("CMDB0SS"));
      printf("%d\n",__LINE__);
      prs.setSW100F0(itMR->getInt("SW100F0"));
      printf("%d\n",__LINE__);
      prs.setCMDB1FSB2(itMR->getInt("CMDB1FSB2"));
      printf("%d\n",__LINE__);
      prs.setOTAQ_PWRADC(itMR->getInt("OTAQ_PWRADC"));
      printf("%d\n",__LINE__);
      prs.setCMDB0FSB1(itMR->getInt("CMDB0FSB1"));
      printf("%d\n",__LINE__);
      prs.setDISCROROR(itMR->getInt("DISCROROR"));
      printf("%d\n",__LINE__);
      prs.setDISCRI0(itMR->getInt("DISCRI0"));
      printf("%d\n",__LINE__);
      prs.setB0(itMR->getInt("B0"));
      prs.setB1(itMR->getInt("B1"));
      prs.setB2(itMR->getInt("B2"));
       LOG4CXX_INFO(_logLdaq," HR2Slow 1/2 filled");
    
      std::vector<int> PAGAIN=itMR->getIntVector("PAGAIN");
      //std::vector<int> CTEST=itMR->getIntVector("CTEST");
      unsigned long long tmask0;
      unsigned long long tmask1;
      unsigned long long tmask2;
      sscanf(itMR->getString("MASK0").c_str(),"%llx\n",&tmask0);   
      sscanf(itMR->getString("MASK1").c_str(),"%llx\n",&tmask1);   
      sscanf(itMR->getString("MASK2").c_str(),"%llx\n",&tmask2);
      unsigned long long tctest;
      sscanf(itMR->getString("CTEST").c_str(),"%llx\n",&tctest);
      
      
      
      for (uint8_t ch=0;ch<64;ch++)
	{
	  prs.setPAGAIN(ch,PAGAIN[ch]);
	  prs.setCTEST(ch,(tctest>>ch)&1);
	  prs.setMASKChannel(0,ch,(tmask0>>ch)&1);
	  prs.setMASKChannel(1,ch,(tmask1>>ch)&1);
	  prs.setMASKChannel(2,ch,(tmask2>>ch)&1);

	}
      LOG4CXX_INFO(_logLdaq," HR2Slow filled");

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
