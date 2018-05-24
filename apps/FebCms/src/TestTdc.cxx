#include "WiznetTest.hh"
using namespace lydaq;
int main()
{
  printf("parsing the config file \n");
DOMConfigurator::configure("/etc/Log4cxxConfig.xml");
//_logger->setLevel(log4cxx::Level::getInfo());
LOG4CXX_INFO (_logLdaq, "this is a info message, after parsing configuration file")
  std::map<uint32_t,std::string> m=lydaq::WiznetMessageHandler::scanNetwork("192.168.10.");
  for (auto x:m)
    printf("%x %s \n",x.first,x.second.c_str());
  WiznetTest w("192.168.10.7",10001,10002);
  w.initialise();


   while (true)
    sleep((unsigned int) 3);
   
  printf("Starting (hit CR)\n");
  getchar();

  w.start();
  printf("Stoping (hit CR)\n");
  getchar();
  
  w.stop();
  printf("Stop Done (hit CR)\n");

  getchar();
  printf("Leaving (hit CR)\n");
}
