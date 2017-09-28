#include "TdcConfigAccess.hh"
#include <unistd.h>
#include <stdint.h>
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;

int main()
{
printf("parsing the config file \n");
//DOMConfigurator::configure("/etc/Log4cxxConfig.xml");
//_logger->setLevel(log4cxx::Level::getInfo());
//LOG4CXX_INFO (_logLdaq, "this is a info message, after parsing configuration file")

 lydaq::TdcConfigAccess a;
 //a.parseJsonFile("/home/mirabito/cernbox/PR2_default.json");
 //a.parseJsonUrl("http://lyosdhcal9/daq/petiroc//PR2_default_3DIF.json");
 a.parseDb("TDC_IPNL_1","DB");
 a.prepareSlowControl("192.168.10.13");
 printf("Bytes %d \n",a.slcBytes());
 getchar();
 a.prepareSlowControl("192.168.10.15");
 printf("Bytes %d \n",a.slcBytes());
 getchar();
 a.dumpMap();
}
