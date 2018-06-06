#ifndef _PRSLOW_HH
#define _PRSLOW_HH
#include <bitset>
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
namespace lydaq {
  class PRSlow
  {
  public:
    PRSlow(){ memset(_l,0,20*sizeof(uint32_t));}
    bool getBit(int b){return (_l[b/32]>>(b%32))&1;}
    void setBit(int b){ _l[b/32]|=(1<<(b%32));};
    void clearBit(int b){ _l[b/32]&= ~(1<<(b%32));};
    void setBitState(int b,bool t){if (t) setBit(b); else clearBit(b);}
    uint8_t getByte(int b){return
	(getBit(b)|getBit(b+1)<<1|getBit(b+2)<<2|getBit(b+3)<<3|getBit(b+4)<<4|getBit(b+5)<<5|getBit(b+6)<<6|getBit(b+7)<<7);
    }
    void setByte(int b,uint8_t val,uint8_t len)
    {
      //unsigned char* p=&_l[b/32];
      for(int i=0;i<len;i++)
	if ((val>>i)&1)
	  setBit(b+i);
	else
	  clearBit(b+i);
    }
    void Print()
    {
      /*
	for (int i=0;i<20;i++)
	{
	std::cout<<std::hex<<_l[i]<<std::dec<<std::endl;
	std::bitset<32> a(_l[i]);
	std::cout<<a<<std::endl;
	}
      */

      std::cout<<"EN10bDac ="<<getEN10bDac()<<std::endl;
      std::cout<<"PP10bDac ="<<getPP10bDac()<<std::endl;
      std::cout<<"EN_adc ="<<getEN_adc()<<std::endl;
      std::cout<<"PP_adc ="<<getPP_adc()<<std::endl;
      std::cout<<"sel_starb_ramp_adc_ext ="<<getsel_starb_ramp_adc_ext()<<std::endl;
      std::cout<<"usebcompensation ="<<getusebcompensation()<<std::endl;
      std::cout<<"EN_bias_dac_delay ="<<getEN_bias_dac_delay()<<std::endl;
      std::cout<<"PP_bias_dac_delay ="<<getPP_bias_dac_delay()<<std::endl;
      std::cout<<"EN_bias_ramp_delay ="<<getEN_bias_ramp_delay()<<std::endl;
      std::cout<<"PP_bias_ramp_delay ="<<getPP_bias_ramp_delay()<<std::endl;
      std::cout<<"EN_discri_delay ="<<getEN_discri_delay()<<std::endl;
      std::cout<<"PP_discri_delay ="<<getPP_discri_delay()<<std::endl;
      std::cout<<"EN_temp_sensor ="<<getEN_temp_sensor()<<std::endl;
      std::cout<<"PP_temp_sensor ="<<getPP_temp_sensor()<<std::endl;
      std::cout<<"EN_bias_pa ="<<getEN_bias_pa()<<std::endl;
      std::cout<<"PP_bias_pa ="<<getPP_bias_pa()<<std::endl;
      std::cout<<"EN_bias_discri ="<<getEN_bias_discri()<<std::endl;
      std::cout<<"PP_bias_discri ="<<getPP_bias_discri()<<std::endl;
      std::cout<<"cmd_polarity ="<<getcmd_polarity()<<std::endl;
      std::cout<<"latch ="<<getlatch()<<std::endl;
      std::cout<<"EN_bias_6bit_dac ="<<getEN_bias_6bit_dac()<<std::endl;
      std::cout<<"PP_bias_6bit_dac ="<<getPP_bias_6bit_dac()<<std::endl;
      std::cout<<"EN_bias_tdc ="<<getEN_bias_tdc()<<std::endl;
      std::cout<<"PP_bias_tdc ="<<getPP_bias_tdc()<<std::endl;
      std::cout<<"ON_OFF_input_dac ="<<getON_OFF_input_dac()<<std::endl;
      std::cout<<"EN_bias_charge ="<<getEN_bias_charge()<<std::endl;
      std::cout<<"PP_bias_charge ="<<getPP_bias_charge()<<std::endl;
      std::cout<<"Cf3_100fF ="<<getCf3_100fF()<<std::endl;
      std::cout<<"Cf2_200fF ="<<getCf2_200fF()<<std::endl;
      std::cout<<"Cf1_2p5pF ="<<getCf1_2p5pF()<<std::endl;
      std::cout<<"Cf0_1p25pF ="<<getCf0_1p25pF()<<std::endl;
      std::cout<<"EN_bias_sca ="<<getEN_bias_sca()<<std::endl;
      std::cout<<"PP_bias_sca ="<<getPP_bias_sca()<<std::endl;
      std::cout<<"EN_bias_discri_charge ="<<getEN_bias_discri_charge()<<std::endl;
      std::cout<<"PP_bias_discri_charge ="<<getPP_bias_discri_charge()<<std::endl;
      std::cout<<"EN_bias_discri_adc_time ="<<getEN_bias_discri_adc_time()<<std::endl;
      std::cout<<"PP_bias_discri_adc_time ="<<getPP_bias_discri_adc_time()<<std::endl;
      std::cout<<"EN_bias_discri_adc_charge ="<<getEN_bias_discri_adc_charge()<<std::endl;
      std::cout<<"PP_bias_discri_adc_charge ="<<getPP_bias_discri_adc_charge()<<std::endl;
      std::cout<<"DIS_razchn_int ="<<getDIS_razchn_int()<<std::endl;
      std::cout<<"DIS_razchn_ext ="<<getDIS_razchn_ext()<<std::endl;
      std::cout<<"SEL_80M ="<<getSEL_80M()<<std::endl;
      std::cout<<"EN_80M ="<<getEN_80M()<<std::endl;
      std::cout<<"EN_slow_lvds_rec ="<<getEN_slow_lvds_rec()<<std::endl;
      std::cout<<"PP_slow_lvds_rec ="<<getPP_slow_lvds_rec()<<std::endl;
      std::cout<<"EN_fast_lvds_rec ="<<getEN_fast_lvds_rec()<<std::endl;
      std::cout<<"PP_fast_lvds_rec ="<<getPP_fast_lvds_rec()<<std::endl;
      std::cout<<"EN_transmitter ="<<getEN_transmitter()<<std::endl;
      std::cout<<"PP_transmitter ="<<getPP_transmitter()<<std::endl;
      std::cout<<"ON_OFF_1mA ="<<getON_OFF_1mA()<<std::endl;
      std::cout<<"ON_OFF_2mA ="<<getON_OFF_2mA()<<std::endl;
      std::cout<<"ON_OFF_otaQ ="<<getON_OFF_otaQ()<<std::endl;
      std::cout<<"ON_OFF_ota_mux ="<<getON_OFF_ota_mux()<<std::endl;
      std::cout<<"ON_OFF_ota_probe ="<<getON_OFF_ota_probe()<<std::endl;
      std::cout<<"DIS_trig_mux ="<<getDIS_trig_mux()<<std::endl;
      std::cout<<"EN_NOR32_time ="<<getEN_NOR32_time()<<std::endl;
      std::cout<<"EN_NOR32_charge ="<<getEN_NOR32_charge()<<std::endl;
      std::cout<<"DIS_triggers ="<<getDIS_triggers()<<std::endl;
      std::cout<<"EN_dout_oc ="<<getEN_dout_oc()<<std::endl;
      std::cout<<"EN_transmit ="<<getEN_transmit()<<std::endl;
      std::cout<<"DacDelay ="<<getDacDelay()<<std::endl;		
      for (int ch=0;ch<32;ch++)
	std::cout<<"InputDac["<<ch<<"]="<<(int) getInputDac(ch)<<std::endl;
      for (int ch=0;ch<32;ch++)
	std::cout<<"6bDac["<<ch<<"]="<<(int) get6bDac(ch)<<std::endl;
      for (int ch=0;ch<32;ch++)
	std::cout<<"MaskDiscriCharge["<<ch<<"]="<<(int) getMaskDiscriCharge(ch)<<std::endl;
      for (int ch=0;ch<32;ch++)
	std::cout<<"MaskDiscriTime["<<ch<<"]="<<(int) getMaskDiscriTime(ch)<<std::endl;
      for (int ch=0;ch<32;ch++)
	std::cout<<"InputDacCommand["<<ch<<"]="<<(int) getInputDacCommand(ch)<<std::endl;

      std::cout<<"VthDiscriCharge ="<<getVthDiscriCharge()<<std::endl;
      std::cout<<"VthTime ="<<getVthTime()<<std::endl;			

  
      for (int i=0;i<20;i++)
	printf("%d %x \n",i,_l[i]);
      /* getchar();
       */


    
    }
    void toJson()
    {
      _jasic.clear();
      _jasic["EN10bDac"]=getEN10bDac();
      _jasic["PP10bDac"]=getPP10bDac();
      _jasic["EN_adc"]=getEN_adc();
      _jasic["PP_adc"]=getPP_adc();
      _jasic["sel_starb_ramp_adc_ext"]=getsel_starb_ramp_adc_ext();
      _jasic["usebcompensation"]=getusebcompensation();
      _jasic["EN_bias_dac_delay"]=getEN_bias_dac_delay();
      _jasic["PP_bias_dac_delay"]=getPP_bias_dac_delay();
      _jasic["EN_bias_ramp_delay"]=getEN_bias_ramp_delay();
      _jasic["PP_bias_ramp_delay"]=getPP_bias_ramp_delay();
      _jasic["EN_discri_delay"]=getEN_discri_delay();
      _jasic["PP_discri_delay"]=getPP_discri_delay();
      _jasic["EN_temp_sensor"]=getEN_temp_sensor();
      _jasic["PP_temp_sensor"]=getPP_temp_sensor();
      _jasic["EN_bias_pa"]=getEN_bias_pa();
      _jasic["PP_bias_pa"]=getPP_bias_pa();
      _jasic["EN_bias_discri"]=getEN_bias_discri();
      _jasic["PP_bias_discri"]=getPP_bias_discri();
      _jasic["cmd_polarity"]=getcmd_polarity();
      _jasic["latch"]=getlatch();
      _jasic["EN_bias_6bit_dac"]=getEN_bias_6bit_dac();
      _jasic["PP_bias_6bit_dac"]=getPP_bias_6bit_dac();
      _jasic["EN_bias_tdc"]=getEN_bias_tdc();
      _jasic["PP_bias_tdc"]=getPP_bias_tdc();
      _jasic["ON_OFF_input_dac"]=getON_OFF_input_dac();
      _jasic["EN_bias_charge"]=getEN_bias_charge();
      _jasic["PP_bias_charge"]=getPP_bias_charge();
      _jasic["Cf3_100fF"]=getCf3_100fF();
      _jasic["Cf2_200fF"]=getCf2_200fF();
      _jasic["Cf1_2p5pF"]=getCf1_2p5pF();
      _jasic["Cf0_1p25pF"]=getCf0_1p25pF();
      _jasic["EN_bias_sca"]=getEN_bias_sca();
      _jasic["PP_bias_sca"]=getPP_bias_sca();
      _jasic["EN_bias_discri_charge"]=getEN_bias_discri_charge();
      _jasic["PP_bias_discri_charge"]=getPP_bias_discri_charge();
      _jasic["EN_bias_discri_adc_time"]=getEN_bias_discri_adc_time();
      _jasic["PP_bias_discri_adc_time"]=getPP_bias_discri_adc_time();
      _jasic["EN_bias_discri_adc_charge"]=getEN_bias_discri_adc_charge();
      _jasic["PP_bias_discri_adc_charge"]=getPP_bias_discri_adc_charge();
      _jasic["DIS_razchn_int"]=getDIS_razchn_int();
      _jasic["DIS_razchn_ext"]=getDIS_razchn_ext();
      _jasic["SEL_80M"]=getSEL_80M();
      _jasic["EN_80M"]=getEN_80M();
      _jasic["EN_slow_lvds_rec"]=getEN_slow_lvds_rec();
      _jasic["PP_slow_lvds_rec"]=getPP_slow_lvds_rec();
      _jasic["EN_fast_lvds_rec"]=getEN_fast_lvds_rec();
      _jasic["PP_fast_lvds_rec"]=getPP_fast_lvds_rec();
      _jasic["EN_transmitter"]=getEN_transmitter();
      _jasic["PP_transmitter"]=getPP_transmitter();
      _jasic["ON_OFF_1mA"]=getON_OFF_1mA();
      _jasic["ON_OFF_2mA"]=getON_OFF_2mA();
      _jasic["ON_OFF_otaQ"]=getON_OFF_otaQ();
      _jasic["ON_OFF_ota_mux"]=getON_OFF_ota_mux();
      _jasic["ON_OFF_ota_probe"]=getON_OFF_ota_probe();
      _jasic["DIS_trig_mux"]=getDIS_trig_mux();
      _jasic["EN_NOR32_time"]=getEN_NOR32_time();
      _jasic["EN_NOR32_charge"]=getEN_NOR32_charge();
      _jasic["DIS_triggers"]=getDIS_triggers();
      _jasic["EN_dout_oc"]=getEN_dout_oc();
      _jasic["EN_transmit"]=getEN_transmit();
      _jasic["DacDelay"]=getDacDelay();
      Json::Value idac;Json::Value bdac;
      Json::Value mdc; Json::Value mdt; Json::Value idc;
      for (int ch=0;ch<32;ch++)
	{
	  idac.append((int) getInputDac(ch));
	  bdac.append((int) get6bDac(ch));
	  mdc.append((int) getMaskDiscriCharge(ch));
	  mdt.append((int) getMaskDiscriTime(ch));
	  idc.append((int) getInputDacCommand(ch));
	}
      _jasic["InputDac"]=idac;
      _jasic["6bDac"]=bdac;
      _jasic["MaskDiscriCharge"]=mdc;
      _jasic["MaskDiscriTime"]=mdt;
      _jasic["InputDacCommand"]=idc;

      _jasic["VthDiscriCharge"]=getVthDiscriCharge();
      _jasic["VthTime"]=getVthTime();			
   

    }
    static void loadAsics(std::string fname,PRSlow& s1, PRSlow& s2)
    {
      FILE *fp=fopen(fname.c_str(), "r");
      int ier;
      uint32_t nl=0;
      do
	{
	  uint32_t n1,v1,n2,v2;
	  ier=fscanf(fp,"%d,%d,%d,%d\n",&n1,&v1,&n2,&v2);
	  printf("%d %d -> %d %d \n",ier,nl,n1,v1); 
	  s1.setBitState(n1,v1);
	  s2.setBitState(n2,v2);
	  nl++;
	} while (ier!=EOF);
      fclose(fp);
      s1.toJson();
      s2.toJson();
      //loadInvert(s);
      //Print();

    }
    void dumpJson()
    {
      Json::StyledWriter styledWriter;
      std::cout << styledWriter.write(_jasic) << std::endl;
    }
    void loadJson(std::string fname)
    {
      Json::Reader reader;
      //bool parsingSuccessful = reader.parse(fname,_jasic);

      std::ifstream ifs (fname.c_str(), std::ifstream::in);
      
      bool parsingSuccessful = reader.parse(ifs, _jasic,false);
      
      if (parsingSuccessful)
	{
	  Json::StyledWriter styledWriter;
	  //std::cout << styledWriter.write(_jasic) << std::endl;
	}
      else
	std::cout<<"parsing erro "<<std::endl;
    }
    void setFromJson()
    {
    
      
      const Json::Value& books = _jasic["6bDac"];
      uint8_t ch=0;
      for (Json::ValueConstIterator it = books.begin(); it != books.end(); ++it)
	{
	  const Json::Value& book = *it;
	  uint8_t sid=(*it).asUInt();
	  set6bDac(ch,sid);
	  ch++;
	}
      const Json::Value& books1 = _jasic["InputDac"];
      ch=0;
      for (Json::ValueConstIterator it = books1.begin(); it != books1.end(); ++it)
	{
	  const Json::Value& book = *it;
	  uint8_t sid=(*it).asUInt();
	  setInputDac(ch,sid);
	  ch++;
	}
      const Json::Value& books2 = _jasic["InputDacCommand"];
      ch=0;
      for (Json::ValueConstIterator it = books2.begin(); it != books2.end(); ++it)
	{
	  const Json::Value& book = *it;
	  uint8_t sid=(*it).asUInt();
	  setInputDacCommand(ch,sid);
	  ch++;
	}
      const Json::Value& books3 = _jasic["MaskDiscriCharge"];
      ch=0;
      for (Json::ValueConstIterator it = books3.begin(); it != books3.end(); ++it)
	{
	  const Json::Value& book = *it;
	  uint8_t sid=(*it).asUInt();
	  setMaskDiscriCharge(ch,sid);
	  ch++;
	}
      const Json::Value& books4 = _jasic["MaskDiscriTime"];
      ch=0;
      for (Json::ValueConstIterator it = books4.begin(); it != books4.end(); ++it)
	{
	  const Json::Value& book = *it;
	  uint8_t sid=(*it).asUInt();
	  setMaskDiscriTime(ch,sid);
	  ch++;
	}
      setCf0_1p25pF(_jasic["Cf0_1p25pF"].asUInt());
      setCf1_2p5pF(_jasic["Cf1_2p5pF"].asUInt());
      setCf2_200fF(_jasic["Cf2_200fF"].asUInt());
      setCf3_100fF(_jasic["Cf3_100fF"].asUInt());
      setDIS_razchn_ext(_jasic["DIS_razchn_ext"].asUInt());
      setDIS_razchn_int(_jasic["DIS_razchn_int"].asUInt());
      setDIS_trig_mux(_jasic["DIS_trig_mux"].asUInt());
      setDIS_triggers(_jasic["DIS_triggers"].asUInt());
      setDacDelay(_jasic["DacDelay"].asUInt());
      setEN10bDac(_jasic["EN10bDac"].asUInt());
      setEN_80M(_jasic["EN_80M"].asUInt());
      setEN_NOR32_charge(_jasic["EN_NOR32_charge"].asUInt());
      setEN_NOR32_time(_jasic["EN_NOR32_time"].asUInt());
      setEN_adc(_jasic["EN_adc"].asUInt());
      setEN_bias_6bit_dac(_jasic["EN_bias_6bit_dac"].asUInt());
      setEN_bias_charge(_jasic["EN_bias_charge"].asUInt());
      setEN_bias_dac_delay(_jasic["EN_bias_dac_delay"].asUInt());
      setEN_bias_discri(_jasic["EN_bias_discri"].asUInt());
      setEN_bias_discri_adc_charge(_jasic["EN_bias_discri_adc_charge"].asUInt());
      setEN_bias_discri_adc_time(_jasic["EN_bias_discri_adc_time"].asUInt());
      setEN_bias_discri_charge(_jasic["EN_bias_discri_charge"].asUInt());
      setEN_bias_pa(_jasic["EN_bias_pa"].asUInt());
      setEN_bias_ramp_delay(_jasic["EN_bias_ramp_delay"].asUInt());
      setEN_bias_sca(_jasic["EN_bias_sca"].asUInt());
      setEN_bias_tdc(_jasic["EN_bias_tdc"].asUInt());
      setEN_discri_delay(_jasic["EN_discri_delay"].asUInt());
      setEN_dout_oc(_jasic["EN_dout_oc"].asUInt());
      setEN_fast_lvds_rec(_jasic["EN_fast_lvds_rec"].asUInt());
      setEN_slow_lvds_rec(_jasic["EN_slow_lvds_rec"].asUInt());
      setEN_temp_sensor(_jasic["EN_temp_sensor"].asUInt());
      setEN_transmit(_jasic["EN_transmit"].asUInt());
      setEN_transmitter(_jasic["EN_transmitter"].asUInt());
      setON_OFF_1mA(_jasic["ON_OFF_1mA"].asUInt());
      setON_OFF_2mA(_jasic["ON_OFF_2mA"].asUInt());
      setON_OFF_input_dac(_jasic["ON_OFF_input_dac"].asUInt());
      setON_OFF_otaQ(_jasic["ON_OFF_otaQ"].asUInt());
      setON_OFF_ota_mux(_jasic["ON_OFF_ota_mux"].asUInt());
      setON_OFF_ota_probe(_jasic["ON_OFF_ota_probe"].asUInt());
      setPP10bDac(_jasic["PP10bDac"].asUInt());
      setPP_adc(_jasic["PP_adc"].asUInt());
      setPP_bias_6bit_dac(_jasic["PP_bias_6bit_dac"].asUInt());
      setPP_bias_charge(_jasic["PP_bias_charge"].asUInt());
      setPP_bias_dac_delay(_jasic["PP_bias_dac_delay"].asUInt());
      setPP_bias_discri(_jasic["PP_bias_discri"].asUInt());
      setPP_bias_discri_adc_charge(_jasic["PP_bias_discri_adc_charge"].asUInt());
      setPP_bias_discri_adc_time(_jasic["PP_bias_discri_adc_time"].asUInt());
      setPP_bias_discri_charge(_jasic["PP_bias_discri_charge"].asUInt());
      setPP_bias_pa(_jasic["PP_bias_pa"].asUInt());
      setPP_bias_ramp_delay(_jasic["PP_bias_ramp_delay"].asUInt());
      setPP_bias_sca(_jasic["PP_bias_sca"].asUInt());
      setPP_bias_tdc(_jasic["PP_bias_tdc"].asUInt());
      setPP_discri_delay(_jasic["PP_discri_delay"].asUInt());
      setPP_fast_lvds_rec(_jasic["PP_fast_lvds_rec"].asUInt());
      setPP_slow_lvds_rec(_jasic["PP_slow_lvds_rec"].asUInt());
      setPP_temp_sensor(_jasic["PP_temp_sensor"].asUInt());
      setPP_transmitter(_jasic["PP_transmitter"].asUInt());
      setSEL_80M(_jasic["SEL_80M"].asUInt());
      setVthDiscriCharge(_jasic["VthDiscriCharge"].asUInt());
      setVthTime(_jasic["VthTime"].asUInt());
      setcmd_polarity(_jasic["cmd_polarity"].asUInt());
      setlatch(_jasic["latch"].asUInt());
      setsel_starb_ramp_adc_ext(_jasic["sel_starb_ramp_adc_ext"].asUInt());
      setusebcompensation(_jasic["usebcompensation"].asUInt());
    
    
    }
    void loadConfig(std::string fname)
    {
      PRSlow s;
      uint8_t* buf=(uint8_t*) ptr();
      uint8_t slcBytes=0;
      FILE *fp=fopen(fname.c_str(), "r");
      int ier;
      do
	{
	  char b;
	  ier=fscanf(fp,"%x",&b);
	  memcpy(&buf[79-slcBytes],&b,1);
	  printf("%d %x %x\n",79-slcBytes,buf[79-slcBytes],_l[(79-slcBytes)/4]);
	  slcBytes++;
	} while (ier!=EOF && slcBytes<80);
      fclose(fp);
      //loadInvert(s);
      //Print();
      //getchar();

    }
    uint32_t* ptr(){return _l;}
    // Now real access to register function
    bool getMaskDiscriCharge(int b){return getBit(b%32);}
    void setMaskDiscriCharge(int b,bool on)  { setBitState(b,on);}
    uint8_t getInputDac(uint8_t ch){return getByte(32+ch*9);}
    void setInputDac(uint8_t ch,uint8_t val){setByte(32+ch*9,val,8);}
    bool getInputDacCommand(int ch){return getBit(32+ch*9+8);}
    void setInputDacCommand(int ch,bool on)  { setBitState(32+ch*9+8,on);}

