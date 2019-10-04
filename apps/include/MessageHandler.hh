#ifndef _MessageHandler_h_
#define _MessageHandler_h_

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
namespace lytdc {
class MessageHandler
{
public:
  virtual void processMessage(NL::Socket* socket) throw (std::string){;}
  virtual void removeSocket(NL::Socket* sock){;}
};


class OnAccept: public NL::SocketGroupCmd 
{

public:
  OnAccept(MessageHandler* msh);
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) ;
private:
  MessageHandler* _msh;
};


class OnRead: public NL::SocketGroupCmd 
{
public:
  OnRead(MessageHandler* msh);
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
public:
  unsigned char _readBuffer[0x10000];
private:
  MessageHandler* _msh;
};


class OnDisconnect: public NL::SocketGroupCmd 
{
 public:
  OnDisconnect(MessageHandler* msh);
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
private:
  MessageHandler* _msh;
};



class OnClientDisconnect: public NL::SocketGroupCmd 
{
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
};
};
#endif
