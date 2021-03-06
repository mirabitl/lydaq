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

c4i::slcHandler::slcHandler(std::string ip) : socketHandler(ip,c4i::Interface::PORT::SLC)
{
  _msg=new c4i::Message();
}

void c4i::slcHandler::sendSlowControl(uint8_t* slc,uint16_t lenbytes)
{
  uint16_t hrlen=lenbytes;
  uint16_t cpl32bit=4-hrlen%4;
  uint16_t len=hrlen+cpl32bit+c4i::Message::Fmt::PAYLOAD+1;
  // Hardcode dasn le firmware
  hrlen=109;
  len=118;
  _msg->setAddress(id());
  _msg->setLength(len);
  uint16_t* sp=(uint16_t*) &(_msg->ptr()[c4i::Message::Fmt::LEN]);
  _msg->ptr()[c4i::Message::Fmt::HEADER]='(';
  sp[0]=htons(len);
  _msg->ptr()[c4i::Message::Fmt::CMD]=lydaq::c4i::Message::command::SLC;
  memcpy(&(_msg->ptr()[c4i::Message::Fmt::PAYLOAD]),slc,hrlen);
  _msg->ptr()[len-1]=')';
  uint32_t tr=this->sendMessage(_msg);
}


bool c4i::slcHandler::processPacket()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[c4i::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[c4i::Message::Fmt::TRANS];

  
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<this->sourceid()<<"SLC data answer="<<transaction<<" length="<<length);
  fprintf(stderr,"\nSLC RC ==> ");
  for (int i=0;i<_idx;i++)
    {
      fprintf(stderr,"%.2x ",(_buf[i]));
      
      if (i%16==15)
	{
	  fprintf(stderr,"\n==> ");
	}
    }
  fprintf(stderr,"\n");
  return true;
}