    bool getMaskDiscriTime(int b){return getBit(328+b);}
    void setMaskDiscriTime(int b,bool on)  { setBitState(328+b,on);}
    uint8_t get6bDac(uint8_t ch){return getByte(360+ch*6)&0x3F;}
    void set6bDac(uint8_t ch,uint8_t val){setByte(360+ch*6,val&0x3F,6);}
    bool getEN10bDac(){return getBit(552);}
    void setEN10bDac(bool on)  { setBitState(552,on);}
    bool getPP10bDac(){return getBit(553);}
    void setPP10bDac(bool on)  { setBitState(553,on);}

    uint16_t getVthDiscriCharge()
    {
      uint16_t r=0;
      for (int i=1;i<=10;i++)
	if (getBit(554+i-1)) r|=(1<<(10-i));
      return r;
    }
    void setVthDiscriCharge(uint16_t val)
    {
      uint16_t r=val&0x3FF;
      for (int i=1;i<=10;i++)
	setBitState(554+i-1,r&(1<<(10-i)));

    }
    uint16_t getVthTime()
    {
      uint16_t r=0;
      for (int i=1;i<=10;i++)
	if (getBit(564+i-1)) r|=(1<<(10-i));
      return r;
    }
    void setVthTime(uint16_t val)
    {
      uint16_t r=val&0x3FF;
      for (int i=1;i<=10;i++)
	setBitState(564+i-1,r&(1<<(10-i)));
   
    }

