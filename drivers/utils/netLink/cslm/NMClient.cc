
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "NMClient.h"
using namespace std;
#include <string>
#include <sstream>
NMClient::NMClient(std::string host,uint32_t port)
{
  NL::init();
  socketClient_=new NL::Socket(host.c_str(),port);
  group_=new NL::SocketGroup();
  mh_ = new MessageHandler();
  onRead_= new OnRead(mh_);
  onDisconnect_= new OnClientDisconnect();
  group_->setCmdOnRead(onRead_);
  group_->setCmdOnDisconnect(onDisconnect_);
  group_->add(socketClient_);
  disconnected_=0;
}
NMClient::~NMClient()
{
  delete onRead_;
  delete onDisconnect_;
  delete mh_;
  delete group_;
  delete socketClient_;


}
void NMClient::start()
{
  m_Thread = boost::thread(&NMClient::svc, this);  
}
void NMClient::join()
{
  m_Thread.join();  
}
void NMClient::svc()
{
  while (disconnected_!=0xDEAD)
    {
      if(!group_->listen(2000,&disconnected_))
	if(1<0) cout << "\nNo msg recieved during the last 2 seconds";
    }
}
 
void NMClient::sendCommand(std::string s,NetMessage* m)
{
  m->setName(s);
  m->setType(NetMessage::COMMAND);
  try {
    MessageHandler::postMessage(socketClient_,m);
  }
  catch (std::string e)
    {
      std::cout<<e<<std::endl;
    }
}

void NMClient::subscribeService(std::string name,boost::function<NetMessage* (NetMessage*)> f)
  {
    
    NetMessage ms("SUBSCRIBE",NetMessage::COMMAND,name.size());
    memcpy(ms.getPayload(),name.c_str(),name.size());
    //std::string s=name;
    //ms.setPayload((unsigned char*) s.c_str(),name.size());
    this->sendCommand("SUBSCRIBE",&ms);
    this->registerHandler(name,f);
  }



void NMClient::registerHandler(std::string cmd,boost::function<NetMessage* (NetMessage*)> f)
{
  mh_->registerCommand(cmd,f);

}
bool NMClient::isDisconnected(){ return (disconnected_==0xDEAD);}
