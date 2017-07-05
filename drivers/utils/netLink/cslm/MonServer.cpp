
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "NMServer.h"
using namespace std;
#include <sstream>
class MonServer: public NMServer
{
public:
  MonServer(std::string host,uint32_t port) : NMServer(host,port)
  {
    
    this->registerCommand("START",boost::bind(&MonServer::commandHandler,this,_1));
    this->registerCommand("STOP",boost::bind(&MonServer::commandHandler,this,_1));
    this->registerService("UNESSAI");
    this->start();
    this->startServices();
    nessai_=0;
    running_=false;
  }
  NetMessage* commandHandler(NetMessage* m)
  {
    std::string start="START";
    std::string stop="STOP";
    
    printf(" J'ai recu %s COMMAND %s %s  \n",m->getName().c_str(),start.c_str(),stop.c_str());
    if (m->getName().compare(start)==0)
      {
	running_=true;
	NetMessage* mrep = new NetMessage("START",NetMessage::COMMAND_ACKNOWLEDGE,4);
	return mrep;
      }
    if (m->getName().compare(stop)==0)
      {
	running_=false;
      }

    return NULL;
  }
  void startServices()
  {
    m_Thread_s = boost::thread(&MonServer::services, this);  
  }
  void joinServices()
  {
    m_Thread_s.join();  
  }
  void services()
  {
    while (true)
      {
	usleep((uint32_t) 100);
	if (!running_) continue;
	DEBUG(" J'update UNESSAI \n");
	uint32_t len=rand()%(0x10000-0x100) +0x100;
	NetMessage m("UNESSAI",NetMessage::SERVICE,len);
	uint32_t* ibuf=(uint32_t*) m.getPayload();
	ibuf[0]=nessai_++;
	this->updateService("UNESSAI",&m);
	
      }
  }
private:
  bool running_;
  boost::thread    m_Thread_s;
  uint32_t nessai_;
};

int main(int argc, char **argv) {
  MonServer s("lyopc252",atoi(argv[1]));
  //s.start();
	// NL::init();

	// cout << "\nStarting Server...";
	// cout.flush();

	// NL::Socket socketServer(SERVER_PORT);

	// NL::SocketGroup group;

	// OnAccept onAccept;
	// OnRead onRead;
	// OnDisconnect onDisconnect;

	// group.setCmdOnAccept(&onAccept);
	// group.setCmdOnRead(&onRead);
	// group.setCmdOnDisconnect(&onDisconnect);

	// group.add(&socketServer);

	// while(true) {

	// 	if(!group.listen(2000))
	// 		cout << "\nNo msg recieved during the last 2 seconds";
	// }
  while (true)
    { sleep((unsigned int) 100);}
}
