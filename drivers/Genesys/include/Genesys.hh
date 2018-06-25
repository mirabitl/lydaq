#ifndef _lydaq_Genesys_H_
#define _lydaq_Genesys_H_
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
#include <time.h>

#include <unistd.h>
#include <string>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <boost/format.hpp>
#include "ReadoutLogger.hh"

using namespace std;
namespace lydaq
{
  class Genesys
  {
  public:
    void setIos();
    Genesys(std::string device,uint32_t address);
    ~Genesys();
    void ON();
    void OFF();
    void readCommand(std::string cmd);
    void INFO();
  
    float ReadVoltageSet();
    float ReadVoltageUsed();
    float ReadCurrentUsed();
    std::string readValue();
  private:


    int fd1;

    unsigned char buff[1024];
    std::string _value;
    int wr,rd,nbytes,tries;
    float _vSet,_vRead,_iSet,_iRead;
    time_t _lastInfo;
  };
};
#ifdef _USE_MAIN_EXAMPLE_
int main()
{
  //
  // Open the serial port. 
  //
  Genesys* z=new Genesys("/dev/ttyUSB1",6);

  printf("Set Volatge\n");
  //z->readCommand("PV 4.50\r");
  z->INFO();
  getchar();
  printf("%f %f %f \n",z->ReadVoltageSet(),z->ReadVoltageUsed(),z->ReadCurrentUsed());
  // printf("%f %f %f \n",z.ReadVoltageSet(),z.ReadVoltageUsed(),z.ReadCurrentUsed());
  z->OFF();
  getchar();
  z->ON();
  // getchar();
  // //z->INFO();
  /*
    printf("Read Volatge\n");
    printf("%f %f %f \n",z->ReadVoltageSet(),z->ReadVoltageUsed(),z->ReadCurrentUsed());
    z->OFF();
    getchar();
    z->INFO();
  
    z->ON();
    getchar();
    z->INFO();
  */
  // // getchar();
}
#endif
#endif
