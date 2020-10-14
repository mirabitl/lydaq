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
  
}


bool c4i::registerHandler::processPacket()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[c4i::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[c4i::Message::Fmt::TRANS];
  _sBuf=(uint16_t*) &_buf[c4i::Message::Fmt::PAYLOAD];
  uint16_t address=ntohs(_sBuf[0]);
  uint32_t* lBuf=(uint32_t*) &_sBuf[1];
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<_ipid<<" Command answer="<<
               std::hex<<(int) address<<":"<<(int) ntohl(lBuf[0])<<std::dec
               <<" length="<<length<<" trame id="<<(int) transaction<<" buffer length "<<_idx);

  memcpy(_answ[transaction%255],_buf,length);

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