    uint8_t getDacDelay(){return getByte(582);}
    void setDacDelay(uint8_t val){setByte(582,val,8);}
    void setEN_adc(bool t){setBitState(574,t);}
    void setPP_adc(bool t){setBitState(575,t);}
    void setsel_starb_ramp_adc_ext(bool t){setBitState(576,t);}
    void setusebcompensation(bool t){setBitState(577,t);}
    void setEN_bias_dac_delay(bool t){setBitState(578,t);}
    void setPP_bias_dac_delay(bool t){setBitState(579,t);}
    void setEN_bias_ramp_delay(bool t){setBitState(580,t);}
    void setPP_bias_ramp_delay(bool t){setBitState(581,t);}
    void setEN_discri_delay(bool t){setBitState(590,t);}
    void setPP_discri_delay(bool t){setBitState(591,t);}
    void setEN_temp_sensor(bool t){setBitState(592,t);}
    void setPP_temp_sensor(bool t){setBitState(593,t);}
    void setEN_bias_pa(bool t){setBitState(594,t);}
    void setPP_bias_pa(bool t){setBitState(595,t);}
    void setEN_bias_discri(bool t){setBitState(596,t);}
    void setPP_bias_discri(bool t){setBitState(597,t);}
    void setcmd_polarity(bool t){setBitState(598,t);}
    void setlatch(bool t){setBitState(599,t);}
    void setEN_bias_6bit_dac(bool t){setBitState(600,t);}
    void setPP_bias_6bit_dac(bool t){setBitState(601,t);}
    void setEN_bias_tdc(bool t){setBitState(602,t);}
    void setPP_bias_tdc(bool t){setBitState(603,t);}
    void setON_OFF_input_dac(bool t){setBitState(604,t);}
    void setEN_bias_charge(bool t){setBitState(605,t);}
    void setPP_bias_charge(bool t){setBitState(606,t);}
    void setCf3_100fF(bool t){setBitState(607,t);}
    void setCf2_200fF(bool t){setBitState(608,t);}
    void setCf1_2p5pF(bool t){setBitState(609,t);}
    void setCf0_1p25pF(bool t){setBitState(610,t);}
    void setEN_bias_sca(bool t){setBitState(611,t);}
    void setPP_bias_sca(bool t){setBitState(612,t);}
    void setEN_bias_discri_charge(bool t){setBitState(613,t);}
    void setPP_bias_discri_charge(bool t){setBitState(614,t);}
    void setEN_bias_discri_adc_time(bool t){setBitState(615,t);}
    void setPP_bias_discri_adc_time(bool t){setBitState(616,t);}
    void setEN_bias_discri_adc_charge(bool t){setBitState(617,t);}
    void setPP_bias_discri_adc_charge(bool t){setBitState(618,t);}
    void setDIS_razchn_int(bool t){setBitState(619,t);}
    void setDIS_razchn_ext(bool t){setBitState(620,t);}
    void setSEL_80M(bool t){setBitState(621,t);}
    void setEN_80M(bool t){setBitState(622,t);}
    void setEN_slow_lvds_rec(bool t){setBitState(623,t);}
    void setPP_slow_lvds_rec(bool t){setBitState(624,t);}
    void setEN_fast_lvds_rec(bool t){setBitState(625,t);}
    void setPP_fast_lvds_rec(bool t){setBitState(626,t);}
    void setEN_transmitter(bool t){setBitState(627,t);}
    void setPP_transmitter(bool t){setBitState(628,t);}
    void setON_OFF_1mA(bool t){setBitState(629,t);}
    void setON_OFF_2mA(bool t){setBitState(630,t);}
    void setON_OFF_otaQ(bool t){setBitState(631,t);}
    void setON_OFF_ota_mux(bool t){setBitState(632,t);}
    void setON_OFF_ota_probe(bool t){setBitState(633,t);}
    void setDIS_trig_mux(bool t){setBitState(634,t);}
    void setEN_NOR32_time(bool t){setBitState(635,t);}
    void setEN_NOR32_charge(bool t){setBitState(636,t);}
    void setDIS_triggers(bool t){setBitState(637,t);}
    void setEN_dout_oc(bool t){setBitState(638,t);}
    void setEN_transmit(bool t){setBitState(639,t);}
    bool getEN_adc(){return getBit(574);}
    bool getPP_adc(){return getBit(575);}
    bool getsel_starb_ramp_adc_ext(){return getBit(576);}
    bool getusebcompensation(){return getBit(577);}
    bool getEN_bias_dac_delay(){return getBit(578);}
    bool getPP_bias_dac_delay(){return getBit(579);}
    bool getEN_bias_ramp_delay(){return getBit(578);}
    bool getPP_bias_ramp_delay(){return getBit(579);}
    bool getEN_discri_delay(){return getBit(590);}
    bool getPP_discri_delay(){return getBit(591);}
    bool getEN_temp_sensor(){return getBit(592);}
    bool getPP_temp_sensor(){return getBit(593);}
    bool getEN_bias_pa(){return getBit(594);}
    bool getPP_bias_pa(){return getBit(595);}
    bool getEN_bias_discri(){return getBit(596);}
    bool getPP_bias_discri(){return getBit(597);}
    bool getcmd_polarity(){return getBit(598);}
    bool getlatch(){return getBit(599);}
    bool getEN_bias_6bit_dac(){return getBit(600);}
    bool getPP_bias_6bit_dac(){return getBit(601);}
    bool getEN_bias_tdc(){return getBit(602);}
    bool getPP_bias_tdc(){return getBit(603);}
    bool getON_OFF_input_dac(){return getBit(604);}
    bool getEN_bias_charge(){return getBit(605);}
    bool getPP_bias_charge(){return getBit(606);}
    bool getCf3_100fF(){return getBit(607);}
    bool getCf2_200fF(){return getBit(608);}
    bool getCf1_2p5pF(){return getBit(609);}
    bool getCf0_1p25pF(){return getBit(610);}
    bool getEN_bias_sca(){return getBit(611);}
    bool getPP_bias_sca(){return getBit(612);}
    bool getEN_bias_discri_charge(){return getBit(613);}
    bool getPP_bias_discri_charge(){return getBit(614);}
    bool getEN_bias_discri_adc_time(){return getBit(615);}
    bool getPP_bias_discri_adc_time(){return getBit(616);}
    bool getEN_bias_discri_adc_charge(){return getBit(617);}
    bool getPP_bias_discri_adc_charge(){return getBit(618);}
    bool getDIS_razchn_int(){return getBit(619);}
    bool getDIS_razchn_ext(){return getBit(620);}
    bool getSEL_80M(){return getBit(621);}
    bool getEN_80M(){return getBit(622);}
    bool getEN_slow_lvds_rec(){return getBit(623);}
    bool getPP_slow_lvds_rec(){return getBit(624);}
    bool getEN_fast_lvds_rec(){return getBit(625);}
    bool getPP_fast_lvds_rec(){return getBit(626);}
    bool getEN_transmitter(){return getBit(627);}
    bool getPP_transmitter(){return getBit(628);}
    bool getON_OFF_1mA(){return getBit(629);}
    bool getON_OFF_2mA(){return getBit(630);}
    bool getON_OFF_otaQ(){return getBit(631);}
    bool getON_OFF_ota_mux(){return getBit(632);}
    bool getON_OFF_ota_probe(){return getBit(633);}
    bool getDIS_trig_mux(){return getBit(634);}
    bool getEN_NOR32_time(){return getBit(635);}
    bool getEN_NOR32_charge(){return getBit(636);}
    bool getDIS_triggers(){return getBit(637);}
    bool getEN_dout_oc(){return getBit(638);}
    bool getEN_transmit(){return getBit(639);}

    void loadInvert(PRSlow r)
    {

      for (int i=639;i>=0;i--)
	setBitState(639-i,r.getBit(i)==1);

    }
    void prepare4Tdc(uint16_t* adr,uint16_t* val,uint16_t offset=0)
    {
      std::stringstream sbits;
      uint8_t* b=(uint8_t*) _l;
      for (int i=0;i<80;i++)
	{
	  adr[i+offset]=i+offset;
	  val[i+offset]=b[79-i];
	  printf("%d %x \n",adr[i+offset],val[i+offset]);
	  std::bitset<8> sb(b[79-i]);
	  sbits<<sb<<" ";
	}
      std::cout<<sbits.str()<<std::endl;
    }
    Json::Value& getJson(){return _jasic;}
    void setJson(Json::Value v){_jasic=v; setFromJson();}
  private:
    uint32_t _l[20];
    Json::Value _jasic;
  };
};
#endif
