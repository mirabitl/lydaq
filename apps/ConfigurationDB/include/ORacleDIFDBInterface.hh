#ifndef _OracleDIFDBManager_H
#define _OracleDIFDBManager_H
#include "DIFDBManager.h"
#include <ILCConfDB.h>

class OracleDIFDBManager : public DIFDBManager
{
public:
 OracleDIFDBManager() : DIFDBManager(){;}
  virtual ~OracleDIFDBManager(){;}
  OracleDIFDBManager(std::string sdifs,std::string setup,std::string xmlfile="NONE");
  virtual void initialize();
  virtual uint32_t LoadDIFHardrocV2Parameters(uint32_t difid,SingleHardrocV2ConfigurationFrame* v);
  virtual uint32_t LoadDIFMicrorocParameters(uint32_t difid, SingleHardrocV2ConfigurationFrame* ConfigMR);
  
  virtual void LoadDIFDefaultParameters(uint32_t difid, UsbDIFSettings* v);
  virtual void storeHardRoc2Configuration(Asic* itMR,unsigned char* ConfigMR);
  virtual void storeMicroRocConfiguration(Asic* itMR,unsigned char* ConfigMR);
  virtual uint32_t LoadAsicParameters();

private:
  Setup* theOracleSetup_;
  std::string theOracleSetupName_;
  std::string theXMLFile_;
};

#endif
