/************************************************************************/
/* ILC test beam daq							*/	
/* C. Combaret								*/
/* V 1.0								*/
/* first release : 30-01-2008						*/
/* revs : 								*/
/************************************************************************/
#include "FtdiCCCDriver.hh"
#include <iostream>
#include <sstream>
#include <sys/timeb.h>


#define MY_DEBUG
using namespace lydaq;

lydaq::FtdiCCCDriver::FtdiCCCDriver(char * deviceIdentifier )      : lydaq::FtdiUsbDriver(deviceIdentifier) 
{


}

lydaq::FtdiCCCDriver::~FtdiCCCDriver()     //throw (LocalHardwareException)

{
  std::cout<<"destructor called"<<std::endl;
  
 
}	



int32_t lydaq::FtdiCCCDriver:: CCCCommandDIFReset(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x00;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandBCIDReset(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x01;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandStartAcquisitionAuto(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x02;
  printf ("sdcc send start acq\n");
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandRamfullExt(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x03;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandTriggerExt(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x04;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	


int32_t lydaq::FtdiCCCDriver::CCCCommandStopAcquisition(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x05;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandDigitalReadout(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x06;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandTrigger(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x07;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandSpillOn(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0C;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandSpillOff(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0D;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandClearMemory(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x08;
  printf (" Not implemented anymore\n");
  return -2;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandStartSingleAcquisition(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x09;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandPulseLemo(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0A;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

int32_t lydaq::FtdiCCCDriver::CCCCommandRazChannel(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0B;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	
/*
  int32_t lydaq::FtdiCCCDriver::CCCCommandNoCommand(void)    //throw (LocalHardwareException)
  {
  uint32_t taddress=0x0E;
	
  try	{		CCCCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
  {
  throw (e);
  return -2;
  }
  return 0;
  }	
*/
int32_t lydaq::FtdiCCCDriver::CCCCommandResetCCC(void)    //throw (LocalHardwareException)
{
  uint32_t taddress=0x0F;
	
  try	{		UsbCommandWrite	(taddress);	}
  catch (LocalHardwareException& e)
    {
      throw (e);
      return -2;
    }
  return 0;
}	

