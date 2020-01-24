#include "hih8000.hh"


/*
hih8000::hih8000(void)
{

 printf("\t on rentre1)\n");
  hih8000Setup () ;
}
 */
using namespace lydaq;
hih8000::hih8000()
{
  Setup () ;
  _temperature[0]=0;
  _temperature[1]=0;
  _humidity[0]=0;
  _humidity[1]=0;
}
 
hih8000::~hih8000()
{
}



int hih8000::Setup (void)
{
  int temp1, temp2;

  if (wiringPiSetup ()<<0) 
  {
    LOG4CXX_INFO(_logLdaq," unable to open  GPIO");
    return -1 ;
  } 
  pinMode (4, OUTPUT) ;
  digitalWrite (4, 1) ; 

  usleep(DELAY); // 2 secs before calling i2c setup required	
  if ((_fd=wiringPiI2CSetup (HUM_ADDR))<0) 
  {
    LOG4CXX_INFO(_logLdaq," unable to open I2C at "<<DEV_ADDR<< " _fd="<<_fd );//DEV_ADDR);
    return -1 ;
  } 
  else 
    printf ("fd=%d\n",_fd);

  return 0 ;
}


int hih8000::Read()
{
  unsigned char buf[4];                 /* Buffer for data read/written on the i2c bus */

  digitalWrite (4, 1) ;  // capteur sur la partir etroite de la carte
  //  if ((i2c_smbus_write_quick(_fd, 0)) != 0)
  // {
  if (wiringPiI2CWrite (_fd, 0)!=0)
      {
    printf("Error writing bit to i2c slave 1 \n");
    // exit(1);
  }
  usleep(100000);
  if (read(_fd, buf, 4) < 0)
  {
    printf("Unable to read from slave\n");
//    exit(1);
  }
  else
  {
    int reading_hum = (buf[0] << 8) + buf[1];
    double humidity = reading_hum / 16382.0 * 100.0;
    //printf("Humidity: %f\n", humidity);

    int reading_temp = (buf[2] << 6) + (buf[3] >> 2);
    double temperature = reading_temp / 16382.0 * 165.0 - 40;
    printf("Temperature 1: %f %f\n", temperature,humidity);
    _humidity[1] =  (humidity);
    _temperature[1] = (temperature) + 273.15;
  }





  digitalWrite (4, 0) ;  // capteur sur la partir large de la carte
  //if ((i2c_smbus_write_quick(_fd, 0)) != 0)
  //{
   if (wiringPiI2CWrite (_fd, 0)!=0)
      {   
    printf("Error writing bit to i2c slave\n");
    //exit(1);
  }
  usleep(100000);
  if (read(_fd, buf, 4) < 0)
  {
    printf("Unable to read from slave\n");
//    exit(1);
  }
  else
  {
    int reading_hum = (buf[0] << 8) + buf[1];
    double humidity = reading_hum / 16382.0 * 100.0;
    //printf("Humidity: %f\n", humidity);

    int reading_temp = (buf[2] << 6) + (buf[3] >> 2);
    double temperature = reading_temp / 16382.0 * 165.0 - 40;
    //printf("Temperature: %f\n", temperature);
    printf("Temperature 0: %f %f\n", temperature,humidity);
    _humidity[0] =  humidity;
    _temperature[0] =  temperature + 273.15;
  }

  return 0;
}

