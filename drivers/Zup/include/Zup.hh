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
#include <json/json.h>
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
    Json::Value Status();
  private:


    int fd1,portstatus;
    std::string _value;

    char buff[1024];
    float _vSet,_vRead,_iSet,_iRead;
    int _status;
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
