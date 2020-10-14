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
#include "MpiInterface.hh"

using namespace lydaq;

c4i::slcHandler::slcHandler(std::string ip) : socketHandler(ip,c4i::Interface::PORT::SLC)
{
  
}


bool c4i::slcHandler::processPacket()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[c4i::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[c4i::Message::Fmt::TRANS];

  
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<_id<<"SLC data answer="<<transaction<<" length="<<length);
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

}
