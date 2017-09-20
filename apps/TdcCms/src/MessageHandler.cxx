
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "MessageHandler.hh"

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>

using namespace std;
using namespace lytdc;




lytdc::OnAccept::OnAccept(MessageHandler* msh) : _msh(msh) {}
void lytdc::OnAccept:: exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) 
{
  NL::Socket* newConnection = socket->accept();
  group->add(newConnection);
  cout << "\nConnection " << newConnection->hostTo() << ":" << newConnection->portTo() << " added...";
  cout.flush();
}




lytdc::OnRead::OnRead(MessageHandler* msh) : _msh(msh) {}
void lytdc::OnRead::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  //cout << "\nREAD -- ";
  _msh->processMessage(socket);


    cout.flush();
  /*    unsigned char buffer[256];
    buffer[255] = '\0';
    memset(theReadBuffer_,0,0x10000);
    //socket->read(buffer, 255);
    size_t msgLen =socket->read(theReadBuffer_,0x10000);
    std::string sread(buffer);
    cout << "Message from " << socket->hostTo() << ":" << socket->portTo() << ". Text received: " << sread<<std::endl;
    cout.flush();
		
    for(unsigned i=1; i < (unsigned) group->size(); ++i)
    {
    if(group->get(i) != socket)
    {
    printf(" \t sending %d  %d %s \n",i,sread.size(),sread.c_str());
    group->get(i)->send(sread.c_str(), sread.size());
    }
    }
  */
}

lytdc::OnDisconnect::OnDisconnect(MessageHandler* msh) : _msh(msh) {}
void lytdc::OnDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  group->remove(socket);
  cout << "\nClient " << socket->hostTo() << " disconnected...";
  _msh->removeSocket(socket);
  cout.flush();
  delete socket;
}
void lytdc::OnClientDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  cout << "\nClient " << socket->hostTo() << " disconnected..."<<std::flush;
  uint32_t* i =(uint32_t*) reference;
  (*i)=0xDEAD;
}
