#ifndef _C4I_INTERFACE_HH
#define _C4I_INTERFACE_HH

#include "MpiMessageHandler.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"
#define MAX_BUFFER_LEN 0x4000
#include "zmSender.hh"
#define C3I_VERSION 145
#define MBSIZE 0x40000

namespace lydaq
{
  namespace c4i
  {
    class board;
    class Message {
    public:
      enum Fmt {HEADER=0,LEN=1,TRANS=3,CMD=4,PAYLOAD=6};
      enum  command { WRITEREG=1,READREG=2,SLC=4,DATA=8,ACKNOWLEDGE=16,ERROR=32};
      enum Register {TEST=0x0,ID=0x1,NBASIC=0x2,
		 SLC_CTRL=0x11,SLC_STATUS=0x12,SLC_SIZE=0x13,
		 PP_PWRA_PWRD=0x21,PP_PWRD_ACQ=0x22,PP_ACQ_PWRD=0x23,PP_PWRD_PWRA=0x24,PP_CTRL=0x25,
		 ACQ_CTRL=0x30,ACQ_FMT=0x31,ACQ_RST=0x32,
		 BME_CAL1=0x40,BME_CAL2=0x41,BME_CAL3=0x42,BME_CAL4=0x43,
		 BME_CAL5=0x44,BME_CAL6=0x45,BME_CAL7=0x46,BME_CAL8=0x47,
		 BME_HUM=0x48,BME_PRES=0x49,BME_TEMP=0x4A,
		 CTEST_CTRL=0x50,VERSION=0xFF};

      Message(): _address(0),_length(2) {;}
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


    /// Gere les connections aux socket et le select
    
    class Interface 
    {
    public:
      enum PORT { REGISTER=10002,SLC=10001,DATA=10003};
      Interface();
      ~Interface(){;}
      void initialise();
      void addDevice(std::string address);
      void listen();

      inline std::map<std::string,c4i::board*>& boards(){ return _boards;}
      inline c4i::board* getBoard(std::string ip) {return _boards[ip];} 
      void close();
    private:
      void dolisten();
      std::map<std::string,c4i::board*> _boards;

      NL::SocketGroup* _group;
 
      mpi::MpiMessageHandler* _msh;
      mpi::OnRead* _onRead;
      mpi::OnAccept* _onAccept;
      mpi::OnClientDisconnect* _onClientDisconnect;
      mpi::OnDisconnect* _onDisconnect;
      boost::thread_group g_store;
      boost::thread_group g_run;
      bool _running;
    };

    // Gere chaque socket
    // processPacket est virtuel
    class socketHandler
    {
    public:
      socketHandler(std::string,uint32_t port);
      int16_t checkBuffer(uint8_t* b,uint32_t maxidx);
      uint32_t sendMessage(c4i::Message* wmsg);
      virtual void processBuffer(uint64_t id, uint16_t l,char* b);
      void purgeBuffer();
      virtual bool processPacket()=0;
      NL::Socket* socket(){return _sock;}
      uint64_t id() {return _id;}
      uint32_t ipid() {return (_id>>32)&0xFFFFFFFF;}
      uint32_t sourceid() {return (ipid()>>16)&0XFFFF;}
      //uint32_t sourceid() {return ipid();}
      inline uint8_t* answer(uint8_t tr){return _answ[tr];}
      uint32_t transaction() {return _transaction;}

      void clear();
    protected:
      uint32_t _idx;
      uint8_t _buf[MBSIZE];
    private:
      uint64_t _id;
      
      NL::Socket* _sock;
      // temporary buffer to collect reply
      uint8_t _b[MBSIZE];
      // Command answers pointers
      std::map<uint8_t,uint8_t*> _answ;
      uint32_t _transaction;

    };
    // Gere la socket registre
    class registerHandler : public socketHandler
    {
    public:
      registerHandler(std::string);
      virtual bool processPacket();
      void writeRegister(uint16_t address,uint32_t value);
      uint32_t readRegister(uint16_t address);
      void processReply(uint32_t tr,uint32_t* rep=0);
      inline uint32_t slcStatus(){return _slcStatus;}
      inline void setSlcStatus(uint32_t i){_slcStatus=i;}
      inline void useTransactionId(){_noTransReply=false;}
    private:
      uint32_t _slcStatus;
      c4i::Message* _msg;
      bool _noTransReply;
    };

    // Gere la socket Slow control
    class slcHandler : public socketHandler
    {
    public:
      slcHandler(std::string);
      virtual bool processPacket();
      void sendSlowControl(uint8_t* slc,uint16_t len_bytes=109);
    private:
      c4i::Message* _msg;
    };

    // gere la socket data et envoie les donnees a l'EVB
    class dataHandler : public socketHandler
    {
    public:
      dataHandler(std::string ip);
      virtual bool processPacket();
      virtual void processBuffer(uint64_t id, uint16_t l,char* b);
      inline void setTriggerId(uint8_t i) {_triggerId=i;}

      inline uint32_t detectorId(){return _detId;}
      inline uint32_t difId(){return ((this->id()>>48)&0xFFFF);}
      inline uint64_t abcid(){return _lastABCID;}
      inline uint32_t gtc(){return _lastGTC;}
      inline uint32_t packets(){return _nProcessed;}
      inline uint32_t event(){return _event;}
      inline uint32_t triggers(){return _ntrg;}
      void clear();
      void connect(zmq::context_t* c,std::string dest);
      void autoRegister(zmq::context_t* c,Json::Value config,std::string appname,std::string portname);
    private:
      uint64_t _lastABCID;
      uint32_t _lastGTC,_lastBCID,_event,_ntrg,_expectedLength;
      uint32_t _nProcessed;
      zdaq::zmSender* _dsData;
      uint8_t _triggerId;
      uint32_t _detId;

      
    };

    /// Un board => 3 socket
    class board
    {
    public:
      board(std::string ip);
      inline c4i::registerHandler* reg(){return _regh;}
      inline c4i::slcHandler* slc(){return _slch;}
      inline c4i::dataHandler* data(){return _datah;}
      inline std::string ipAddress(){return _ip;}
    private:
      std::string _ip;
      c4i::registerHandler* _regh;
      c4i::slcHandler* _slch;
      c4i::dataHandler* _datah;
      
    };

  };
};
#endif
