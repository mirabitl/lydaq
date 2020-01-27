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
//#include "WiznetMessageHandler.hh"
#include <ILCConfDB.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>


using namespace lydaq;




lydaq::TdcConfigAccess::TdcConfigAccess()
{
  _slcBytes = 0;
  memset(_slcBuffer, 0, 0x1000 * sizeof(uint16_t));
  memset(_slcAddr, 0, 0x1000 * sizeof(uint16_t));
  _jall = Json::Value::null;
#ifndef NO_DB
  std::cout << "On initialise Oracle " << std::endl;
  try
  {
    DBInit::init();
  }
  catch (...)
  {
    LOG4CXX_FATAL(_logLdaq, "Cannot initialise Oracle");
    return;
  }

#endif
}

void lydaq::TdcConfigAccess::parseMongoDb(std::string state,uint32_t version)
{
  std::stringstream scmd;
  scmd<<"/bin/bash -c 'mgroc --download --state="<<state<<" --version="<<version<<"'";
  system(scmd.str().c_str());
  std::stringstream sname;
  sname<<"/dev/shm/"<<state<<"_"<<version<<".json";
  Json::Reader reader;
  std::ifstream ifs(sname.str().c_str(), std::ifstream::in);
  //      Json::Value _jall;
  bool parsingSuccessful = reader.parse(ifs, _jall, false);
  if (!_jall.isMember("asics"))
  {
    std::cout << " No DIF tag found " << std::endl;
    return;
  }
 
  const Json::Value &asics = _jall["asics"];
  
  for (Json::ValueConstIterator ita = asics.begin(); ita != asics.end(); ++ita)
    {
      const Json::Value &asic = *ita;
      uint32_t ipadr = asic["dif"].asUInt();
      uint8_t header = asic["num"].asUInt();
      lydaq::PR2 prs;
      prs.setJson(asic);
      uint64_t eid = ((uint64_t) ipadr) << 32 | header;
      _asicMap.insert(std::pair<uint64_t, lydaq::PR2>(eid, prs));
    }
  
}

void lydaq::TdcConfigAccess::parseJsonFile(std::string jsf)
{
  Json::Reader reader;
  std::ifstream ifs(jsf.c_str(), std::ifstream::in);
  //      Json::Value _jall;
  bool parsingSuccessful = reader.parse(ifs, _jall, false);
  if (!_jall.isMember("DIF"))
  {
    std::cout << " No DIF tag found " << std::endl;
    return;
  }
  this->parseJson();
}
void lydaq::TdcConfigAccess::parseJson()
{
  const Json::Value &difs = _jall["DIF"];

  for (Json::ValueConstIterator itd = difs.begin(); itd != difs.end(); ++itd)
  {
    const Json::Value &dif = *itd;
    //uint8_t difid=dif["NUM"].asUInt()>>8;
    std::string ipadr = dif["IPADDRESS"].asString();
    if (!dif.isMember("ASICS"))
    {
      std::cout << " No ASICS tag found " << std::endl;
      return;
    }
    const Json::Value &asics = dif["ASICS"];

    for (Json::ValueConstIterator ita = asics.begin(); ita != asics.end(); ++ita)
    {
      const Json::Value &asic = *ita;
      uint8_t header = asic["header"].asUInt();
      lydaq::PR2 prs;
      prs.setJson(asic);
      uint64_t eid = ((uint64_t)lydaq::TdcConfigAccess::convertIP(ipadr)) << 32 | header;
      _asicMap.insert(std::pair<uint64_t, lydaq::PR2>(eid, prs));
    }
  }
}

