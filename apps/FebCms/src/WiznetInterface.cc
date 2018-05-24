#include "WiznetInterface.hh"
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

void lydaq::WiznetInterface::dolisten()
{
 
       while(true) {

                if(!_group->listen(2000))
		  std::cout << "\nNo msg recieved during the last 2 seconds";
        }

    
}
void lydaq::WiznetInterface::listen()
{
  g_store.create_thread(boost::bind(&lydaq::WiznetInterface::dolisten, this));
  _running=true;
  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));

}

lydaq::WiznetInterface::WiznetInterface() :  _group(NULL)
{
  // Initialise NetLink
  NL::init();
  _msh =new lydaq::WiznetMessageHandler("/dev/shm");

  
}

void lydaq::WiznetInterface::initialise()
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
  _vsTdc.clear();

}
void lydaq::WiznetInterface:: addCommunication(std::string address,uint16_t port)
{
  uint64_t id=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(address)<<32)|port;

  NL::Socket* sc=new NL::Socket(address,port);
  _group->add(sc);
  std::pair<uint64_t, NL::Socket*> p1(id,sc);

  printf("Communication on %s %d \n",address.c_str(),port);
  _vsCtrl.insert(p1);
}
void lydaq::WiznetInterface::addDataTransfer(std::string address,uint16_t port)
{
  uint64_t id=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(address)<<32)|port;
  NL::Socket* stdc=new NL::Socket(address,port);
  _group->add(stdc);
  std::pair<uint64_t, NL::Socket*> p1(id,stdc);
  printf("Data on %s %d \n",address.c_str(),port);
  _vsTdc.insert(p1);
}

void lydaq::WiznetInterface::sendMessage(WiznetMessage* m)
{
  std::map<uint64_t,NL::Socket*>::iterator itsock=_vsCtrl.find(m->_address);
  if (itsock == _vsCtrl.end())
    {
      printf("Address %x on port %d not found \n", (m->_address>>32)&0xFFFFFFFF,m->_address&0XFFFF);
    }
  // Send the Buffer
  try
  {
    itsock->second->send((const void*) m->_buf,m->_length*sizeof(uint16_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
  printf("Buffer sent %d bytes at Address %lx on port %d \n",m->_length,(m->_address>>32)&0xFFFFFFF,m->_address&0XFFFF);
}

void lydaq::WiznetInterface::registerDataHandler(std::string  address,uint16_t port,FEBFunctor f)
{
  uint64_t id=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(address)<<32)|port;
  _msh->addHandler(id,f);
}
