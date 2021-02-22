#include <iostream>
#include <stdint.h>
#include <stdio.h>

#include "HVCaenInterface.hh"
#include <unistd.h>

int main()
{
 lydaq::HVCaenInterface h("lyoac28","admin","admin");

 while (true)
   {
     int slot=0;
     for (int ch=0;ch<6;ch++)
       {
	 h.Connect();
	 std::cout<<h.ChannelInfo(slot,ch);
	 /*
	 float vmon=h.GetVoltageRead(ch);
	 float imon=h.GetCurrentRead(ch);
	 int status =h.GetStatus(ch);
	   printf("%d %f %f %d \n",ch,vmon,imon,status);
	 */
       }
     // for (int ch=24;ch<30;ch++)
     //   {
     // 	  float vmon=h.GetVoltageRead(ch);
     // 	 float imon=h.GetCurrentRead(ch);
     // 	 int status =h.GetStatus(ch);
     // 	   printf("%d %f %f %d \n",ch,vmon,imon,status);
     //   }
     ::sleep(5);
   }
}
