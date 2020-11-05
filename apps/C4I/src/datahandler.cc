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

c4i::dataHandler::dataHandler(std::string ip) : socketHandler(ip,c4i::Interface::PORT::DATA),_ntrg(0),_dsData(NULL),_triggerId(0),_detId(160)
{
  
}
void c4i::dataHandler::connect(zmq::context_t* c,std::string dest)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new zdaq::zmSender(c,_detId,this->sourceid());
  _dsData->connect(dest);
}
void c4i::dataHandler::clear()
{
  _nProcessed=0;
  _lastGTC=0;
  _lastABCID=0;
  _lastBCID=0;
  _event=0;
  for (int i=0;i<255;i++)
    {
      memset(this->answer(i),0,MAX_BUFFER_LEN);
    }
}


void c4i::dataHandler::autoRegister(zmq::context_t* c,Json::Value config,std::string appname,std::string portname)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new zdaq::zmSender(c,_detId,this->sourceid());
  _dsData->autoDiscover(config,appname,portname);//
  //for (uint32_t i=0;i<_mStream.size();i++)
  //        ds->connect(_mStream[i]);
  _dsData->collectorRegister();

}

bool c4i::dataHandler::processPacket()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[c4i::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint16_t trame=_buf[c4i::Message::Fmt::TRANS];
  uint16_t command=_buf[c4i::Message::Fmt::CMD];

  uint32_t* idb=(uint32_t*) &_buf[c4i::Message::Fmt::PAYLOAD];
  uint8_t* cdb=(uint8_t*) &_buf[c4i::Message::Fmt::PAYLOAD];
  
  _lastGTC=((uint32_t) cdb[0] <<24)|((uint32_t) cdb[1] <<16)|((uint32_t) cdb[2] <<8)|((uint32_t) cdb[3]);
  _lastABCID = ((uint64_t) cdb[4] <<48)|((uint64_t) cdb[5] <<32)|((uint64_t) cdb[6] <<24)|((uint64_t) cdb[7] <<16)|((uint64_t) cdb[8] <<8)|((uint64_t) cdb[9]);
  _lastBCID=((uint32_t) cdb[10] <<24)|((uint32_t) cdb[11] <<16)|((uint32_t) cdb[12] <<8)|((uint32_t) cdb[13]);
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<this->sourceid()<<" Command answer="<<command<<" length="<<length<<" trame id="<<trame<<" buffer length "<<_idx);

#define DEBUGEVENT
#ifdef DEBUGEVENT  
  fprintf(stderr,"Length %d \n==> ",length);
  for (int i=c4i::Message::Fmt::PAYLOAD;i<length;i++)
    {
      fprintf(stderr,"%.2x ",(_buf[i]));
      
      if (i%16==15)
        {
          fprintf(stderr,"\n==> ");
        }
    }
  fprintf(stderr,"\n");
#endif
  uint8_t temp[0x100000];
  uint32_t* itemp=(uint32_t*) temp;
  uint64_t* ltemp=(uint64_t*) temp;
  itemp[0]=_event;

  itemp[1]=_lastGTC;
  ltemp[1]=_lastABCID;
  itemp[4]= _event;
  itemp[5]=this->ipid();
  itemp[6]=length;
  uint32_t idx=28; // 4 x5 int + 1 int64
  uint32_t trbcid=0;
  memcpy(&temp[idx],&_buf[14],length-14);
  idx+=(length-14);
  if (_dsData!=NULL)
    {
      //memcpy((unsigned char*) _dsData->payload(),temp,idx);
      memcpy((unsigned char*) _dsData->payload(),_buf,length);
      _dsData->publish(_lastABCID,_lastGTC,idx);
      if (_event%100==0)
        LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<this->sourceid()<<"Publish  Event="<<_event<<" GTC="<<_lastGTC<<" ABCID="<<_lastABCID<<" size="<<idx);
    }
  _event++;
  return true;
}
void c4i::dataHandler::processBuffer(uint64_t id, uint16_t l,char* bb)
{
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" procesBuffer  "<<std::hex<<id<<std::dec<<" received "<<l<<" stored "<<_idx);
  if ((_idx+l)>MBSIZE)
    {
      _idx=0;
    }

      
  uint8_t temp[0x100000];
  memcpy(&_buf[_idx],bb,l);
  _idx+=l;
 processFullBuffer:
  if (_buf[0]!='(')
    { _idx=0;return;}
  uint16_t* _sBuf= (uint16_t*) &_buf[c4i::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  if (length>MBSIZE)
    {
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<" procesBuffer Size too large "<<std::hex<<id<<std::dec<<" Length "<<length);
      _idx=0;
      return;
    }
  if (_idx>length  && _buf[length-1]!=')')
    {
      //Bad buffer found next a0 00 00 00 00 00 00 29 28
      for (int i=0;i<_idx-9;i++)
	{
	  if (_buf[i]==0xa0 &&
	      _buf[i+1]==0x00 &&
	      _buf[i+2]==0x00 &&
	      _buf[i+3]==0x00 &&
	      _buf[i+4]==0x00 &&
	      _buf[i+5]==0x00 &&
	      _buf[i+6]==0x00 &&
	      _buf[i+7]==0x29 &&
	      _buf[i+8]==0x28)
	    {
	      memcpy(temp,&_buf[i+8],_idx-(i+8));
	      memcpy(_buf,temp,_idx-(i+8));
	      _idx=_idx-(i+8);
	      goto processFullBuffer;
	    }
	}
	
    }
  if (_buf[length-1]==')')
    {
      bool ok=processPacket();
      uint32_t nlen=_idx-length;
      if (nlen>0)
	{
	  memcpy(temp,&_buf[length],nlen);
	  _idx=nlen;
	  memcpy(&_buf[0],temp,nlen);
	  goto processFullBuffer;
	}
      else
	_idx=0;
    }
   
  /*
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Entering procesBuffer "<<std::hex<<id<<std::dec<<" Length "<<l);
 fprintf(stderr,"Length %d \n==> ",_idx);
 for (int i=0;i<l;i++)
    {
      fprintf(stderr,"%.2x ",(bb[i]));
      
      if (i%16==15)
        {
          fprintf(stderr,"\n==> ");
        }
    }
  fprintf(stderr,"\n");
  
for (uint16_t ibx = 0; ibx < l; ibx++)
  {
    int16_t tag = checkBuffer((uint8_t *)&bb[ibx], l - ibx + 1);
    LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"Entering procesBuffer "<<std::hex<<id<<std::dec<<ibx<<" Tag "<<tag);
    if (tag > 0)
    {
      bool ok = processPacket();
      if (ok)
      {
        _idx = 0;
	//ibx=tag-1;
	//continue;
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
        LOG4CXX_DEBUG(_logFeb, __PRETTY_FUNCTION__ << this->difId() << "StructError Tag=" << tag << " ibx=" << ibx << " _idx=" << _idx);
      }
    }
    _buf[_idx] = bb[ibx];
    _idx++;
  }
 return;

  */














  
}
