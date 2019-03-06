#include "GricMpi.hh"
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
#define CHBYTES 6
using namespace lydaq;
using namespace zdaq;

int16_t lydaq::GricMpi::checkBuffer(uint8_t* b,uint32_t maxidx)
{
  uint32_t* _lBuf= (uint32_t*) b;
  uint16_t* _sBuf= (uint16_t*) b;
  uint32_t elen=0;
  if (b[0]!='(')  return 0;
 // Check TAG coherency
 if (b[0]=='(')
   {

     uint16_t* _sBuf= (uint16_t*) &b[1];
     elen=_sBuf[0]; // Header
     uint32_t* leb=(uint32_t*) &b[elen-4];
     if (elen>maxidx)
       {
	 LOG4CXX_WARN(_logFeb,__PRETTY_FUNCTION__<<"CheckBuf header:Not enough data ELEN "<<elen<<" MAXID "<<maxidx);
	 return -5;
       }
     if (b[elen-1]==')')
       return elen;
     else
       {
	 LOG4CXX_WARN(_logFeb,__PRETTY_FUNCTION__<<"CheckBuf header :Missing CAFEDADE end tag ");
	 return -1;
       }
   }
 
}


lydaq::GricMpi::GricMpi(uint32_t adr) : _ntrg(0),_adr(adr),_dsData(NULL),_triggerId(0)
{
  this->clear();
  _id=(adr>>16)&0xFFFF; // Last byte of IP address
  _detid=GRIC_VERSION&0xFF;
  this->clear();

}
void lydaq::GricMpi::connect(zmq::context_t* c,std::string dest)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new zdaq::zmPusher(c,_detid,_id);
  _dsData->connect(dest);
}
void lydaq::GricMpi::clear()
{
  _nProcessed=0;
  _lastGTC=0;
  _lastABCID=0;
  _event=0;
  _chlines=0;
}




bool lydaq::GricMpi::isHardrocData(){ return (_buf[4]==0xDD);}
bool lydaq::GricMpi::isSensorData(){ return (_buf[4]==0xDE);}

bool lydaq::GricMpi::processPacket()
{
 uint32_t* _lBuf= (uint32_t*) &_buf[1];
 uint16_t* _sBuf= (uint16_t*) &_buf[1];


 int16_t tag=checkBuffer(_buf,_idx);
 if (tag<0) return false;
 
 if (_buf[0]=='(')
   {
     if (_lastABCID!=0 && isHardrocData())
       {
	 // Write Event
	 LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Writing completed Event"<<_event<<" GTC "<<_lastGTC<<" ABCID "<<_lastABCID<<" Lines "<<_chlines<<" ID "<<_id);
	 // To be done
	 this->processEventGric();
	 //fprintf(stderr,"Event send \n");
	 // Reset lines number
	 _chlines=0;
	 _lastABCID=_lBuf[0];
       }

     if (isSensorData())
       {
	 // Write Event
	 LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Processing Sensor data"<<_event<<" GTC "<<_lastGTC<<" ABCID "<<_lastABCID<<" Lines "<<_chlines<<" ID "<<_id);
	 // To be done
	 this->processSensorGric();
	 
       }
     if (isCommandData())
       {
	 // Write Event
	 LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Processing Command answer"<<_event<<" GTC "<<_lastGTC<<" ABCID "<<_lastABCID<<" Lines "<<_chlines<<" ID "<<_id);
	 // To be done
	 this->processCommandGric();
	 
       }
   }

     
#define DEBUGPACKET
#ifdef DEBUGPACKET
     printf("\n==> ");
     for (int i=0;i<_idx;i++)
       {
	 printf("%.2x ",(_buf[i]));
	 
	 if (i%16==15)
	   {
	     printf("\n==> ");
	   }
       }
     printf("\n");
#endif
     //temporaire

     return true;

 
}
void lydaq::GricMpi::processBuffer(uint64_t id,uint16_t l,char* b)
{
  DEBUG_PRINT("Entering procesBuffer %llx %d  IDX %d \n",id,l,_idx);
  for (uint16_t ibx=0;ibx<l;ibx++)
    {
      int16_t tag = checkBuffer((uint8_t*) &b[ibx],l-ibx+1);
      if (tag>0)
	{
	  bool ok=processPacket();
	  if (ok)
	    {
	      _idx=0;
	    }
	}
      else
	if (tag<0)
	  {
	    LOG4CXX_WARN(_logFeb,__PRETTY_FUNCTION__<<_id<<"StructError Tag="<<tag<<" ibx="<<ibx<< " _idx="<<_idx);
	  }
      _buf[_idx]=b[ibx];
      _idx++;
    }
  



  DEBUG_PRINT("Exiting procesBuffer %llx %d  IDX %d \n",id,l,_idx);
  return;
  

}

