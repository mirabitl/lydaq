#ifndef _BMP280_H_
#define _BMP280_H_

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
#include <linux/spi/spidev.h>
#include <sys/ioctl.h>
#include <stdio.h>

namespace lydaq {
  
  class BMP280
  {
  public:
    BMP280();
    ~BMP280();

    int Setup();
     void myAnalogWrite (uint32_t addr, uint32_t value);
  
     uint32_t myAnalogRead (uint32_t addr);

    int GetCalibration();
    int  TemperaturePressionRead(float *temperature, float *pression  );
 
  private:
    uint32_t _fd;
    uint16_t _dig_T1;
    int16_t _dig_T2;
    int16_t _dig_T3;

    uint16_t _dig_P1;
    int16_t _dig_P2;
    int16_t _dig_P3;
    int16_t _dig_P4;
    int16_t _dig_P5;
    int16_t _dig_P6;
    int16_t _dig_P7;
    int16_t _dig_P8;
    int16_t _dig_P9;

    float _Pres, _Temp;
    uint32_t _bits;
    uint32_t _speed;
	
	 
  };
};

#endif

