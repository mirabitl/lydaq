#include "MDCCReadout.hh"
using namespace lydaq;

lydaq::MDCCReadout::MDCCReadout(std::string name,uint32_t productid) : _name(name),_productid(productid),_driver(NULL)
{
}
lydaq::MDCCReadout::~MDCCReadout()
{
  if (_driver!=NULL)
    this->close();
}

void lydaq::MDCCReadout::open()
{
  try 
    {
      std::cout<<_name<<" "<<_productid<<std::endl;
      _driver= new FtdiUsbDriver((char*) _name.c_str(),_productid);
    } 
  catch(LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logMDCC," Cannot open "<<_name<<" err="<<e.message());
      return;
    }
  try
    {
      _driver->UsbRegisterRead(0x1,&_version);
      _driver->UsbRegisterRead(0x100,&_id);
    }
  catch(LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logMDCC," Cannot read version and ID ");
      return;
    }
  LOG4CXX_INFO(_logMDCC," MDCC "<<_name<<" ID="<<_id<<" version="<<_version);

}
void lydaq::MDCCReadout::close()
{
  try 
    {
      if (_driver!=NULL)
	delete _driver;
    } 
  catch(LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logMDCC," Cannot delete "<<_name<<" err="<<e.message());
      return;
    }

}
uint32_t lydaq::MDCCReadout::version(){return this->readRegister(0x100);}
uint32_t lydaq::MDCCReadout::id(){return this->readRegister(0x1);}
uint32_t lydaq::MDCCReadout::spillCount(){return this->readRegister(0x3);}
uint32_t lydaq::MDCCReadout::busy1Count(){return this->readRegister(0x5);}
uint32_t lydaq::MDCCReadout::busy2Count(){return this->readRegister(0x6);}
uint32_t lydaq::MDCCReadout::busy3Count(){return this->readRegister(0x7);}
uint32_t lydaq::MDCCReadout::spillOn(){return this->readRegister(0x8);}
uint32_t lydaq::MDCCReadout::spillOff(){return this->readRegister(0x9);}
void lydaq::MDCCReadout::setSpillOn(uint32_t nc){this->writeRegister(0x8,nc);}
void lydaq::MDCCReadout::setSpillOff(uint32_t nc){this->writeRegister(0x9,nc);}
uint32_t lydaq::MDCCReadout::beam(){return this->readRegister(0xa);}
void lydaq::MDCCReadout::setBeam(uint32_t nc){this->writeRegister(0xa,nc);}



uint32_t lydaq::MDCCReadout::mask(){return this->readRegister(0x2);}
uint32_t lydaq::MDCCReadout::ecalmask(){return this->readRegister(0x10);}
void lydaq::MDCCReadout::maskTrigger(){this->writeRegister(0x2,0x1);}
void lydaq::MDCCReadout::unmaskTrigger(){this->writeRegister(0x2,0x0);}
void lydaq::MDCCReadout::maskEcal(){this->writeRegister(0x10,0x1);}
void lydaq::MDCCReadout::unmaskEcal(){this->writeRegister(0x10,0x0);}
void lydaq::MDCCReadout::calibOn(){this->writeRegister(0xB,0x2);}
void lydaq::MDCCReadout::calibOff(){this->writeRegister(0xB,0x0);}
void lydaq::MDCCReadout::reloadCalibCount(){
  
  this->writeRegister(0xB,0x4);
  usleep(2);
  this->writeRegister(0xB,0x0);
  this->calibOn();
}
uint32_t lydaq::MDCCReadout::calibCount(){return this->readRegister(0xD);}
void lydaq::MDCCReadout::setCalibCount(uint32_t nc){this->writeRegister(0xD,nc);}

void lydaq::MDCCReadout::resetCounter(){this->writeRegister(0x4,0x1);this->writeRegister(0x4,0x0);}
uint32_t lydaq::MDCCReadout::readRegister(uint32_t adr)
{
  if (_driver==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,"Cannot read no driver created ");
       return 0xbad;
    }
  uint32_t rc;
  try
    {
      _driver->UsbRegisterRead(adr,&rc);
    }
  catch(LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logMDCC," Cannot read at adr "<<adr);
      return 0xbad;
    }
  return rc;
}

void lydaq::MDCCReadout::writeRegister(uint32_t adr,uint32_t val)
{
  if (_driver==NULL)
    {
       LOG4CXX_ERROR(_logMDCC,"Cannot write no driver created ");
       return;
    }
  try
    {
      _driver->UsbRegisterWrite(adr,val);
    }
  catch(LocalHardwareException& e)
    {
      LOG4CXX_ERROR(_logMDCC," Cannot write at adr "<<adr);
      return;
    }
}
