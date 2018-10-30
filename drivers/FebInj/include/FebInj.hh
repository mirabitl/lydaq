#ifndef _FebInj_H_
#define _FebInj_H_

#include <iostream>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdio.h>

#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <math.h>
#include <dirent.h>

#include <wiringPi.h>
#include <wiringPiSPI.h>


/**
x"00" => w_enableHigh0_7_reg 
x"01" => w_enableHigh8_15_reg
x"02" => w_enableHigh16_23_reg
default = x"FFFFFF" -> tout actif
x"03" => w_enableLow0_7_reg
x"04" => w_enableLow8_15_reg
x"05" => w_enableLow16_23_reg
default =x"000000" -> tout inactif
x"06" => w_TriggerSource_reg
default = x"02"
x"01" -> interne single
x"02" -> interne multiple
x"04" -> externe single 
x"08" ->  externe multiple
x"10" -> software 
x"07" => w_SoftTrigger_reg
default =x"00"
x"01" -> bit de trigger
x"08" => w_TriggerNumber0_7_reg
x"09" => w_TriggerNumber8_15_reg
x"0A" => w_TriggerNumber16_23_reg
x"0B" => w_TriggerNumber24_31_reg
default= x"000010"
x"0C" => w_TriggerDelay0_7_reg
x"0D" => w_TriggerDelay8_15_reg
x"0E" => w_TriggerDelay16_23_reg
x"0F" => w_TriggerDelay24_31_reg
default =x"000000"
x"10" => w_TriggerDuration0_7_reg
x"11" => w_TriggerDuration8_15_reg
x"12" => w_TriggerDuration16_23_reg
x"13" => w_TriggerDuration24_31_reg
default = x"0000FF"
x"14" => w_mcp4021_value_reg
default = x"00"
bit 0-6 : value 
bit 7 : 1: enable loading
          0 : do nothing
x"15" => w_software_veto_reg
default = x"00"
bit 0: 0 : no pulse for external trigger  
         1 : external trigger enabled

 */


namespace lydaq {
  
  class FebInj
  {
  public:
    FebInj();
    ~FebInj();
    
    static void myAnalogWrite (struct wiringPiNodeStruct *node, int addr, int value);
    static int myAnalogRead (struct wiringPiNodeStruct *node, int addr);
    void setMask(uint32_t mask,uint32_t hr);
    void setTriggerSource(uint32_t source);
    void softwareTrigger();
    void pauseExternalTrigger();
    void resumeExternalTrigger();
    void setNumberOfTrigger(uint32_t n);
    void setDelay(uint32_t n);
    void setDuration(uint32_t n);
    void setPulseHeight(uint32_t n);
  private:
    struct wiringPiNodeStruct *node ;
	


  };
};
#endif
