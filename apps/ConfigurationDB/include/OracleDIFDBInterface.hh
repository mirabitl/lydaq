#ifndef _OracleDIFDBInterface_H
#define _OracleDIFDBInterface_H
#include "DIFDBInterface.hh"
#include <ILCConfDB.h>

namespace lydaq
{
  class OracleDIFDBInterface : public lydaq::DIFDBInterface
  {
  public:
    OracleDIFDBInterface() : DIFDBInterface(){;}
    virtual ~OracleDIFDBInterface(){;}
    OracleDIFDBInterface(State* s);
    virtual void initialize();
    virtual uint32_t LoadDIFHardrocV2Parameters(uint32_t difid,SingleHardrocV2ConfigurationFrame* v);
    virtual uint32_t LoadDIFMicrorocParameters(uint32_t difid, SingleHardrocV2ConfigurationFrame* ConfigMR);
  
    virtual void LoadDIFDefaultParameters(uint32_t difid, UsbDIFSettings* v);
    virtual void storeHardRoc2Configuration(Asic* itMR,unsigned char* ConfigMR);
    virtual void storeMicroRocConfiguration(Asic* itMR,unsigned char* ConfigMR);
    virtual uint32_t LoadAsicParameters();

  private:
    State* _state;
  };
};
#endif
