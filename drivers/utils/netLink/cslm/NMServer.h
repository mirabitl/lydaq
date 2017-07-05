#ifndef _NMServer_h_
#define _NMServer_h_

#include "NetMessage.h"
#include "MessageHandler.h"
#include <boost/function.hpp>
#include <boost/thread.hpp>



class NMServer
{
public:
  NMServer(std::string host,uint32_t port);
 
  void start();
 
  void join();
 
  void svc();

  void registerService(std::string s);
  void destroyService(std::string s);

  void updateService(std::string s,NetMessage* m);
  void registerCommand(std::string cmd,boost::function<NetMessage* (NetMessage*)> f);
protected:
  NL::Socket* socketServer_;
  NL::SocketGroup* group_;

  OnAccept* onAccept_;
  OnRead* onRead_;
  OnDisconnect* onDisconnect_;
  MessageHandler* mh_;
  boost::thread    m_Thread;  
};
#endif
