#ifndef _MessageHandler_h_
#define _MessageHandler_h_

#include "NetMessage.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>
#undef DEBUG_PRINT_ENABLED 

#if DEBUG_PRINT_ENABLED
#define DEBUG printf
#else
#define DEBUG(format, args...) ((void)0)
#endif
#define INFO_PRINT_ENABLED 0

#if INFO_PRINT_ENABLED
#define INFO printf
#else
#define INFO(format, args...) ((void)0)
#endif

class MessageHandler
{
public:
  MessageHandler();
  
  NetMessage* StructureHandler(NetMessage* m);

  virtual void processMessage(NL::Socket* socket) throw (std::string);
  void registerCommand(std::string cmd,boost::function<NetMessage* (NetMessage*)> f);
  static void postMessage(NL::Socket* socket,NetMessage* m) throw (std::string);
  void destroyService(std::string s);
  void registerService(std::string s);
  void subscribeService(std::string s,NL::Socket* sock);
  void removeSocket(NL::Socket* sock);
  void updateService(std::string s,NetMessage* m);
private:
  uint32_t msgLen_;
  unsigned char buf_[0x100000];
  std::map<std::string,boost::function<NetMessage*(NetMessage*)> > commandHandler_; 
  std::map<std::string,std::vector<NL::Socket*> > serviceHandler_;
  NL::Socket* readSocket_;
};


class OnAccept: public NL::SocketGroupCmd 
{

public:
  OnAccept(MessageHandler* msh);
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) ;
private:
  MessageHandler* msh_;
};


class OnRead: public NL::SocketGroupCmd 
{
public:
  OnRead(MessageHandler* msh);
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
public:
  unsigned char theReadBuffer_[0x10000];
private:
  MessageHandler* msh_;
};


class OnDisconnect: public NL::SocketGroupCmd 
{
 public:
  OnDisconnect(MessageHandler* msh);
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
private:
  MessageHandler* msh_;
};



class OnClientDisconnect: public NL::SocketGroupCmd 
{
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
};

#endif
