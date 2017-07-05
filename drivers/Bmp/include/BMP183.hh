#ifndef _BMP183_H_
#define _BMP183_H_

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

#define CAL_AC1 0xAA
#define CAL_AC2 0xAC
#define CAL_AC3 0xAE
#define CAL_AC4 0xB0
#define CAL_AC5 0xB2
#define CAL_AC6 0xB4
#define CAL_B1 0xB6
#define CAL_B2 0xB8 
#define CAL_MB 0xBA
#define CAL_MC 0xBC
#define CAL_MD 0xBE

#define CTRL_MEAS  0xF4
#define DATA 0xF6

namespace lydaq {
  
  class BMP183
  {
  public:
    BMP183(void);
    ~BMP183();



    int BMP183Setup (void);
    static void myAnalogWrite (struct wiringPiNodeStruct *node, int addr, int value);
  
    static int myAnalogRead (struct wiringPiNodeStruct *node, int addr);

    int BMP183GetCalibration(void);
    float BMP183TemperatureRead(void);

    float BMP183PressionRead(void);
 
  private:
    struct wiringPiNodeStruct *node ;
	
    unsigned char spiData [2] ;
    // calibration
    short AC1;
    short AC2;
    short AC3;
    unsigned short AC4;
    unsigned short AC5;
    unsigned short AC6;
    short B1;
    short B2;
    short MB;
    short MC;
    short MD;
	
    short B5;

  };
};
#endif
