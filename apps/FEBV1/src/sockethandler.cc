#include "Febv1Interface.hh"
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

febv1::socketHandler::socketHandler(std::string ip,uint32_t port) : _idx(0),_transaction(0)
{
  memset(_buf,0,MBSIZE);
  memset(_b,0,MBSIZE);
  // initialise answer storage
  for (int i=0;i<NREP;i++)
    {
      uint8_t* b=new uint8_t[0x4000];
      std::pair<uint8_t,uint8_t*> p(i,b);
      _answ.insert(p);
    }
  // Now create the socket
  _id=( (uint64_t) mpi::MpiMessageHandler::convertIP(ip)<<32)|port;

  _sock=new NL::Socket(ip,port);
}

void febv1::socketHandler::clear()
{
  for (int i=0;i<NREP;i++)
    {
      memset(_answ[i],0,0x4000);
    }

}
uint32_t febv1::socketHandler::sendMessage(febv1::Message* m)
{
  LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<" Address "<<std::hex<<((m->address()>>32)&0xFFFFFFFF)<<std::dec<<" Port "<<(m->address()&0XFFFF)<<" Length "<<m->length()<<" Transaction "<<_transaction);
  // Send the Buffer
  try
  {
    //m->ptr()[febv1::Message::Fmt::TRANS]=(_transaction++)%NREP;
    // Clear the ack tag for reply
    memset(_answ[0],0,0x4000);
    _transaction=0;
    _sock->send((const void*) m->ptr(),m->length()*sizeof(uint8_t));

    LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<" Address "<<std::hex<<((m->address()>>32)&0xFFFFFFFF)<<std::dec<<" Port "<<(m->address()&0XFFFF)<<" Length "<<m->length()<<" Transaction "<<_transaction);

    return (0);
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
  printf("Buffer sent %d bytes at Address %lx on port %ld \n",m->length(),(m->address()>>32)&0xFFFFFFF,m->address()&0XFFFF);


}
int16_t febv1::socketHandler::checkBuffer(uint8_t* b,uint32_t maxidx)
{

   uint32_t *_lBuf = (uint32_t *)b;
  uint16_t *_sBuf = (uint16_t *)b;
  uint32_t elen = 0;
  if (ntohl(_lBuf[0]) != 0xcafedade && ntohl(_lBuf[0]) != 0xcafebabe)
    return 0;
  // Check TAG coherency
  if (ntohl(_lBuf[0]) == 0xcafedade)
  {
    elen = 18; // Header
    uint32_t *leb = (uint32_t *)&b[elen - 4];
    if (elen > maxidx)
    {
      LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << "CheckBuf header:Not enough data ELEN " << elen << " MAXID " << maxidx);
      return -5;
    }
    if (ntohl(leb[0]) == 0xdadecafe)
      return elen;
    else
    {
      LOG4CXX_WARN(_logFeb, __PRETTY_FUNCTION__ << "CheckBuf header :Missing CAFEDADE end tag ");
      return -1;
    }
  }
  else
  {
    if (ntohl(_lBuf[0]) == 0xcafebabe)
    {
      elen = ntohs(_sBuf[5]) * CHBYTES + 16; //Channels
      if (elen < 16 || elen > 208)
      {
        LOG4CXX_WARN(_logFeb, __PRETTY_FUNCTION__ << "CheckBuf:Wrong size " << elen);
        return -2;
      }
      if (elen > maxidx)
      {
        LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << "CheckBuf:Not enough data ELEN" << elen << " MAXID " << maxidx);
        return -3;
      }
      uint32_t *leb = (uint32_t *)&b[elen - 4];
      if (ntohl(leb[0]) == 0xbabecafe)
        return elen;
      else
      {
        LOG4CXX_WARN(_logFeb, __PRETTY_FUNCTION__ << "CheckBuf:Missing CAFEBABE end tag ");
        return -4;
      }
    }
    else
      return 0;
  }
}

void febv1::socketHandler::processBuffer(uint64_t id, uint16_t l,char* bb)
{
  LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"Entering procesBuffer "<<std::hex<<id<<std::dec<<" Length "<<l);
for (uint16_t ibx = 0; ibx < l; ibx++)
  {
    int16_t tag = checkBuffer((uint8_t *)&bb[ibx], l - ibx + 1);
    if (tag > 0)
    {
      bool ok = processPacket();
      if (ok)
      {
        _idx = 0;
      }
    }
    else
    {
#undef LUT
#ifdef LUT
      if (ibx == 0)
      {
        printf("LUT content \n==> ");
        for (int i = 0; i < l; i++)
        {
          printf("%.2x ", ((uint8_t)bb[i]));

          if (i % 16 == 15)
          {
            printf("\n==> ");
          }
        }
        printf("\n");
      }
#endif
      if (tag < 0)
      {
        LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << _id << "StructError Tag=" << tag << " ibx=" << ibx << " _idx=" << _idx);
      }
    }
    _buf[_idx] = bb[ibx];
    _idx++;
  }
 return;
















  
}
void febv1::socketHandler::purgeBuffer()
{
  fprintf(stderr,"Entering PURGEBUFFER %d \n",_idx);
}
