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


  uint8_t* cdb=(uint8_t*) &_buf[c4i::Message::Fmt::PAYLOAD+2];
  
  _lastGTC=((uint32_t) cdb[2] <<16)|((uint32_t) cdb[3] <<8)|((uint32_t) cdb[4]);
  _lastABCID = ((uint64_t) cdb[5] <<48)|((uint64_t) cdb[6] <<32)|((uint64_t) cdb[7] <<24)|((uint64_t) cdb[8] <<16)|((uint64_t) cdb[9] <<8)|((uint64_t) cdb[10]);
  _lastBCID=((uint32_t) cdb[11] <<16)|((uint32_t) cdb[12] <<8)|((uint32_t) cdb[13]);
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<this->sourceid()<<" Command answer="<<command<<" length="<<length<<" trame id="<<trame<<" buffer length "<<_idx<<" GTC" <<_lastGTC<<" ABCID "<<_lastABCID<<" Last BCID "<<_lastBCID);

#define DEBUGEVENTN
#ifdef DEBUGEVENT  
  fprintf(stderr,"Length %d \n==> ",length);
  for (int i=0;i<c4i::Message::Fmt::PAYLOAD+256;i++)
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
      LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<this->sourceid()<<"Publishing  Event="<<_event<<" GTC="<<_lastGTC<<" ABCID="<<_lastABCID<<" size="<<idx);
      memcpy((unsigned char*) _dsData->payload(),_buf,length);
      _dsData->publish(_lastABCID,_lastGTC,idx);
      //if (_event%100==0)
      LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<this->sourceid()<<"Published  Event="<<_event<<" GTC="<<_lastGTC<<" ABCID="<<_lastABCID<<" size="<<idx);
    }
  _event++;
  return true;
}
void c4i::dataHandler::processBuffer(uint64_t id, uint16_t l,char* bb)
{
  LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<" procesBuffer  "<<std::hex<<id<<std::dec<<" received "<<l<<" stored "<<_idx);
  if ((_idx+l)>MBSIZE)
    {
      LOG4CXX_WARN(_logFeb,__PRETTY_FUNCTION__<<" Resetting the buffer ");
      _idx=0;
    }
  
      
  uint8_t temp[0x100000];
  memcpy(&_buf[_idx],bb,l);
  _idx+=l;
 processFullBuffer:
  // Bad start of buffer
  if (_buf[0]!='(')
    {
      LOG4CXX_WARN(_logFeb,__PRETTY_FUNCTION__<<" Invalid start of buffer "<<std::hex<<_buf[0]<<std::dec);
      _idx=0;memset(_buf,0,MBSIZE);return;
    }
  
  uint16_t* _sBuf= (uint16_t*) &_buf[c4i::Message::Fmt::LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header

  // Invalid length
  if (length>MBSIZE)
    {
      LOG4CXX_WARN(_logFeb,__PRETTY_FUNCTION__<<" procesBuffer Size too large "<<std::hex<<id<<std::dec<<" Length "<<length);
      memset(_buf,0,MBSIZE);
      _idx=0;
      return;
    }
  //  Data corrupted
  if (_idx>length  && _buf[length-1]!=')')
    {
      LOG4CXX_WARN(_logFeb,__PRETTY_FUNCTION__<<"Corrupted buffer "<<std::hex<<id<<std::dec<<" Length "<<length);
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
  // End of buffer
  if (_buf[length-1]==')')
    {
      bool ok=processPacket();
      int32_t nlen=_idx-length;
      LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"Packet processed  "<<std::hex<<id<<std::dec<<" remaining Length "<<_idx<<" "<<length);
      if (nlen>0)
	{
	  memcpy(temp,&_buf[length],nlen);
	  _idx=nlen;
	  memset(_buf,0,MBSIZE);
	  memcpy(&_buf[0],temp,nlen);
	  goto processFullBuffer;
	}
      else	
	{_idx=0;memset(_buf,0,MBSIZE);}
    }
   
  
}
