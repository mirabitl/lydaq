#ifndef _MPI_INTERFACE_HH
#define _MPI_INTERFACE_HH

#include "MpiMessageHandler.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"
#define MAX_BUFFER_LEN 0x4000
namespace lydaq
{
  class MpiMessage {
  public:
    enum  command { STARTACQ=0,STOPACQ=1,RESET=2,READSC=5,LOADSC=6,STORESC=7,LASTABCID=3,LASTGTC=4,CLOSE=10};
    MpiMessage(): _address(0),_length(2) {;}
    inline uint64_t address(){return _address;}
    inline uint16_t length(){return _length;}
    inline uint8_t* ptr(){return _buf;}
    inline void setLength(uint16_t l){_length=l;}
    inline void setAddress(uint64_t a){_address=a;}
  private:
    uint64_t _address;
    uint16_t _length;
    uint8_t _buf[MAX_BUFFER_LEN];
    
  };

class MpiInterface 
{
public:
  enum PORT { CTRL=9760,DATA=9761,SENSOR=9762};
  MpiInterface();
  ~MpiInterface(){;}
  void initialise();
  void addCommunication(std::string address,uint16_t port);
  void addDataTransfer(std::string address,uint16_t port);
  void listen();
  uint32_t sendMessage(MpiMessage* wmsg);
  void registerDataHandler(std::string  address,uint16_t port,FEBFunctor f);
  uint32_t transaction() {return _transaction;}
  inline std::map<uint64_t,NL::Socket*>& controlSockets(){ return _vsCtrl;}
  void close();
private:
  void dolisten();
  
  std::map<uint64_t,NL::Socket*> _vsCtrl;
  std::map<uint64_t,NL::Socket*> _vsMpi;

  NL::SocketGroup* _group;
 
  lydaq::MpiMessageHandler* _msh;
  lytdc::OnRead* _onRead;
  lytdc::OnAccept* _onAccept;
  lytdc::OnClientDisconnect* _onClientDisconnect;
  lytdc::OnDisconnect* _onDisconnect;
  boost::thread_group g_store;
  boost::thread_group g_run;
  bool _running;
  uint32_t _transaction;
};
};
#endif
