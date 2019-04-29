
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "MessageHandler.hh"

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>
#include "ReadoutLogger.hh"
using namespace std;
using namespace lytdc;




lytdc::OnAccept::OnAccept(MessageHandler* msh) : _msh(msh) {}
void lytdc::OnAccept:: exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) 
{
  NL::Socket* newConnection = socket->accept();
  group->add(newConnection);
  LOG4CXX_INFO(_logLdaq,"\nConnection " << newConnection->hostTo() << ":" << newConnection->portTo() << " added...");
}




lytdc::OnRead::OnRead(MessageHandler* msh) : _msh(msh) {}
void lytdc::OnRead::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  _msh->processMessage(socket);  
}

lytdc::OnDisconnect::OnDisconnect(MessageHandler* msh) : _msh(msh),_disconnect(false)  {}
void lytdc::OnDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  group->remove(socket);
  LOG4CXX_WARN(_logLdaq,"Server disconnected "<<socket->hostTo()<<":"<<socket->portTo());
  cout.flush();
  _disconnect=true;
}
lytdc::OnClientDisconnect::OnClientDisconnect() : _disconnect(false) {}
void lytdc::OnClientDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference)  {

if (!_disconnect)
  LOG4CXX_WARN(_logLdaq,"Client disconnected "<<socket->hostTo()<<":"<<socket->portTo());
  _disconnect=true;
 
}
