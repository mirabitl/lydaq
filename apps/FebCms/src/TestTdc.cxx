#include "WiznetTest.hh"
using namespace lydaq;
int main()
{
  std::map<uint32_t,std::string> m=lydaq::WiznetMessageHandler::scanNetwork("192.168.10.");
  for (auto x:m)
    printf("%x %s \n",x.first,x.second.c_str());
  WiznetTest w("192.168.10.7",10001,10002);
  w.initialise();
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
