#include "C4iInterface.hh"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
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
#include <map>
#include <bitset>
#include <boost/format.hpp>
#include <arpa/inet.h>


using namespace lydaq;

c4i::registerHandler::registerHandler(std::string ip) : socketHandler(ip,c4i::Interface::PORT::REGISTER)
{
  _msg= new c4i::Message();
}

void c4i::registerHandler::writeRegister(uint16_t address,uint32_t value)
{
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[c4i::Message::Fmt::LEN]);
  _msg->ptr()[c4i::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[c4i::Message::Fmt::CMD]=lydaq::c4i::Message::command::WRITEREG;
  uint16_t radr=htons(address);uint32_t rval=htonl(value);
  memcpy(&(_msg->ptr()[c4i::Message::Fmt::PAYLOAD]),&radr,2);
  memcpy(&(_msg->ptr()[c4i::Message::Fmt::PAYLOAD+2]),&rval,4);
  
  _msg->ptr()[len-1]=')';    
  uint32_t tr=this->sendMessage(_msg);
  this->processReply(0);
}
uint32_t c4i::registerHandler::readRegister(uint16_t address)
{
  uint16_t len=16;
  _msg->setAddress(this->id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[c4i::Message::Fmt::LEN]);
  _msg->ptr()[c4i::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[c4i::Message::Fmt::CMD]=lydaq::c4i::Message::command::READREG;
  uint16_t radr=htons(address);
  memcpy(&(_msg->ptr()[c4i::Message::Fmt::PAYLOAD]),&radr,2);
  _msg->ptr()[len-1]=')';    
  uint32_t tr=this->sendMessage(_msg);
  uint32_t rep=0;
  this->processReply(0,&rep);
  return ntohl(rep);
}

void c4i::registerHandler::processReply(uint32_t tr,uint32_t* reply)
{
  uint8_t b[0x4000];
  
  uint8_t* rep=this->answer(tr%255);
  int cnt=0;
  while (rep[c4i::Message::Fmt::CMD]!=lydaq::c4i::Message::ACKNOWLEDGE )
    {
      usleep(1000);
      cnt++;
      if (cnt>1000)
	{
	  LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" no return after "<<cnt);
	  break;
	}
    }

  // Dump returned buffer
  memcpy(b,rep,0x4000);
  uint16_t* _sBuf= (uint16_t*) &b[c4i::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t trame=b[c4i::Message::Fmt::TRANS];
  uint8_t command=b[c4i::Message::Fmt::CMD];
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" REPLY command ="<<(int) command<<" length="<<length<<" trame id="<<(int) trame);
  /*
  fprintf(stderr,">>>>>>>>>>>>>>>>>>>>>>>>>>>>>\n");
  
  for (int i=c4i::Message::Fmt::PAYLOAD;i<length-1;i++)
    {
      fprintf(stderr,"%.2x ",(b[i]));
      
      if ((i-4)%16==15)
	{
	  fprintf(stderr,"\n");
	}
    }
  fprintf(stderr,"\n<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
  */
  if (reply!=0) //special case for read register
    {
      memcpy(reply,&rep[c4i::Message::Fmt::PAYLOAD+2],sizeof(uint32_t));
      return;
    }

}

bool c4i::registerHandler::processPacket()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[c4i::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[c4i::Message::Fmt::TRANS];
  _sBuf=(uint16_t*) &_buf[c4i::Message::Fmt::PAYLOAD];
  uint16_t address=ntohs(_sBuf[0]);
  uint32_t* lBuf=(uint32_t*) &_sBuf[1];
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<sourceid()<<" Command answer="<<
               std::hex<<(int) address<<":"<<(int) ntohl(lBuf[0])<<std::dec
               <<" length="<<length<<" trame id="<<(int) transaction<<" buffer length "<<_idx);

  memcpy(answer(transaction%255),_buf,length);

#define DUMPREGREPN
#ifdef DUMPREGRE
  fprintf(stderr,"\n REGISTER RC ==> ");
  for (int i=0;i<_idx-1;i++)
    {
      fprintf(stderr,"%.2x ",(_buf[i]));
         
      if (i%16==15)
	{
	  fprintf(stderr,"\n REGISTER RC ==> ");
	}
    }
  fprintf(stderr,"\n");
#endif
}
