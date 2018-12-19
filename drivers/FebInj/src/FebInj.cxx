#include "FebInj.hh"
using namespace lydaq;

lydaq::FebInj::FebInj()
{
  if (wiringPiSPISetup (0, 10000) < 0)	// 10MHz Max
    exit(0);

  node = wiringPiNewNode (64, 2) ;

  node->fd          = 0 ;
  node->analogWrite = myAnalogWrite ;
  node->analogRead = myAnalogRead ;

}
 
lydaq::FebInj::~FebInj()
{
  delete node;
}



void lydaq::FebInj::myAnalogWrite (struct wiringPiNodeStruct *node, int addr, int value)
{

unsigned char spiData [2] ;

  spiData [0] = (addr&0x7f) ;
  spiData [1] = value&0xff ;

  wiringPiSPIDataRW (node->fd, spiData, 2) ;
}

int lydaq::FebInj::myAnalogRead (struct wiringPiNodeStruct *node, int addr)
{
  unsigned char spiData [2] ;


  spiData [0] = (addr&0x7f) +0x80;
  spiData [1] = 0 ;

  wiringPiSPIDataRW (node->fd, spiData, 2) ;
  return spiData [1];
}

void lydaq::FebInj::setMask(uint32_t mask,uint32_t hr)
{

  uint8_t c[3];
  for (int i=0;i<3;i++)
    {
      c[i]=(mask>>(i*8))&0xFF;
      if (hr==0)
	{
	myAnalogWrite(node,i,c[i]);
	fprintf(stderr," HR = %d MASK= %x Byte %d ByteMask %x \n",hr,mask,i,c[i]);
	}
      else
	{
	myAnalogWrite(node,i+3,c[i]);
      	fprintf(stderr," HR = %d MASK= %x Byte %d ByteMask %x \n",hr,mask,i+3,c[i]);
	}
      usleep(1000);
    }
}
void lydaq::FebInj::setTriggerSource(uint32_t source)
{
  myAnalogWrite(node,6,source &0xFF);
}
void lydaq::FebInj::softwareTrigger()
{
  myAnalogWrite(node,7,1);
  usleep(100);
  myAnalogWrite(node,7,0);
}
void lydaq::FebInj::internalTrigger()
{
  myAnalogWrite(node,7,2);
  usleep(100);
  myAnalogWrite(node,7,0);
}
void lydaq::FebInj::pauseExternalTrigger()
{
  myAnalogWrite(node,0x15,0);
}
void lydaq::FebInj::resumeExternalTrigger()
{
  myAnalogWrite(node,0x15,1);
}

void lydaq::FebInj::setNumberOfTrigger(uint32_t n)
{

  uint8_t c[4];
  for (int i=0;i<4;i++)
    {
      c[i]=(n>>(i*8))&0xFF;
      myAnalogWrite(node,i+0x8,c[i]);
      usleep(1);
    }
  
}
void lydaq::FebInj::setDelay(uint32_t n)
{

  uint8_t c[4];
  for (int i=0;i<4;i++)
    {
      c[i]=(n>>(i*8))&0xFF;
      myAnalogWrite(node,i+0xc,c[i]);
      usleep(1);
    }
  
}
void lydaq::FebInj::setDuration(uint32_t n)
{

  uint8_t c[4];
  for (int i=0;i<4;i++)
    {
      c[i]=(n>>(i*8))&0xFF;
      myAnalogWrite(node,i+0x10,c[i]);
      usleep(1);
    }
  
}
void lydaq::FebInj::setPulseHeight(uint32_t n)
{

  uint8_t c=n&0x7f+128;
  myAnalogWrite(node,0x14,c);
    
  
}

