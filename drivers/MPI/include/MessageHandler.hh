#ifndef _MessageHandler_h_
#define _MessageHandler_h_

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>
#include "debug.hh"
namespace mpi {

struct MpiException : public std::exception
{
   std::string s;
   MpiException(std::string ss) : s(ss) {}
   ~MpiException() throw () {} // Updated
   const char* what() const throw() { return s.c_str(); }
};
  
class MessageHandler
{
public:
  virtual void processMessage(NL::Socket* socket){;} //throw (MpiException){;}
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
  bool disconnected(){return _disconnect;}
private:
  MessageHandler* _msh;
  bool _disconnect;
};



class OnClientDisconnect: public NL::SocketGroupCmd 
{
public:
  OnClientDisconnect();
  void exec(NL::Socket* socket, NL::SocketGroup* group, void* reference);
  bool disconnected(){return _disconnect;}
private:
  bool _disconnect;
};
};
#endif
