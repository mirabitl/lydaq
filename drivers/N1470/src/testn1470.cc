#include "N1470.hh"
using namespace lydaq;
int main()
{
  //
  // Open the serial port. 
  //
  N1470* z=new N1470("/dev/ttyUSB0",2);


  //z->readCommand("PV 4.50\r");
  z->setRampUp(0,5);
  z->setRampDown(0,50);
  z->setRampUp(1,5);
  z->setRampDown(1,50);
  z->status(0);
  getchar();
  z->status(1);
  getchar();
  z->status(2);
  getchar();
  z->status(3);
  getchar();
  
  
}
