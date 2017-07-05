#ifndef _lydaq_WIENERSNMP_HH
#define _lydaq_WIENERSNMP_HH
#include <stdio.h>

#include <iostream>
#include <sstream>
#include <fstream>
#include <json/writer.h> 
#include "json/json.h"
#include <iostream>
#include <fstream>

#include <boost/algorithm/string.hpp>
using namespace  std;

namespace lydaq
{
  class WienerSnmp
  {
  public:
    std::string exec(const char* cmd);
    WienerSnmp(std::string ipa);
    std::string getSysMainSwitch();
    std::string setOutputVoltage(uint32_t module,uint32_t voie,float tension);
    float getOutputVoltage(uint32_t module,uint32_t voie);
    std::string setOutputVoltageRiseRate(uint32_t module,uint32_t voie,float val);
    float getOutputVoltageRiseRate(uint32_t module,uint32_t voie);
    std::string setOutputCurrentLimit(uint32_t module,uint32_t voie,float cur );
    float getOutputCurrentLimit(uint32_t module,uint32_t voie);
    float getOutputMeasurementSenseVoltage(uint32_t module,uint32_t voie);
    float getOutputMeasurementCurrent(uint32_t module,uint32_t voie);
    std::string setOutputSwitch(uint32_t module,uint32_t voie,uint32_t val );
    std::string getOutputSwitch(uint32_t module,uint32_t voie);
    std::string getOutputStatus(uint32_t module,uint32_t voie);
  private:
    std::string _ip;
  };
};
#endif
