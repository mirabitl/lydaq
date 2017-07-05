#ifndef _NMClient_h
#define _NMClient_h
#include "MessageHandler.h"
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>

class NMClient
{
public:
  NMClient(std::string host,uint32_t port);
  ~NMClient();
  void start();
  void join();
  void svc();
  void subscribeService(std::string name,boost::function<NetMessage* (NetMessage*)> f);
  void sendCommand(std::string s,NetMessage* m);
  void registerHandler(std::string cmd,boost::function<NetMessage* (NetMessage*)> f);
  bool isDisconnected();
protected:
  NL::Socket* socketClient_;
  NL::SocketGroup* group_;

  OnRead* onRead_;
  OnClientDisconnect* onDisconnect_;
  MessageHandler* mh_;
  boost::thread    m_Thread;  
  uint32_t disconnected_;
};
#endif