void lydaq::GricMpi::purgeBuffer()
{
  fprintf(stderr,"Entering PURGEBUFFER %d \n",_idx);

  uint8_t cbuf[0x1000000];
  for (uint16_t ibx=0;ibx<_idx;ibx++)
    {
      uint16_t* _sBuf= (uint16_t*) &_buf[ibx];
      uint32_t*_lBuf= (uint32_t*) &_buf[ibx];
      uint8_t* _cBuf= (uint8_t*) &_buf[ibx];

      if (ntohl(_lBuf[0]) ==0xcafebabe || ntohl(_lBuf[0]) ==0xcafedade  )
	{
	  fprintf(stderr,"Purge Header Found \n");
	  if (_expectedLength>0 )
	    {

	      if (this->processPacket())
	      {

		if (ntohl(_lBuf[0]) ==0xcafebabe)
		  _expectedLength=ntohs(_sBuf[5])*CHBYTES+16;
		else
		  _expectedLength=18;

		memcpy(cbuf,&_buf[ibx],_idx-ibx);
		_idx=_idx-ibx;
		memcpy(&_buf[0],cbuf,_idx);
		break;
		
	      }
	    }
	}

  
    }
}









 void lydaq::GricMpi::processCommandGric()
{
  uint16_t length=_buf[1];
  uint16_t trame=_buf[3];
  uint16_t command=_buf[4];
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<_id<<"Command answer="<<command<<" length="<<length<<" trame id="<<trame);
  fprintf(stderr,"\n==> ");
     for (int i=4;i<_idx;i++)
       {
	 fprintf(stderr,"%.2x ",(_buf[i]));
	 
	 if (i%16==15)
	   {
	     fprintf(stderr,"\n==> ");
	   }
       }
     fprintf(stderr,"\n");
}
void lydaq::GricMpi::processSensorGric()
{
  uint16_t length=_buf[1];
  uint16_t trame=_buf[3];
  uint16_t command=_buf[4];
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<_id<<"Command answer="<<command<<" length="<<length<<" trame id="<<trame);
  fprintf(stderr,"\n==> ");
     for (int i=4;i<_idx;i++)
       {
	 fprintf(stderr,"%.2x ",(_buf[i]));
	 
	 if (i%16==15)
	   {
	     fprintf(stderr,"\n==> ");
	   }
       }
     fprintf(stderr,"\n");
}

 void lydaq::GricMpi::processEventGric()
{
  uint16_t length=_buf[1];
  uint16_t trame=_buf[3];
  uint16_t command=_buf[4];
  LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<_id<<"Command answer="<<command<<" length="<<length<<" trame id="<<trame);
  fprintf(stderr,"\n==> ");
     for (int i=4;i<_idx;i++)
       {
	 fprintf(stderr,"%.2x ",(_buf[i]));
	 
	 if (i%16==15)
	   {
	     fprintf(stderr,"\n==> ");
	   }
       }
     fprintf(stderr,"\n");
}

 /*
void lydaq::GricMpi::processEventGric()
{
  uint8_t temp[0x100000];
  uint32_t* itemp=(uint32_t*) temp;
  uint64_t* ltemp=(uint64_t*) temp;
  itemp[0]=_event;

  itemp[1]=_lastGTC;
  ltemp[1]=_lastABCID;
  itemp[4]= _event;
  itemp[5]=_adr;
  itemp[6]=_chlines;
  uint32_t idx=28; // 4 x5 int + 1 int64
  uint32_t trbcid=0;
  memcpy(&temp[idx],_linesbuf,_chlines*CHBYTES);
  idx+=(_chlines*CHBYTES);
  if (_dsData!=NULL)
    {
      memcpy((unsigned char*) _dsData->payload(),temp,idx);
      _dsData->publish(_lastABCID,_lastGTC,idx);
      //if (_event%100==0)
      LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<_id<<"Publish  Event="<<_event<<" GTC="<<_lastGTC<<" ABCID="<<_lastABCID<<" Lines="<<_chlines<<" size="<<idx);
    }
}
*/
