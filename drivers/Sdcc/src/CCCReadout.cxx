#include "CCCReadout.hh"
#include <pthread.h>
//#include "toolbox/string.h"

using namespace lydaq;
lydaq::CCCReadout::CCCReadout (std::string name) :theName_(name)
{
  theDriver_=NULL;
  //std::cout<<" On sort"<<std::hex<< this<<std::dec<<std::endl;

}

int lydaq::CCCReadout::open() //throw( LocalHardwareException ) 
{
  if (theDriver_==NULL)
  {
    try 
		{
			std::cout<<"Opening "<<theName_<<std::endl;
			std::cout<<"Opening 1yy "<<theName_<<std::endl;



#ifndef USE_FTDI
        theDriver_=new USBDRIVER((char*) theName_.c_str());
#else
        theDriver_=new USBDRIVER((char*) theName_.c_str());
        theDriver_->checkReadWrite(0x1234,100);
        theDriver_->checkReadWrite(0x1234,100);
#endif



	//theDriver_=new UsbCCCDriver((char*) theName_.c_str());
			std::cout<<"Opening 2yy "<<theName_<<std::endl;
  		  printf ("%p\n", theDriver_);

	  }
    catch (LocalHardwareException& e)
		{
			std::cout<<"fail in Opening "<<theName_<<std::endl;
			LOG4CXX_FATAL(_logCCC,"fail openning "<<theName_);
	  	throw ;
		}
  }
  // std::cout<<" On sort"<<std::hex<<(int) this<<std::dec<<std::endl;
  return 0;
}

int lydaq::CCCReadout::close() //throw( LocalHardwareException )
{
  try 
  {
    delete theDriver_;
    theDriver_=NULL;
  }
  catch (LocalHardwareException& e) 
  {
    LOG4CXX_FATAL(_logCCC,"fail closing "<<theName_);
    throw;
  }
  return 0;
}
void lydaq::CCCReadout::DoSendPauseTrigger()
{
  try	{	theDriver_->UsbCommandWrite(0x10);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send Pause");	}	
	return;
}
void lydaq::CCCReadout::DoSendResumeTrigger()
{
  try	{	theDriver_->UsbCommandWrite(0x11);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send Resume");	}	
	return;
}


void lydaq::CCCReadout::DoSendDIFReset()
{
  try	{	theDriver_->CCCCommandDIFReset();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send dif reset");	}	
	return;
}

void lydaq::CCCReadout::DoSendBCIDReset()
{
  try	{	theDriver_->CCCCommandBCIDReset();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send bcid reset");	}	
	return;
}

void lydaq::CCCReadout::DoSendStartAcquisitionAuto()
{
  try	{	theDriver_->CCCCommandStartAcquisitionAuto();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send start acquisition auto");	}	
	return;
}

void lydaq::CCCReadout::DoSendRamfullExt()
{
  try	{	theDriver_->CCCCommandRamfullExt();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send ramfull ext");	}	
	return;
}
void lydaq::CCCReadout::DoSendTrigExt()
{
  try	{	theDriver_->CCCCommandTriggerExt();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send trig ext");	}	
	return;
}
void lydaq::CCCReadout::DoSendStopAcquisition()
{
  try	{	theDriver_->CCCCommandStopAcquisition();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send stop acquisition");	}	
	return;
}
void lydaq::CCCReadout::DoSendDigitalReadout()
{
  try	{	theDriver_->CCCCommandDigitalReadout();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send digital readout");	}	
	return;
}
void lydaq::CCCReadout::DoSendTrigger()
{
  try	{	theDriver_->CCCCommandTrigger();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send analog readout");	}	
	return;
}
void lydaq::CCCReadout::DoSendClearMemory()
{
  try	{	theDriver_->CCCCommandClearMemory();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send clear memory");	}	
	return;
}
void lydaq::CCCReadout::DoSendStartSingleAcquisition()
{
  try	{	theDriver_->CCCCommandStartSingleAcquisition();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send start single acquisition");	}	
	return;
}
void lydaq::CCCReadout::DoSendPulseLemo()
{
  try	{	theDriver_->CCCCommandPulseLemo();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send lemo pulse");	}	
	return;
}
void lydaq::CCCReadout::DoSendRazChannel()
{
  try	{	theDriver_->CCCCommandRazChannel();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send raz channel");	}	
	return;
}
void lydaq::CCCReadout::DoSendCCCReset()
{
printf ("1*\n");
  try	{	
  
  printf ("%p\n", theDriver_);
  theDriver_->CCCCommandResetCCC();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send ccc reset");	}	
printf ("2*\n");
	return;
}

void lydaq::CCCReadout::DoSendSpillOn()
{
  try	{	theDriver_->CCCCommandSpillOn();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send ccc spill on");	}	
	return;
}
void lydaq::CCCReadout::DoSendSpillOff()
{
  try	{	theDriver_->CCCCommandSpillOn();	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send ccc spill on");	}	
	return;
}

void lydaq::CCCReadout::DoWriteRegister(uint32_t addr,uint32_t data)
{
  try	{	theDriver_->UsbRegisterWrite(addr,data);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send ccc spill on");	}	
	return;

}
uint32_t lydaq::CCCReadout::DoReadRegister(uint32_t addr)
{
  uint32_t data=0;
  try	{	theDriver_->UsbRegisterRead(addr,&data);	}
  catch (LocalHardwareException& e)  {  LOG4CXX_ERROR(_logCCC, "CCCReadout : Unable to send ccc spill on");	}	
  return data;

}
