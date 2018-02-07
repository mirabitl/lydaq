#ifndef _GPIO_H_
#define _GPIO_H_

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

#define IN  0
#define OUT 1
 
#define LOW  0
#define HIGH 1
 
#define PVME  24 /* P1-16 */
#define PDIF 23  /* P1-18 */

#define BUFFER_MAX 3
#define DIRECTION_MAX 35
#define VALUE_MAX 30
namespace lydaq
{
class GPIO
{
public:
  GPIO();
  ~GPIO();
	
  int32_t freeGPIO ();

  static int	GPIOExport(int32_t pin);
  static int32_t GPIOUnexport(int32_t pin);
  static int32_t GPIODirection(int32_t pin, int32_t dir);
  int32_t GPIORead(int32_t pin);
  int32_t GPIOWrite(int32_t pin, int32_t value);
  void DIFON();
  void DIFOFF();
  void VMEON();
  void VMEOFF();
  void INFO();
  int32_t getDIFPower();
  int32_t getVMEPower();
private:

	int32_t DIFPower;
	int32_t VMEPower;

};
};
#endif
