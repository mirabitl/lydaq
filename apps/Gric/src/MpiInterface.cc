#include "MpiInterface.hh"
using namespace lytdc;
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <boost/format.hpp>




using namespace lydaq;

void lydaq::MpiInterface::dolisten()
{
 
       while(true) {

                if(!_group->listen(2000))
		  std::cout << "\nNo msg recieved during the last 2 seconds";
        }

    
}
void lydaq::MpiInterface::listen()
{
  g_store.create_thread(boost::bind(&lydaq::MpiInterface::dolisten, this));
  _running=true;
  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));

}

lydaq::MpiInterface::MpiInterface() :  _group(NULL),_transaction(0)
{
  // Initialise NetLink
  NL::init();
  _msh =new lydaq::MpiMessageHandler("/dev/shm");

  
}

void lydaq::MpiInterface::initialise()
{
 
   // Initialise the network
   if (_group!=NULL) delete _group;
   _group=new NL::SocketGroup();
   _onRead= new lytdc::OnRead(_msh);
  _onClientDisconnect= new lytdc::OnClientDisconnect();
  _onDisconnect= new lytdc::OnDisconnect(_msh);
  _onAccept=new lytdc::OnAccept(_msh);
  _group->setCmdOnRead(_onRead);
  _group->setCmdOnAccept(_onAccept);
  _group->setCmdOnDisconnect(_onClientDisconnect);
   // Loop on Asic Map and find existing DIF
  // Register their slow control socket (10001) and readout one (10002)
  _vsCtrl.clear();
  _vsMpi.clear();

}
void lydaq::MpiInterface:: addCommunication(std::string address,uint16_t port)
{
  uint64_t id=( (uint64_t) lydaq::MpiMessageHandler::convertIP(address)<<32)|port;

  NL::Socket* sc=new NL::Socket(address,port);
  _group->add(sc);
  std::pair<uint64_t, NL::Socket*> p1(id,sc);

  printf("Communication on %s %d \n",address.c_str(),port);
  _vsCtrl.insert(p1);
}
void lydaq::MpiInterface::addDataTransfer(std::string address,uint16_t port)
{
  uint64_t id=( (uint64_t) lydaq::MpiMessageHandler::convertIP(address)<<32)|port;
  NL::Socket* stdc=new NL::Socket(address,port);
  _group->add(stdc);
  std::pair<uint64_t, NL::Socket*> p1(id,stdc);
  printf("Data on %s %d \n",address.c_str(),port);
  _vsMpi.insert(p1);
}

void lydaq::MpiInterface::sendMessage(MpiMessage* m)
{
  std::map<uint64_t,NL::Socket*>::iterator itsock=_vsCtrl.find(m->address());
  if (itsock == _vsCtrl.end())
    {
      printf("Address %x on port %d not found \n", (m->address()>>32)&0xFFFFFFFF,m->address()&0XFFFF);
    }
  // Send the Buffer
  try
  {
    m->ptr()[3]=(_transaction++)%255;
    itsock->second->send((const void*) m->ptr(),m->length()*sizeof(uint8_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
  printf("Buffer sent %d bytes at Address %lx on port %d \n",m->length(),(m->address()>>32)&0xFFFFFFF,m->address()&0XFFFF);
}

void lydaq::MpiInterface::registerDataHandler(std::string  address,uint16_t port,FEBFunctor f)
{
  uint64_t id=( (uint64_t) lydaq::MpiMessageHandler::convertIP(address)<<32)|port;
  _msh->addHandler(id,f);
}


