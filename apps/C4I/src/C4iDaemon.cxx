#include "C4iManager.hh"
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
DOMConfigurator::configure("/etc/Log4cxxConfig.xml");
//_logger->setLevel(log4cxx::Level::getInfo());
LOG4CXX_INFO (_logLdaq, "this is a info message, after parsing configuration file");
  std::stringstream s0;
  uint32_t instance=0;
  char* wp=getenv("INSTANCE");
  if (wp!=NULL)      instance=atoi(wp);
  s0<<"C4I-"<<instance;
  std::cout<<"Starting  "<<s0.str()<<std::endl;


  lydaq::C4iManager* s=new lydaq::C4iManager(s0.str());

  while (true)
    sleep((unsigned int) 3);
}
