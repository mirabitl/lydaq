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

void lydaq::c3i::MpiInterface::dolisten()
{
 
  while(!_onClientDisconnect->disconnected()) {

                if(!_group->listen(4000))
		  std::cout << "\nNo msg recieved during the last 4 seconds";
        }

    
}
void lydaq::c3i::MpiInterface::listen()
{
  g_store.create_thread(boost::bind(&lydaq::c3i::MpiInterface::dolisten, this));
  _running=true;
  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));

}

lydaq::c3i::MpiInterface::MpiInterface() :  _group(NULL),_transaction(0)
{
  // Initialise NetLink
  NL::init();
  _msh =new mpi::MpiMessageHandler("/dev/shm");

 
}

void lydaq::c3i::MpiInterface::initialise()
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
  _vsSlc.clear();
  _vsData.clear();

}
void lydaq::c3i::MpiInterface:: addRegister(std::string address,uint16_t port)
{
  uint64_t id=( (uint64_t) mpi::MpiMessageHandler::convertIP(address)<<32)|port;

  NL::Socket* sc=new NL::Socket(address,port);
  _group->add(sc);
  std::pair<uint64_t, NL::Socket*> p1(id,sc);

  printf("Register on %s %d \n",address.c_str(),port);
  _vsCtrl.insert(p1);
}
void lydaq::c3i::MpiInterface:: addSlcTransfer(std::string address,uint16_t port)
{
  uint64_t id=( (uint64_t) mpi::MpiMessageHandler::convertIP(address)<<32)|port;

  NL::Socket* sc=new NL::Socket(address,port);
  _group->add(sc);
  std::pair<uint64_t, NL::Socket*> p1(id,sc);

  printf("SLC on %s %d \n",address.c_str(),port);
  _vsSlc.insert(p1);
}
void lydaq::c3i::MpiInterface::addDataTransfer(std::string address,uint16_t port)
{
  uint64_t id=( (uint64_t) mpi::MpiMessageHandler::convertIP(address)<<32)|port;
  NL::Socket* stdc=new NL::Socket(address,port);
  _group->add(stdc);
  std::pair<uint64_t, NL::Socket*> p1(id,stdc);
  printf("Data on %s %d \n",address.c_str(),port);
  _vsData.insert(p1);
}

uint32_t  lydaq::c3i::MpiInterface::sendMessage(MpiMessage* m)
{
  std::map<uint64_t,NL::Socket*>::iterator itsock=_vsCtrl.find(m->address());
  if (itsock == _vsCtrl.end())
    {
      printf("Address %lx on port %ld not found \n", (m->address()>>32)&0xFFFFFFFF,m->address()&0XFFFF);
    }
  // Send the Buffer
  try
  {
    m->ptr()[3]=(_transaction++)%255;
    
    itsock->second->send((const void*) m->ptr(),m->length()*sizeof(uint8_t));

    LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" Address "<<std::hex<<((m->address()>>32)&0xFFFFFFFF)<<std::dec<<" Port "<<(m->address()&0XFFFF)<<" Length "<<m->length()<<" Transaction "<<_transaction);

    return (_transaction-1);
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
  printf("Buffer sent %d bytes at Address %lx on port %d \n",m->length(),(m->address()>>32)&0xFFFFFFF,m->address()&0XFFFF);

}

void lydaq::c3i::MpiInterface::registerDataHandler(std::string  address,uint16_t port,MPIFunctor f)
{
  uint64_t id=( (uint64_t) mpi::MpiMessageHandler::convertIP(address)<<32)|port;
  _msh->addHandler(id,f);
}

void lydaq::c3i::MpiInterface::close()
{
  for (auto x=_vsData.begin();x!=_vsData.end();x++)
    {
      _group->remove(x->second);
      x->second->disconnect();
    }
  _vsData.clear();
  for (auto x=_vsSlc.begin();x!=_vsSlc.end();x++)
    {
      _group->remove(x->second);
      x->second->disconnect();
    }
  _vsSlc.clear();
  for (auto x=_vsCtrl.begin();x!=_vsCtrl.end();x++)
    {
      _group->remove(x->second);
      x->second->disconnect();
    }
  _vsCtrl.clear();
  
}
