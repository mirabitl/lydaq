#include "MDCCHandler.hh"

using namespace lydaq;
lydaq::MDCCHandler::MDCCHandler(std::string name,uint32_t productid) : _name(name),_productid(productid),_driver(NULL)
{
}
lydaq::MDCCHandler::~MDCCHandler()
{
  if (_driver!=NULL)
    this->close();
}

void lydaq::MDCCHandler::open()
{
  try 
    {
      std::cout<<_name<<" "<<_productid<<std::endl;
      _driver= new lydaq::FtdiUsbDriver((char*) _name.c_str(),_productid);
    } 
  catch(LocalHardwareException& e)
    {
      LOG4CXX_FATAL(_logCCC," Cannot open "<<_name<<" err="<<e.message());
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
void lydaq::MDCCHandler::close()
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

/*
when x"0001" => USB_data_out <= ID_register;
when x"0002" => USB_data_out <= software_veto_register;
when x"0003" => USB_data_out <= spillNb_register;
when x"0004" => USB_data_out <= Control_register;
when x"0005" => USB_data_out <= spillon_register;
when x"0006" => USB_data_out <= spilloff_register;
when x"0007" => USB_data_out <= beam_register;
when x"0008" => USB_data_out <= Calib_register;
when x"0009" => USB_data_out <= Calib_Counter_register;
when x"000A" => USB_data_out <= nb_windows_register;
when x"000B" => USB_data_out <= software_ECALveto_register;
when x"000C" => USB_data_out <= Rstdet_register;
 0xD   bit 0 => start/end of spill used
       bit 1 => trigext used
 default 0

 0XE delay trigext
 OXF length busy trigext 

when x"0010" => USB_data_out <= busy0Nb_register;
when x"0011" => USB_data_out <= busy1Nb_register;
when x"0012" => USB_data_out <= busy2Nb_register;
when x"0013" => USB_data_out <= busy3Nb_register;
when x"0014" => USB_data_out <= busy4Nb_register;
when x"0015" => USB_data_out <= busy5Nb_register;
when x"0016" => USB_data_out <= busy6Nb_register;
when x"0017" => USB_data_out <= busy7Nb_register;
when x"0018" => USB_data_out <= busy8Nb_register;
when x"0019" => USB_data_out <= busy9Nb_register;
when x"001A" => USB_data_out <= busy10Nb_register;
when x"001B" => USB_data_out <= busy11Nb_register;
when x"001C" => USB_data_out <= busy12Nb_register;
when x"001D" => USB_data_out <= busy13Nb_register;
when x"001E" => USB_data_out <= busy14Nb_register;
when x"001F" => USB_data_out <= busy15Nb_register;

when x"0020" => USB_data_out <= spare0Nb_register;
when x"0021" => USB_data_out <= spare1Nb_register;
 
when x"0100" => USB_data_out <= version;
*/
uint32_t lydaq::MDCCHandler::version(){return this->readRegister(0x100);}
uint32_t lydaq::MDCCHandler::id(){return this->readRegister(0x1);}
uint32_t lydaq::MDCCHandler::mask(){return this->readRegister(0x2);}
void lydaq::MDCCHandler::maskTrigger(){this->writeRegister(0x2,0x1);}
void lydaq::MDCCHandler::unmaskTrigger(){this->writeRegister(0x2,0x0);}
uint32_t lydaq::MDCCHandler::spillCount(){return this->readRegister(0x3);}
void lydaq::MDCCHandler::resetCounter(){this->writeRegister(0x4,0x1);this->writeRegister(0x4,0x0);}
uint32_t lydaq::MDCCHandler::spillOn(){return this->readRegister(0x5);}
uint32_t lydaq::MDCCHandler::spillOff(){return this->readRegister(0x6);}
void lydaq::MDCCHandler::setSpillOn(uint32_t nc){this->writeRegister(0x5,nc);}
void lydaq::MDCCHandler::setSpillOff(uint32_t nc){this->writeRegister(0x6,nc);}
uint32_t lydaq::MDCCHandler::beam(){return this->readRegister(0x7);}
void lydaq::MDCCHandler::setBeam(uint32_t nc){this->writeRegister(0x7,nc);}
void lydaq::MDCCHandler::calibOn(){this->writeRegister(0x8,0x2);}
void lydaq::MDCCHandler::calibOff(){this->writeRegister(0x8,0x0);}
uint32_t lydaq::MDCCHandler::calibCount(){return this->readRegister(0xa);}
void lydaq::MDCCHandler::setCalibCount(uint32_t nc){this->writeRegister(0xa,nc);}

uint32_t lydaq::MDCCHandler::hardReset(){return this->readRegister(0xc);}
void lydaq::MDCCHandler::setHardReset(uint32_t nc){this->writeRegister(0xc,nc);}

void lydaq::MDCCHandler::setSpillRegister(uint32_t nc){this->writeRegister(0xD,nc);}
uint32_t lydaq::MDCCHandler::spillRegister(){this->readRegister(0xD);}
void lydaq::MDCCHandler::useSPSSpill(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|1);
  else
    this->setSpillRegister(reg&~1);
}
void lydaq::MDCCHandler::useTrigExt(bool t)
{
  uint32_t reg=this->spillRegister();
  if (t)
    this->setSpillRegister(reg|2);
  else
    this->setSpillRegister(reg&~2);
}

void lydaq::MDCCHandler::setTriggerDelay(uint32_t nc){this->writeRegister(0xE,nc);}
uint32_t lydaq::MDCCHandler::triggerDelay(){this->readRegister(0xE);}
void lydaq::MDCCHandler::setTriggerBusy(uint32_t nc){this->writeRegister(0xF,nc);}
uint32_t lydaq::MDCCHandler::triggerBusy(){this->readRegister(0xF);}


void lydaq::MDCCHandler::reloadCalibCount(){
  
  this->writeRegister(0x8,0x4);
  usleep(2);
  this->writeRegister(0x8,0x0);
  this->calibOn();
}




uint32_t lydaq::MDCCHandler::ecalmask(){return this->readRegister(0xB);}
void lydaq::MDCCHandler::maskEcal(){this->writeRegister(0xB,0x1);}
void lydaq::MDCCHandler::unmaskEcal(){this->writeRegister(0xB,0x0);}
void lydaq::MDCCHandler::resetTDC(uint8_t b){this->writeRegister(0xC,b);}
uint32_t lydaq::MDCCHandler::busyCount(uint8_t b){return this->readRegister(0x10+(b&0xF));}



uint32_t lydaq::MDCCHandler::readRegister(uint32_t adr)
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

void lydaq::MDCCHandler::writeRegister(uint32_t adr,uint32_t val)
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
