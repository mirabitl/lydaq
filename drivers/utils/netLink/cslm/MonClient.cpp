
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "NMClient.h"
using namespace std;
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <sstream>

class MonClient: public NMClient
{
public:
  MonClient(std::string host,uint32_t port) : NMClient(host,port)
  {
   
    //this->start();
    nessai_=0;
    nbytes_=0;
  }
  
  NetMessage* serviceHandler(NetMessage* m)
  {
    nessai_++;
    nbytes_+=m->getPayloadSize()*1.;
    if (nessai_%1000 ==0)
{
      uint32_t* ibuf=(uint32_t*) m->getPayload();
      printf(" J'ai recu un service %s %d %d-%d %f \n",m->getName().c_str(),m->getPayloadSize(),nessai_,ibuf[0],nbytes_/1024./1024.);
}
    return NULL;
  }
  void sendSecureCommand(std::string s,NetMessage* m)
  {
    // printf("LOCKING %s\n",s.c_str());
    bsem_.lock();
    this->sendCommand(s,m);
    // printf("LOCKING %s\n",s.c_str());
    bsem_.lock();
    bsem_.unlock();

  }

  NetMessage* answerHandler(NetMessage* m)
  {
    printf("I got answer for command %s \n",m->getName().c_str());
    //sleep((unsigned int) 5);
    bsem_.unlock();
    //global_stream_lock.unlock();
    return NULL;
  }

private:
  uint32_t nessai_;
  double nbytes_;
  std::string prefix_;
  boost::interprocess::interprocess_mutex bsem_;
};

int main(int argc, char **argv) {
  uint32_t nport=atoi(argv[1]);
  std::vector<MonClient*> clients;
  for (uint32_t i=0;i<nport;i++)
    {
      MonClient* s = new MonClient("lyoilcrpi02",5000+i);
      clients.push_back(s);
    }


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
  //  NetMessage ms("SUBSCRIBE",NetMessage::COMMAND,50);
  //std::string ser("UNESSAI");
  //ms.setPayload((unsigned char*)ser.c_str(),ser.size());
  //s.sendCommand("SUBSCRIBE",&ms);
  //s.registerHandler("UNESSAI",boost::bind(&MonClient::serviceHandler,&s,_1));
  for (std::vector<MonClient*>::iterator ic=clients.begin();ic!=clients.end();ic++)
    {
      if ((*ic)->isDisconnected()) continue;
      (*ic)->subscribeService("UNESSAI",boost::bind(&MonClient::serviceHandler,(*ic),_1));
      (*ic)->registerHandler("START",boost::bind(&MonClient::answerHandler,(*ic),_1));
      (*ic)->registerHandler("STOP",boost::bind(&MonClient::answerHandler,(*ic),_1));
      (*ic)->start();
    }
  bool disc=false;
  while (!disc) 
    {
      for (std::vector<MonClient*>::iterator ic=clients.begin();ic!=clients.end();ic++)
	{
	  if ((*ic)->isDisconnected()) continue;
	  NetMessage m("START",NetMessage::COMMAND,4);
	  (*ic)->sendCommand("START",&m);
	}
      sleep((unsigned int) 5);
      for (std::vector<MonClient*>::iterator ic=clients.begin();ic!=clients.end();ic++)
	{
	  if ((*ic)->isDisconnected()) continue;
	  NetMessage m("STOP",NetMessage::COMMAND,4);
	  (*ic)->sendCommand("STOP",&m);
	}

      sleep((unsigned int) 3);
      disc=true;
      for (std::vector<MonClient*>::iterator ic=clients.begin();ic!=clients.end();ic++)
	{
	  disc = disc && (*ic)->isDisconnected();
	}
      
    }
}