void lydaq::TdcConfigAccess::parseJsonUrl(std::string jsf)
{
  std::string jsconf = fsmwebCaller::curlQuery(jsf);
  std::cout << jsconf << std::endl;
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(jsconf, _jall);

  this->parseJson();
}
uint16_t *lydaq::TdcConfigAccess::slcBuffer() { return _slcBuffer; }
uint16_t *lydaq::TdcConfigAccess::slcAddr() { return _slcAddr; }
uint32_t lydaq::TdcConfigAccess::slcBytes() { return _slcBytes; }
std::map<uint64_t, lydaq::PR2> &lydaq::TdcConfigAccess::asicMap() { return _asicMap; }
void lydaq::TdcConfigAccess::prepareSlowControl(std::string ipadr)
{
std::cout<<"entering TDCConfigAccess.cc  lydaq::TdcConfigAccess::prepareSlowControl"<<std::endl;

  // Initialise
  _slcBytes = 0;
  uint64_t eid = ((uint64_t)lydaq::TdcConfigAccess::convertIP(ipadr)) << 32;
  // Loop on 4 Asic maximum
  for (int ias = 4; ias >= 1; ias--)
  {
    uint64_t eisearch = eid | ias;
    std::map<uint64_t, lydaq::PR2>::iterator im = _asicMap.find(eisearch);
    if (im == _asicMap.end())
      continue;
    printf("DIF %x ,Asic %d Found\n", eid >> 32, ias);
    im->second.prepare4Tdc(_slcAddr, _slcBuffer, _slcBytes);
    _slcBytes += SLC_BYTES_LENGTH;
  }
  if (_slcBytes >= SLC_BYTES_LENGTH)
  {
    _slcBuffer[_slcBytes] = 0x3;
    _slcAddr[_slcBytes] = 0x201;
    _slcBytes++;
  }
}
void lydaq::TdcConfigAccess::clear()
{
  _asicMap.clear();
}
void lydaq::TdcConfigAccess::dumpMap()
{
  for (auto x : _asicMap)
  {
    uint32_t ip = (x.first) >> 32 & 0XFFFFFFFF;
    uint8_t as = (x.first) & 0xFF;
    std::cout << " DIF " << std::hex << ip << std::dec << " ASIC " << (int)as << std::endl;
    x.second.dumpJson();
  }
}
void lydaq::TdcConfigAccess::parseDb(std::string stateName, std::string mode)
{
  State *_state = NULL;
#ifndef NO_DB
  Setup *theOracleSetup_ = NULL;
#endif
  if (mode.compare("DB") == 0)
  {
#ifndef NO_DB
    try
    {
      theOracleSetup_ = Setup::getSetup(stateName);
      std::cout << "On initialise Oracle " << (long)theOracleSetup_ << std::endl;
      LOG4CXX_INFO(_logLdaq, "Downloading " << stateName);
    }
    catch (...)
    {
      LOG4CXX_FATAL(_logLdaq, "Setup initialisation failed");
    }
    _state = theOracleSetup_->getStates()[0];
#else
    LOG4CXX_FATAL(_logLdaq, "No Oracle DB access compiled");
    return;
#endif
  }
  if (mode.compare("WEB") == 0)
  {
    try
    {
      LOG4CXX_INFO(_logLdaq, "  Web access" << stateName);
      char *wp = getenv("CONFDB_WEB");
      LOG4CXX_INFO(_logLdaq, "  Web access 1->" << wp);
      _state = State::getState_WebServer(stateName);
    }
    catch (ILCException::Exception e)
    {
      LOG4CXX_ERROR(_logLdaq, " Error in Web access" << e.getMessage());
      return;
    }
  }
  if (mode.compare("FILE") == 0)
  {

    try
    {
      std::string fullname = stateName;
      std::string basen(basename((char *)stateName.c_str()));
      size_t lastindex = basen.find_last_of(".");
      stateName = basen.substr(0, lastindex);
      _state = State::createStateFromXML_NODB(stateName, fullname);
    }
    catch (ILCException::Exception e)
    {
      LOG4CXX_ERROR(_logLdaq, " Error in File access" << e.getMessage());
      return;
    }
  }
  // Loop on Asic and create asicMap
  std::vector<ConfigObject *> dim = _state->getDifConfiguration()->getVector();
  std::vector<ConfigObject *> asic_vector_ = _state->getAsicConfiguration()->getVector();
  for (std::vector<ConfigObject *>::iterator itMRp = asic_vector_.begin(); itMRp != asic_vector_.end(); itMRp++)
  {
    Asic *itMR = (Asic *)(*itMRp);
    if (itMR->getInt("ENABLED") == 0)
      continue;
    uint32_t difid = itMR->getInt("DIF_ID");
    uint64_t eid = 0;
    for (std::vector<ConfigObject *>::iterator itDIFp = dim.begin(); itDIFp != dim.end(); itDIFp++)
    {
      Dif *itDIF = (Dif *)(*itDIFp);

      //      std::cout<<"DIF found "<<itDIF->getInt("ID")<<std::endl;
      if (itDIF->getInt("ID") != difid)
        continue;
      string ipadr = itDIF->getString("IP_ADDRESS");
      eid = (((uint64_t)lydaq::TdcConfigAccess::convertIP(ipadr)) << 32) | itMR->getInt("HEADER");
      break;
    }

    if (_asicMap.find(eid) != _asicMap.end())
      continue;
    lydaq::PR2 prs;
    // Fill it
    prs.setCf0_1p25pF(itMR->getInt("CF0_1P25PF"));
    prs.setCf1_2p5pF(itMR->getInt("CF1_2P5PF"));
    prs.setCf2_200fF(itMR->getInt("CF2_200FF"));
    prs.setCf3_100fF(itMR->getInt("CF3_100FF"));
    prs.setDIS_razchn_ext(itMR->getInt("DIS_RAZCHN_EXT"));
    prs.setDIS_razchn_int(itMR->getInt("DIS_RAZCHN_INT"));
    prs.setDIS_trig_mux(itMR->getInt("DIS_TRIG_MUX"));
    prs.setDIS_triggers(itMR->getInt("DIS_TRIGGERS"));
    prs.setDacDelay(itMR->getInt("DACDELAY"));
    prs.setEN10bDac(itMR->getInt("EN10BDAC"));
    prs.setEN_80M(itMR->getInt("EN_80M"));
    prs.setEN_NOR32_charge(itMR->getInt("EN_NOR32_CHARGE"));
    prs.setEN_NOR32_time(itMR->getInt("EN_NOR32_TIME"));
    prs.setEN_adc(itMR->getInt("EN_ADC"));
    prs.setEN_bias_6bit_dac(itMR->getInt("EN_BIAS_6BIT_DAC"));
    prs.setEN_bias_charge(itMR->getInt("EN_BIAS_CHARGE"));
    prs.setEN_bias_dac_delay(itMR->getInt("EN_BIAS_DAC_DELAY"));
    prs.setEN_bias_discri(itMR->getInt("EN_BIAS_DISCRI"));
    prs.setEN_bias_discri_adc_charge(itMR->getInt("EN_BIAS_DISCRI_ADC_CHARGE"));
    prs.setEN_bias_discri_adc_time(itMR->getInt("EN_BIAS_DISCRI_ADC_TIME"));
    prs.setEN_bias_discri_charge(itMR->getInt("EN_BIAS_DISCRI_CHARGE"));
    prs.setEN_bias_pa(itMR->getInt("EN_BIAS_PA"));
    prs.setEN_bias_ramp_delay(itMR->getInt("EN_BIAS_RAMP_DELAY"));
    prs.setEN_bias_sca(itMR->getInt("EN_BIAS_SCA"));
    prs.setEN_bias_tdc(itMR->getInt("EN_BIAS_TDC"));
    prs.setEN_discri_delay(itMR->getInt("EN_DISCRI_DELAY"));
    prs.setEN_dout_oc(itMR->getInt("EN_DOUT_OC"));
    prs.setEN_fast_lvds_rec(itMR->getInt("EN_FAST_LVDS_REC"));
    prs.setEN_slow_lvds_rec(itMR->getInt("EN_SLOW_LVDS_REC"));
    prs.setEN_temp_sensor(itMR->getInt("EN_TEMP_SENSOR"));
    prs.setEN_transmit(itMR->getInt("EN_TRANSMIT"));
    prs.setEN_transmitter(itMR->getInt("EN_TRANSMITTER"));
    prs.setON_OFF_1mA(itMR->getInt("ON_OFF_1MA"));
    prs.setON_OFF_2mA(itMR->getInt("ON_OFF_2MA"));
    prs.setON_OFF_input_dac(itMR->getInt("ON_OFF_INPUT_DAC"));
    prs.setON_OFF_otaQ(itMR->getInt("ON_OFF_OTAQ"));
    prs.setON_OFF_ota_mux(itMR->getInt("ON_OFF_OTA_MUX"));
    prs.setON_OFF_ota_probe(itMR->getInt("ON_OFF_OTA_PROBE"));
    prs.setPP10bDac(itMR->getInt("PP10BDAC"));
    prs.setPP_adc(itMR->getInt("PP_ADC"));
    prs.setPP_bias_6bit_dac(itMR->getInt("PP_BIAS_6BIT_DAC"));
    prs.setPP_bias_charge(itMR->getInt("PP_BIAS_CHARGE"));
    prs.setPP_bias_dac_delay(itMR->getInt("PP_BIAS_DAC_DELAY"));
    prs.setPP_bias_discri(itMR->getInt("PP_BIAS_DISCRI"));
    prs.setPP_bias_discri_adc_charge(itMR->getInt("PP_BIAS_DISCRI_ADC_CHARGE"));
    prs.setPP_bias_discri_adc_time(itMR->getInt("PP_BIAS_DISCRI_ADC_TIME"));
    prs.setPP_bias_discri_charge(itMR->getInt("PP_BIAS_DISCRI_CHARGE"));
    prs.setPP_bias_pa(itMR->getInt("PP_BIAS_PA"));
    prs.setPP_bias_ramp_delay(itMR->getInt("PP_BIAS_RAMP_DELAY"));
    prs.setPP_bias_sca(itMR->getInt("PP_BIAS_SCA"));
    prs.setPP_bias_tdc(itMR->getInt("PP_BIAS_TDC"));
    prs.setPP_discri_delay(itMR->getInt("PP_DISCRI_DELAY"));
    prs.setPP_fast_lvds_rec(itMR->getInt("PP_FAST_LVDS_REC"));
    prs.setPP_slow_lvds_rec(itMR->getInt("PP_SLOW_LVDS_REC"));
    prs.setPP_temp_sensor(itMR->getInt("PP_TEMP_SENSOR"));
    prs.setPP_transmitter(itMR->getInt("PP_TRANSMITTER"));
    prs.setSEL_80M(itMR->getInt("SEL_80M"));
    prs.setVthDiscriCharge(itMR->getInt("VTHDISCRICHARGE"));
    prs.setVthTime(itMR->getInt("VTHTIME"));
    prs.setcmd_polarity(itMR->getInt("CMD_POLARITY"));
    prs.setlatch(itMR->getInt("LATCH"));
    prs.setsel_starb_ramp_adc_ext(itMR->getInt("SEL_STARB_RAMP_ADC_EXT"));
    prs.setusebcompensation(itMR->getInt("USEBCOMPENSATION"));

    // PR2B
#ifdef USE_PR2A
      LOG4CXX_INFO(_logLdaq, " PR2A!!");
#endif
  
#ifndef USE_PR2A
      LOG4CXX_INFO(_logLdaq, " PR2B!!");
 
    prs.setPA_ccomp_0(itMR->getInt("PA_CCOMP_0"));
    prs.setPA_ccomp_1(itMR->getInt("PA_CCOMP_1"));
    prs.setPA_ccomp_2(itMR->getInt("PA_CCOMP_2"));
    prs.setPA_ccomp_3(itMR->getInt("PA_CCOMP_3"));
    prs.setChoice_Trigger_Out(itMR->getInt("CHOICE_TRIGGER_OUT"));
#endif

    
    std::vector<int> DAC6B = itMR->getIntVector("DAC6B");
    std::vector<int> INPUTDAC = itMR->getIntVector("INPUTDAC");
    std::vector<int> INPUTDACCOMMAND = itMR->getIntVector("INPUTDACCOMMAND");
    std::vector<int> MASKDISCRICHARGE = itMR->getIntVector("MASKDISCRICHARGE");
    std::vector<int> MASKDISCRITIME = itMR->getIntVector("MASKDISCRITIME");
    for (uint8_t ch = 0; ch < 32; ch++)
    {
      prs.set6bDac(ch, DAC6B[ch]);
      prs.setInputDac(ch, INPUTDAC[ch]);
      prs.setInputDacCommand(ch, INPUTDACCOMMAND[ch]);
      prs.setMaskDiscriCharge(ch, MASKDISCRICHARGE[ch]);
      prs.setMaskDiscriTime(ch, MASKDISCRITIME[ch]);
    }
    prs.toJson();
    // Update map
    _asicMap.insert(std::pair<uint64_t, lydaq::PR2>(eid, prs));
  }

#ifndef NO_DB
  if (theOracleSetup_ != NULL)
  {
    delete theOracleSetup_;
    _state = NULL;
  }
#else
  if (_state != NULL)
    delete _state;
  _state = NULL;
#endif
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
uint32_t lydaq::TdcConfigAccess::convertIP(std::string hname)
{
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
  char ip[100];
  if ((he = gethostbyname(hname.c_str())) == NULL)
  {
    return 0;
  }

  addr_list = (struct in_addr **)he->h_addr_list;

  for (i = 0; addr_list[i] != NULL; i++)
  {
    //Return the first one;
    strcpy(ip, inet_ntoa(*addr_list[i]));
    break;
  }

  in_addr_t ls1 = inet_addr(ip);
  return (uint32_t)ls1;
}
