#include "BMP280.hh"


using namespace lydaq;
lydaq::BMP280::BMP280()
{
  Setup () ;
  GetCalibration();
}
 
lydaq::BMP280::~BMP280()
{
}



int lydaq::BMP280::Setup ()
{
  _bits=8;
  _speed=10000;
  int ret = 0;
  /*
    mode |= SPI_CPHA;
    mode |= SPI_CPOL;
    mode |= SPI_LSB_FIRST;
    mode |= SPI_CS_HIGH;
  */

  uint32_t mode = 0;

	

  _fd = open("/dev/spidev0.0", O_RDWR);
  if (_fd < 0)
    {	
      perror("open fail");
      return -1;
    }
  ret = ioctl(_fd, SPI_IOC_WR_MODE32, &mode);
  if (ret == -1)		perror("can't set spi mode");

  ret = ioctl(_fd, SPI_IOC_WR_BITS_PER_WORD, &_bits);
  if (ret == -1)		perror("can't set bits per word");

	
  ret = ioctl(_fd, SPI_IOC_WR_MAX_SPEED_HZ, &_speed);
  if (ret <0 )		perror ("can't set max speed hz");
	
	
  if (myAnalogRead(0xD0) != 0x58)
    printf ("not a BMP280... \n");
  // reset	
  //	myAnalogWrite(0xE0, 0xB6);
  //	myAnalogWrite(0xE0, 0x00);
  // config  	
  myAnalogWrite(0xF5,(0<<5)+(16<<2)+0);	// t_sb=0 filter =16 spi4
  // ctrl_meas 	
  myAnalogWrite(0xF4,(1<<5)+(4<<2)+3);	// Tx1 Px4, normal 

  // read back 
  myAnalogRead(0xF4);
  myAnalogRead(0xF5);
  return 0 ;
}

void lydaq::BMP280::myAnalogWrite (uint32_t addr, uint32_t value)
{
  struct spi_ioc_transfer xfer[1];
  unsigned char buf[32];
  uint32_t  status;
	

  memset(xfer, 0, sizeof xfer);
  memset(buf, 0, sizeof buf);
	
  buf[0] = (addr &0x7f); // bit 7 = 0 for write
  buf[1]= value;
		
  xfer[0].speed_hz=_speed;
  xfer[0].bits_per_word=_bits;
  xfer[0].tx_buf = (unsigned long)buf; 
  xfer[0].len = 2	;					 
	
  /*	xfer[1].speed_hz=speed;
	xfer[1].bits_per_word=bits;
	xfer[1].rx_buf = (unsigned long) buf;
	xfer[1].len = 0;
  */

  status = ioctl(_fd, SPI_IOC_MESSAGE(1), &xfer);
  //	printf (" at 0x%x, set 0x%x\n",addr, value);
  //	getchar();
  if (status < 1)
    perror("can't send spi message");
}  

uint32_t lydaq::BMP280::myAnalogRead (uint32_t addr)
{

  struct spi_ioc_transfer xfer[2];
  unsigned char buf[32];
  uint32_t  status;

  memset(xfer, 0, sizeof xfer);
  memset(buf, 0, sizeof buf);

  buf[0] = (addr&0x7f) +0x80;// bit 7 = & for read

  xfer[0].speed_hz=_speed;
  xfer[0].bits_per_word=_bits;
  xfer[0].tx_buf = (unsigned long)buf; 
  xfer[0].len = 1;					 
	
  xfer[1].speed_hz=_speed;
  xfer[1].bits_per_word=_bits;
  xfer[1].rx_buf = (unsigned long) buf;
  xfer[1].len = 1;


  status = ioctl(_fd, SPI_IOC_MESSAGE(2), &xfer);
  if (status < 1)	perror("can't get  spi message");
  //	printf (" at 0x%x, get 0x%x\n",addr, buf[0]);
  //	getchar();
 
  return buf[0];
}

