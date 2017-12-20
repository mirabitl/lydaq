#ifndef _lydaq_N1470_H_
#define _lydaq_N1470_H_
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

using namespace std;
namespace lydaq
{
  class N1470
  {
  public:
    void setIos();
    N1470(std::string device,uint32_t board);
    ~N1470();
    void setOn(uint32_t ch);
    void setOff(uint32_t ch);
    void status(uint32_t ch);
    void setVoltage(uint32_t ch, float val);
    void setCurrent(uint32_t ch, float val);
    void setRampUp(uint32_t ch, float val);
    void setRampDown(uint32_t ch, float val);
    void readCommand(std::string cmd);

  
    float voltageSet(uint32_t ch);
    float voltageUsed(uint32_t ch);
    float currentSet(uint32_t ch);
    float currentUsed(uint32_t ch);
    float rampUp(uint32_t ch);
    float rampDown(uint32_t ch);
    uint32_t statusBits(uint32_t ch);
    std::string readValue();
  private:

    uint32_t _board;
    int _fd;

    unsigned char _buff[1024];
    std::string _value;
    int wr,rd,nbytes,tries;
    float _vSet,_vMon,_iSet,_iMon,_rampUp,_rampDown;
    uint32_t _statusBits;
    time_t _lastInfo;
  };
};
#ifdef _USE_MAIN_EXAMPLE_
int main()
{
  //
  // Open the serial port. 
  //
  N1470* z=new N1470("/dev/ttyUSB1",6);

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
