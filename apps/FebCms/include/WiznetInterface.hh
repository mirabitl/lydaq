#ifndef _WIZNET_INTERFACE_HH
#define _WIZNET_INTERFACE_HH

#include "WiznetMessageHandler.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"
#define MAX_BUFFER_LEN 0x3FFC
namespace lydaq
{
  class WiznetMessage {
  public:
    WiznetMessage(): _address(0),_length(0) {;}
    uint64_t _address;
    uint16_t _length;
    uint16_t _buf[MAX_BUFFER_LEN];
    
  };

class WiznetInterface 
{
public:
  WiznetInterface();
  ~WiznetInterface(){;}
  void initialise();
  void addCommunication(std::string address,uint16_t port);
  void addDataTransfer(std::string address,uint16_t port);
  void listen();
  void sendMessage(WiznetMessage* wmsg);
  void registerDataHandler(std::string  address,uint16_t port,FEBFunctor f);
private:
  void dolisten();
  
  std::map<uint64_t,NL::Socket*> _vsCtrl;
  std::map<uint64_t,NL::Socket*> _vsTdc;

  NL::SocketGroup* _group;
 
  lydaq::WiznetMessageHandler* _msh;
  lytdc::OnRead* _onRead;
  lytdc::OnAccept* _onAccept;
  lytdc::OnClientDisconnect* _onClientDisconnect;
  lytdc::OnDisconnect* _onDisconnect;
  boost::thread_group g_store;
  boost::thread_group g_run;
  bool _running;
};
};
#endif