int lydaq::BMP280::GetCalibration()
{ 

  _dig_T1 = ( myAnalogRead(0x89)<<8) + myAnalogRead(0x88);
  _dig_T2 = ( myAnalogRead(0x8B)<<8) + myAnalogRead(0x8A);
  _dig_T3 = ( myAnalogRead(0x8D)<<8) + myAnalogRead(0x8C);

  _dig_P1 = ( myAnalogRead(0x8F)<<8) + myAnalogRead(0x8E);
  _dig_P2 = ( myAnalogRead(0x91)<<8) + myAnalogRead(0x90);
  _dig_P3 = ( myAnalogRead(0x93)<<8) + myAnalogRead(0x92);
  _dig_P4 = ( myAnalogRead(0x95)<<8) + myAnalogRead(0x94);
  _dig_P5 = ( myAnalogRead(0x97)<<8) + myAnalogRead(0x96);
  _dig_P6 = ( myAnalogRead(0x99)<<8) + myAnalogRead(0x98);
  _dig_P7 = ( myAnalogRead(0x9B)<<8) + myAnalogRead(0x9A);
  _dig_P8 = ( myAnalogRead(0x9D)<<8) + myAnalogRead(0x9C);
  _dig_P9 = ( myAnalogRead(0x9F)<<8) + myAnalogRead(0x9E);


  printf ("_dig_T1 = %x\n",_dig_T1);
  printf ("_dig_T2 = %x\n",_dig_T2);
  printf ("_dig_T3 = %x\n",_dig_T3);
  printf ("_dig_P1 = %x\n",_dig_P1);
  printf ("_dig_P2 = %x\n",_dig_P2);
  printf ("_dig_P3 = %x\n",_dig_P3);
  printf ("_dig_P4 = %x\n",_dig_P4);
  printf ("_dig_P5 = %x\n",_dig_P5);
  printf ("_dig_P6 = %x\n",_dig_P6);
  printf ("_dig_P7 = %x\n",_dig_P7);
  printf ("_dig_P8 = %x\n",_dig_P8);
  printf ("_dig_P9 = %x\n",_dig_P9);
  return 0;
}

int  lydaq::BMP280::TemperaturePressionRead(float *temperature, float *pression  )
{
  // corrections basees sur 
  //http://www.pibits.net/code/raspberry-pi-and-bmp280-sensor-example.php 

  uint32_t adc_T, adc_P;
  uint32_t temp_msb, temp_lsb, temp_xlsb;
  uint32_t pres_msb, pres_lsb, pres_xlsb;
  float var1, var2, p;

  temp_msb= myAnalogRead( 0xFA );
  temp_lsb= myAnalogRead( 0xFB );
  temp_xlsb= myAnalogRead( 0xFC );
  
  temp_xlsb=temp_xlsb >>4;
  
  adc_T = (temp_msb << 12) + (temp_lsb <<4 ) +(temp_xlsb >> 4);

  pres_msb= myAnalogRead( 0xF7 );
  pres_lsb= myAnalogRead( 0xF8 );
  pres_xlsb= myAnalogRead( 0xF9 );
  
  pres_xlsb=pres_xlsb >>4;
  
  adc_P = (pres_msb << 12) + (pres_lsb <<4 ) +(pres_xlsb >> 4);

  
  /*	var1 = ( ( (adc_T>>3) - (_dig_T1<<1)) * _dig_T2)>>11;
  
	var2 = ((((( adc_T>>4) -_dig_T1*((adc_T>>4)-_dig_T1)>>12)*(_dig_T3)>>14;
  */
  var1=(adc_T/16384.0-_dig_T1/1024.0)*_dig_T2;
  var2=((adc_T/131072.0-_dig_T1/8192.0) *(adc_T/131072.0-_dig_T1/8192.0))*_dig_T3;

  //printf ("var1= %f var2=%f\n", var1,var2);

  *temperature= (var1+var2)/5120.0;
  
  var1= (var1+var2)/2.0 - 64000.0;
  var2= var1*var1*_dig_P6/32768.0;
  var2= var2+var1*_dig_P5*2.0;
  var2= (var2/4.0)+ (_dig_P4*65536.0);
  var1= (_dig_P3*var1*var1/524288.0 + _dig_P2*var1)/524288.0;
  var1= (1.0 + var1 / 32768.0) * _dig_P1;
  p=1048576.0-adc_P;
  p= (p-var2/4096.0)*6250.0/var1;
  var1=_dig_P9*p*p/2147483648.0;
  var2=p*_dig_P8/32768.0;
  *pression = (p + (var1+var2 +_dig_P7)/16.0)/100.0;
  //  printf ("0x%x 0x%x 0x%x - %f C\n",temp_msb, temp_lsb, temp_xlsb, *temperature);

  // printf ("0x%x 0x%x 0x%x - %f hPa\n",pres_msb, pres_lsb, pres_xlsb, *pression);
  
  return 0;
  
}



// int main()
// {
// 	BMP280Setup();
	
// 	BMP280GetCalibration();		
//  	for (;;)
// 	{
// 		BMP280TemperaturePressionRead(&Temp,&Pres);
// 		 printf ("%f C - %f hPa\n",Temp, Pres);
// 		usleep(1000000);
// 	}	
	
// 	return 0;
// }
