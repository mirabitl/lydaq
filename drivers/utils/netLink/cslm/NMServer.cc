
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "NMServer.h"
using namespace std;
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>
#include <sstream>

NMServer::NMServer(std::string host,uint32_t port)
  {
    NL::init();
    try {
      //socketServer_=new NL::Socket(host.c_str(),port);
      socketServer_=new NL::Socket(port);
    }
    catch (NL::Exception e)
      {
	std::cout << e.msg()<<std::endl;
	exit(1);
      }
    group_=new NL::SocketGroup();
    mh_ = new MessageHandler();
    onAccept_= new OnAccept(mh_);
    onRead_= new OnRead(mh_);
    onDisconnect_= new OnDisconnect(mh_);
    group_->setCmdOnAccept(onAccept_);
    group_->setCmdOnRead(onRead_);
    group_->setCmdOnDisconnect(onDisconnect_);
    group_->add(socketServer_);
  }
  void NMServer::start()
  {
    m_Thread = boost::thread(&NMServer::svc, this);  
  }
  void NMServer::join()
  {
    m_Thread.join();  
  }
  void NMServer::svc()
  {
    while (true)
      {
	if(!group_->listen(2000))
	  if(1<0) cout << "\nNo msg recieved during the last 2 seconds";
      }
  }
  void NMServer::registerService(std::string s)
  {
    mh_->registerService(s);
  }
  void NMServer::destroyService(std::string s)
  {
    mh_->destroyService(s);
  }
  void NMServer::updateService(std::string s,NetMessage* m)
  {
    mh_->updateService(s,m);
  }
  void NMServer::registerCommand(std::string cmd,boost::function<NetMessage* (NetMessage*)> f)
  {
    mh_->registerCommand(cmd,f);
  }
