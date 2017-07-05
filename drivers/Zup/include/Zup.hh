#ifndef _lydaq_Zup_H_
#define _lydaq_Zup_H_
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
#include <string>
#include <iostream>

namespace lydaq
{
  class Zup
  {
  public:
    Zup(std::string device,uint32_t address);
    ~Zup();
    void ON();
    void OFF();
    void readCommand(std::string cmd);
    void INFO();
    float ReadVoltageSet();
    float ReadVoltageUsed();
    float ReadCurrentUsed();
  private:


    int fd1;

    char buff[100];

    int wr,rd,nbytes,tries;
  };
};
#ifdef _USE_MAIN_EXAMPLE_
using namespace lydaq;
int main()
{
  //
  // Open the serial port. 
  //
  Zup z("/dev/ttyUSB0",1);
  //z.INFO();
  printf("%f %f %f \n",z.ReadVoltageSet(),z.ReadVoltageUsed(),z.ReadCurrentUsed());
  //getchar();
  z.ON();
  getchar();
  //z.INFO();
  printf("%f %f %f \n",z.ReadVoltageSet(),z.ReadVoltageUsed(),z.ReadCurrentUsed());
  z.OFF();
  getchar();
  //z.INFO();
  printf("%f %f %f \n",z.ReadVoltageSet(),z.ReadVoltageUsed(),z.ReadCurrentUsed());
  // getchar();
}

#endif
#endif
