#include "Zup.hh"
using namespace lydaq;
int main()
{
  //
  // Open the serial port. 
  //
  Zup* z=new Zup("/dev/ttyUSB0",1);

  printf("Set Volatge\n");
  //z->readCommand("PV 4.50\r");
  std::cout<<z->Status()<<std::endl;
   getchar();
  z->ON();
  std::cout<<"turn On "<<z->Status()<<std::endl;
   getchar();
  z->OFF();
  std::cout<<"turn Off "<<z->Status()<<std::endl;
  


   getchar();
   return 0;
  //printf("%f %f %f \n",z->ReadVoltageSet(),z->ReadVoltageUsed(),z->ReadCurrentUsed());
  // printf("%f %f %f \n",z.ReadVoltageSet(),z.ReadVoltageUsed(),z.ReadCurrentUsed());
  //z->OFF();
  //getchar();
  //z->ON();
  // getchar();
  int n=0;
  while(1)
    {
      z->INFO();
      printf("%d \n",n++);
    }
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
