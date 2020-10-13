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
#define  C3I_FMT_HEADER 0
#define  C3I_FMT_LEN 1
#define  C3I_FMT_TRANS 3
#define  C3I_FMT_CMD 4
#define  C3I_FMT_PAYLOAD 6


namespace lydaq
{
  namespace c3i
  {
    class MpiMessage {
    public:
      enum  command { WRITEREG=1,READREG=2,SLC=4,DATA=8,ACKNOWLEDGE=16,ERROR=32};
      enum Register {TEST=0x0,ID=0x1,NBASIC=0x2,
		 SLC_CTRL=0x11,SLC_STATUS=0x12,SLC_SIZE=0x13,
		 PP_PWRA_PWRD=0x21,PP_PWRD_ACQ=0x22,PP_ACQ_PWRD=0x23,PP_PWRD_PWRA=0x24,PP_CTRL=0x25,
		 ACQ_CTRL=0x31,ACQ_FMT=0x31,ACQ_RST=0x32,
		 BME_CAL1=0x40,BME_CAL2=0x41,BME_CAL3=0x42,BME_CAL4=0x43,
		 BME_CAL5=0x44,BME_CAL6=0x45,BME_CAL7=0x46,BME_CAL8=0x47,
		 BME_HUM=0x48,BME_PRES=0x49,BME_TEMP=0x4A,
		 CTEST_CTRL=0x50,VERSION=0xFF};

      MpiMessage(): _address(0),_length(2) {;}
      inline uint64_t address(){return _address;}
      inline uint16_t length(){return _length;}
      inline uint8_t* ptr(){return _buf;}
      inline void setLength(uint16_t l){_length=l;}
      inline void setAddress(uint64_t a){_address=a;}
      inline void setAddress(std::string address,uint16_t port){_address=( (uint64_t) mpi::MpiMessageHandler::convertIP(address)<<32)|port;}
    private:
      uint64_t _address;
      uint16_t _length;
      uint8_t _buf[MAX_BUFFER_LEN];
    
    };

    class MpiInterface 
    {
    public:
      enum PORT { REGISTER=10002,SLC=10001,DATA=10003};
      MpiInterface();
      ~MpiInterface(){;}
      void initialise();
      void addRegister(std::string address,uint16_t port);
      void addDataTransfer(std::string address,uint16_t port);
      void addSlcTransfer(std::string address,uint16_t port);
      void listen();
      uint32_t sendMessage(MpiMessage* wmsg);
      void registerDataHandler(std::string  address,uint16_t port,MPIFunctor f);
      uint32_t transaction() {return _transaction;}
      inline std::map<uint64_t,NL::Socket*>& controlSockets(){ return _vsCtrl;}
      inline std::map<uint64_t,NL::Socket*>& slcSockets(){ return _vsSlc;}
      void close();
    private:
      void dolisten();
  
      std::map<uint64_t,NL::Socket*> _vsCtrl;
      std::map<uint64_t,NL::Socket*> _vsData;
      std::map<uint64_t,NL::Socket*> _vsSlc;

      NL::SocketGroup* _group;
 
      mpi::MpiMessageHandler* _msh;
      mpi::OnRead* _onRead;
      mpi::OnAccept* _onAccept;
      mpi::OnClientDisconnect* _onClientDisconnect;
      mpi::OnDisconnect* _onDisconnect;
      boost::thread_group g_store;
      boost::thread_group g_run;
      bool _running;
      uint32_t _transaction;
    };
  };
};
#endif
