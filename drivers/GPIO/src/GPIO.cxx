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
#include "GPIO.hh"
using namespace lydaq;
lydaq::GPIO::GPIO()
{
  if (geteuid()==0)  seteuid(getuid());

  // Enable GPIO pins
	
  if (-1 == GPIOExport(PVME) || -1 == GPIOExport(PDIF))
    perror("GPIOExport error");
 
  // Set GPIO directions
  if (-1 == GPIODirection(PVME, OUT) || -1 == GPIODirection(PDIF, OUT))
    perror("GPIODirection error");
}
 
lydaq::GPIO:: ~GPIO()
{
  freeGPIO();
}
	
int32_t lydaq::GPIO::freeGPIO ()
{	
  if (-1 == GPIOUnexport(PDIF) || -1 == GPIOUnexport(PVME))
    perror("Failed to free!\n");
}

 int32_t lydaq::GPIO::GPIOExport(int32_t pin)
{
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int32_t fd;
  char tcommand[80];

  strcpy(tcommand,"sudo chmod 777 -R  /sys/class/gpio/export");
  system(tcommand);
	
  fd = open("/sys/class/gpio/export", O_WRONLY);
  if (-1 == fd) 
    perror("Failed to open export for writing!\n");
 
  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
}
	
int32_t lydaq::GPIO::GPIOUnexport(int32_t pin)
{
  char buffer[BUFFER_MAX];
  ssize_t bytes_written;
  int32_t fd;
  char tcommand[80];
  strcpy(tcommand,"sudo chmod 777 -R  /sys/class/gpio/unexport");
  system(tcommand);
 
  fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if (-1 == fd) 
    perror("Failed to open unexport for writing!\n");
	 
  bytes_written = snprintf(buffer, BUFFER_MAX, "%d", pin);
  write(fd, buffer, bytes_written);
  close(fd);
  return(0);
}
	
int32_t lydaq::GPIO::GPIODirection(int32_t pin, int32_t dir)
{
  static const char s_directions_str[]  = "in\0out";
 
  char path[DIRECTION_MAX];
  int32_t fd;
   
  char tcommand[80];
  sprintf(tcommand,"sudo chmod 777 -R  /sys/class/gpio/gpio%d/",pin);
  system(tcommand);

  snprintf(path, DIRECTION_MAX, "/sys/class/gpio/gpio%d/direction", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) 
    perror( "Failed to open gpio direction for writing \n");
	 
  if (-1 == write(fd, &s_directions_str[IN == dir ? 0 : 3], IN == dir ? 2 : 3)) 
    perror( "Failed to set direction!\n");
 
  close(fd);
} 

int32_t lydaq::GPIO::GPIORead(int32_t pin)
{
  char path[VALUE_MAX];
  char value_str[3];
  int32_t fd;
 
  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_RDONLY);
  if (-1 == fd) 
    perror( "Failed to open gpio value for reading!\n");

  if (-1 == read(fd, value_str, 3)) 
    perror( "Failed to read value!\n");
  close(fd);
  return (atoi(value_str));
}
 
int32_t lydaq::GPIO::GPIOWrite(int32_t pin, int32_t value)
{
  const char s_values_str[] = "01";
 
  char path[VALUE_MAX];
  int32_t fd;
 		
  printf ("writing %d %d...\n",pin,value);
  snprintf(path, VALUE_MAX, "/sys/class/gpio/gpio%d/value", pin);
  fd = open(path, O_WRONLY);
  if (-1 == fd) 
    perror("Failed to open gpio value for writing!\n");
 	
  if (1 != write(fd, &s_values_str[LOW == value ? 0 : 1], 1)) 
    perror ("Failed to write value!\n");
 		
  close(fd);
}
	
void lydaq::GPIO::DIFON()
{
  DIFPower=1;
  if (-1 == GPIOWrite(PDIF, DIFPower))
    printf ("unable to set VME Power\n");
}
 
void  lydaq::GPIO::DIFOFF()
{
  DIFPower=0;
  if (-1 == GPIOWrite(PDIF, DIFPower))
    printf ("unable to set VME Power\n");

}
void  lydaq::GPIO::VMEON()
{
  VMEPower=1;
  if (-1 == GPIOWrite(PVME, VMEPower))
    printf ("unable to set VME Power\n");
}
 
void lydaq::GPIO::VMEOFF()
{
  VMEPower=0;
  if (-1 == GPIOWrite(PVME, VMEPower))
    printf ("unable to set VME Power\n");
}
 
void  lydaq::GPIO::INFO()
{
  DIFPower= GPIORead(PDIF);
  VMEPower= GPIORead(PVME);
}
 
int32_t  lydaq::GPIO::getDIFPower(void)
{
  return DIFPower;
}	

int32_t  lydaq::GPIO::getVMEPower(void)
{
  return VMEPower;
}	

