#include "BMP183.hh"
using namespace lydaq;

lydaq::BMP183::BMP183(void)
{
  BMP183Setup () ;
  BMP183GetCalibration();
}
 
lydaq::BMP183::~BMP183()
{
}


int lydaq::BMP183::BMP183Setup (void)
{
  unsigned char spiData [2] ;

  if (wiringPiSPISetup (0, 10000) < 0)	// 10MHz Max
    return -1 ;

  node = wiringPiNewNode (64, 2) ;

  node->fd          = 0 ;
  node->analogWrite = myAnalogWrite ;
  node->analogRead = myAnalogRead ;

  return 0 ;
}

void lydaq::BMP183::myAnalogWrite (struct wiringPiNodeStruct *node, int addr, int value)
{
  unsigned char spiData [2] ;


  spiData [0] = (addr&0x7f) ;
  spiData [1] = value&0xff ;

  wiringPiSPIDataRW (node->fd, spiData, 2) ;
}

int lydaq::BMP183::myAnalogRead (struct wiringPiNodeStruct *node, int addr)
{
  unsigned char spiData [2] ;


  spiData [0] = (addr&0x7f) +0x80;
  spiData [1] = 0 ;

  wiringPiSPIDataRW (node->fd, spiData, 2) ;
  return spiData [1];
}

int lydaq::BMP183::BMP183GetCalibration(void)
{ 
  int tlsb, tmsb;

  tmsb=myAnalogRead(node, CAL_AC1);
  tlsb=myAnalogRead(node, CAL_AC1+1);
  AC1=tlsb+(tmsb<<8);
	
  tmsb=myAnalogRead(node, CAL_AC2);
  tlsb=myAnalogRead(node, CAL_AC2+1);
  AC2=tlsb+(tmsb<<8);

  tmsb=myAnalogRead(node, CAL_AC3);
  tlsb=myAnalogRead(node, CAL_AC3+1);
  AC3=tlsb+(tmsb<<8);

  tmsb=myAnalogRead(node, CAL_AC4);
  tlsb=myAnalogRead(node, CAL_AC4+1);
  AC4=tlsb+(tmsb<<8);

  tmsb=myAnalogRead(node, CAL_AC5);
  tlsb=myAnalogRead(node, CAL_AC5+1);
  AC5=tlsb+(tmsb<<8);

  tmsb=myAnalogRead(node, CAL_AC6);
  tlsb=myAnalogRead(node, CAL_AC6+1);
  AC6=tlsb+(tmsb<<8);

  tmsb=myAnalogRead(node, CAL_B1);
  tlsb=myAnalogRead(node, CAL_B1+1);
  B1=tlsb+(tmsb<<8);

  tmsb=myAnalogRead(node, CAL_B2);
  tlsb=myAnalogRead(node, CAL_B2+1);
  B2=tlsb+(tmsb<<8);

  tmsb=myAnalogRead(node, CAL_MB);
  tlsb=myAnalogRead(node, CAL_MB+1);
  MB=tlsb+(tmsb<<8);

  tmsb=myAnalogRead(node, CAL_MC);
  tlsb=myAnalogRead(node, CAL_MC+1);
  MC=tlsb+(tmsb<<8);

  tmsb=myAnalogRead(node, CAL_MD);
  tlsb=myAnalogRead(node, CAL_MD+1);
  MD=tlsb+(tmsb<<8);
	
  printf ("AC5=%d AC6=%d MC=%d MD=%d\n", AC5, AC6, MC, MD);
}

float lydaq::BMP183::BMP183TemperatureRead(void)
{
  int temp_msb, temp_lsb,UT;
  int X1, X2;
  int T;
  float temperature;

  myAnalogWrite(node, CTRL_MEAS, 0x2E);
  usleep (20000); //20ms
  temp_msb=myAnalogRead(node, DATA);
  temp_lsb=myAnalogRead(node, DATA+1);
	
  UT=(temp_msb<<8)+temp_lsb;
  X1 = (((UT - AC6) * AC5 )>>15);
  X2 = (MC <<11) / (X1 + MD); 
	
  B5= X1+X2; // mandatory for pressure calculation
  temperature = ((X1+X2) + 8) >>4;
  temperature =temperature/10;
  printf ("temperature = %f deg C\n",temperature);
  return temperature;
}

float lydaq::BMP183::BMP183PressionRead(void)
{
  float pression;
  int UP1_lsb, UP1_msb, UP1_xsb;	
  long UP1, B3,  B6;	
  long X1, X2, X3;
  long p;
  long B7;
  unsigned long B4;
  unsigned int oss=3;
  myAnalogWrite(node, CTRL_MEAS, 0x34+(oss<<6) );
  //myAnalogWrite(node, CTRL_MEAS, 0x34 |(0x3<<4));
  usleep (50000);//50ms
  UP1_msb=myAnalogRead(node, DATA);
  UP1_lsb=myAnalogRead(node, DATA+1);
  UP1_xsb=myAnalogRead(node, DATA+2);

  UP1 = ((UP1_msb<<16)+(UP1_lsb<<8)+UP1_xsb);
  UP1=UP1>>(8-oss);
  // Calculate atmospheric pressure in [Pa]
  B6 = B5 - 4000;
  X1 = (B2 * (B6 * B6)>>12)>>11;
  X2 = (AC2 * B6) >>11;
  X3 = X1 + X2;
  B3 = (((AC1 * 4 + X3) << oss) + 2 ) / 4;
  X1 = (AC3 * B6)>>13;
  X2 = (B1 * (B6 * B6)>>12)>>16;
  X3 = (X1 + X2 + 2)>>2;
  B4 = (AC4 * (unsigned long)(X3 + 32768)) >>15;
  B7 = ((unsigned long)UP1 - B3) * (50000>>oss);
  if (B7<0x80000000)
    p=(B7*2)/B4;
  else
    p =  (B7/ B4)*2;
  X1 = (p >>8) * ( p >>8);
  X1 = (X1 * 3038) >>16;
  X2 = (-7357 * p) >>16;
  pression= p + ((X1 + X2 +3791)>>4);
  pression= pression/100.0;
  printf ("pression = %f hPa \n",pression);
  return pression;
}
