#ifndef _hih8000_H_
#define _hih8000_H_

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
#include <linux/i2c-dev.h>

#include <wiringPi.h>
#include <wiringPiI2C.h>

#define TEMP 0x00
#define CONFIG 0x01
#define CONFIG  0x01
#define DEV_ADDR 0x4A
#define HUM_ADDR 0x27

#define DELAY  5000000

using namespace std;



class hih8000
{
public:
  hih8000();
  ~hih8000();



  int Setup ();
  int Read();
 
  double humidity(int i){return _humidity[i];}
  double temperature(int i){return _temperature[i];}
private:
  int _fd;
  double _humidity[2];
  double _temperature[2];

};
#endif
