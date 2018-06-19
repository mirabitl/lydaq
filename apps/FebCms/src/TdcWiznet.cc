#include "TdcWiznet.hh"
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

int16_t lydaq::TdcWiznet::checkBuffer(uint8_t* b,uint32_t maxidx)
{
  uint32_t* _lBuf= (uint32_t*) b;
  uint16_t* _sBuf= (uint16_t*) b;

  uint32_t elen=0;
 if (ntohl(_lBuf[0])!=0xcafedade && ntohl(_lBuf[0])!=0xcafebabe)
   return 0;
 // Check TAG coherency
 if (ntohl(_lBuf[0])==0xcafedade)
   {
     elen=18; // Header
     uint32_t* leb=(uint32_t*) &b[elen-4];
     if (elen>maxidx)
       {
	 fprintf(stderr,"CheckBuf header:Not enough data ELEN %d MAXID %d \n",elen,maxidx);
	 return -5;
       }
     if (ntohl(leb[0])==0xdadecafe)
       return elen;
     else
       {
	 fprintf(stderr,"CheckBuf header :Missing CAFEDADE end tag \n");
	 return -1;
       }
   }
 else
   {
     if (ntohl(_lBuf[0])==0xcafebabe)
       {
	 elen=ntohs(_sBuf[5])*CHBYTES+16; //Channels
	 if (elen<16 || elen > 208)
	   {
	     fprintf(stderr,"CheckBuf:Wrong size %d \n",elen);
	     return -2;
	   }
	 if (elen>maxidx)
	   {
	     fprintf(stderr,"CheckBuf:Not enough data ELEN %d MAXID %d \n",elen,maxidx);
	     return -3;
	   }
	 uint32_t* leb=(uint32_t*) &b[elen-4];
	 if (ntohl(leb[0])==0xbabecafe)
	   return elen;
	 else
	   {
	     fprintf(stderr,"CheckBuf:Missing CAFEBABE end tag \n");
	     return -4;
	   }
	 
       }
     else
       return 0;
   }
}


lydaq::TdcWiznet::TdcWiznet(uint32_t adr) : _ntrg(0),_adr(adr),_dsData(NULL),_triggerId(0)
{
  this->clear();
  _id=(adr>>16)&0xFFFF; // Last byte of IP address
  _detid=TDC_VERSION&0xFF;
  this->clear();

}
void lydaq::TdcWiznet::connect(zmq::context_t* c,std::string dest)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new zdaq::zmPusher(c,_detid,_id);
  _dsData->connect(dest);
}
void lydaq::TdcWiznet::clear()
{
  _nProcessed=0;
  _lastGTC=0;
  _lastABCID=0;
  _event=0;
  _chlines=0;
}






bool lydaq::TdcWiznet::processPacket()
{
 uint32_t* _lBuf= (uint32_t*) &_buf[0];
 uint16_t* _sBuf= (uint16_t*) &_buf[0];

 // // Check TAG coherency
 // if (ntohl(_lBuf[0])==0xcafedade)
 //   {
 //     _expectedLength=18; // Header
 //   }
 // else
 //   {
 //     _expectedLength=ntohs(_sBuf[5])*CHBYTES+16; //Channels
 //   }
 // fprintf(stderr,"Current length %d  ExpectedLength %d \n",_idx,_expectedLength);
 // // Exit if bad buffer structure
 // // if (_idx!=_expectedLength) return false;
 // if (_idx!=_expectedLength) return false;

 int16_t tag=checkBuffer(_buf,_idx);
 if (tag<0) return false;
 
 if (ntohl(_lBuf[0])==0xcafedade)
   {
     if (_lastABCID!=0)
       {
	 // Write Event
	 fprintf(stderr,"Writing completed Event %d GTC %d ABCID %llu  Lines %d written\n",_event,_lastGTC,_lastABCID,_chlines);
	 // To be done
	 this->processEventTdc();
	 //fprintf(stderr,"Event send \n");
	 // Reset lines number
	 _chlines=0;
       }

     // Find new gtc and absolute bcid
     uint32_t gtc= (_buf[7]|(_buf[6]<<8)|(_buf[5]<<16)|(_buf[4]<<24));
     uint64_t abcid=((uint64_t) _buf[13]|((uint64_t) _buf[12]<<8)|((uint64_t) _buf[11]<<16)|((uint64_t) _buf[10]<<24)|((uint64_t) _buf[9]<<32));
     if (abcid==_lastABCID)
       {
	 fprintf(stderr," ABCID HEADER ERROR %lld last %lld \n",abcid,_lastABCID);
       }
     if (gtc==_lastGTC)
       {
	 fprintf(stderr," GTC HEADER ERROR  %d last %d \n",gtc,_lastGTC);
       }
     _nProcessed++;
     _event++;
     _lastGTC=gtc;
     _lastABCID=abcid;
     DEBUG_PRINT(" New Event Header  %d Packets %d GTC %d ABCID %llu Size %d\n",_event,_nProcessed,gtc,abcid,_idx);

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
     return true;
   }

 // Channel packet processing
 _nProcessed++;
 
 uint16_t channel = ntohs(_sBuf[2]);
 uint16_t* tmp= (uint16_t*) &_buf[7];
 uint32_t gtc= (_buf[9]|(_buf[8]<<8)|(_buf[7]<<16)|(_buf[6]<<24));
 uint16_t nlines= ntohs(_sBuf[5]);
 fprintf(stderr,"%d packet %x # %d for channel %d with  lines %d and byte size %d \n",_nProcessed,ntohl(_lBuf[0]),gtc,channel,nlines,_idx);
 //  printf ("packet %x # %d with payload length %d  and tottal size %d \n",(_lBuf[0]),(_lBuf[1]),(_lBuf[2]),_idx);


 uint8_t* cl= (uint8_t*) &_buf[12];
 uint8_t* cdestl=(uint8_t*) &_linesbuf[_chlines*CHBYTES];
 memcpy(cdestl,cl,nlines*CHBYTES);
 _chlines+=nlines;
 #undef DEBUGLINES
#ifdef DEBUGLINES
 for (int i=0;i<nlines;i++)
   {
     for (int j=0;j<CHBYTES;j++)
       printf("%.2x ",(cl[i*CHBYTES+j]));
     printf("\n");

   } 
#endif 

 
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
 // fprintf(stderr,"packet processed \n");

 uint16_t expectedSize=16+nlines*CHBYTES;
 if (_idx>(expectedSize+1))
   {
     char temp[0x10000];
     uint16_t remain=_idx-expectedSize;
     memcpy(temp,&_buf[expectedSize],remain);
     memcpy(_buf,temp,remain);
     _idx=remain;
     this->processPacket();
   }
 return true;
 
}
void lydaq::TdcWiznet::processBuffer(uint64_t id,uint16_t l,char* b)
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
	    fprintf(stderr,"StructError %d ibx %d _idx %d \n",tag,ibx,_idx);
	  }
      _buf[_idx]=b[ibx];
      _idx++;
    }
  



  DEBUG_PRINT("Exiting procesBuffer %llx %d  IDX %d \n",id,l,_idx);
  return;
  
  uint16_t* sptr=(uint16_t*) b;
  uint16_t lines=l/2;


  int header_len=10;
  for (uint16_t ibx=0;ibx<l;ibx++)
    {
      uint16_t* _sBuf= (uint16_t*) &b[ibx];
      uint32_t*_lBuf= (uint32_t*) &b[ibx];
      uint8_t* _cBuf= (uint8_t*) &b[ibx];

      if (ntohl(_lBuf[0]) ==0xcafebabe || ntohl(_lBuf[0]) ==0xcafedade  )
	{
	  fprintf(stderr,"Header Found %x %d  expected length %d  current index %d \n", ntohl(_lBuf[0]),ibx,_expectedLength,_idx);
	  if (_expectedLength>0 )
	    if (this->processPacket())
	      {
		_idx=0;
		if (ntohl(_lBuf[0]) ==0xcafebabe)
		  _expectedLength=ntohs(_sBuf[5])*CHBYTES+16;
		else
		  _expectedLength=18;
	      }
	    else
	      fprintf(stderr,"Packet not processed \n");
	  if (_expectedLength==0)
	    {
	      _idx=0;
	      if (ntohl(_lBuf[0]) ==0xcafebabe)
		_expectedLength=ntohs(_sBuf[5])*CHBYTES+16;
	      else
		_expectedLength=18;
	    }
	  //printf("Starting packet %d of %d bytes \n",_packetNb,_expectedLength);
	  _buf[_idx]= b[ibx];
	  _idx++;
	}
      else
	{
	  _buf[_idx]= b[ibx];
	  _idx++;
	}
    }
  //  if (l==16384) purgeBuffer();
  fprintf(stderr,"Exiting procesBuffer %llx %d \n",id,l);  

}

void lydaq::TdcWiznet::purgeBuffer()
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
void lydaq::TdcWiznet::processSlc(uint64_t id,uint16_t l,char* b)
{
  fprintf(stderr,"Entering procesSLC %x %d \n",id,l);  
  uint16_t* sptr=(uint16_t*) b;
  uint16_t lines=l/2;
      
  fprintf(stderr,"\t \t ==> SLC %d bytes |%x|%x|%x|%x|%x| \n",l,ntohs(sptr[0]),ntohs(sptr[1]),ntohs(sptr[2]),ntohs(sptr[3]),ntohs(sptr[4]));
  if (l<800)
    {
      fprintf(stderr,"\n\t ");

      for (int i=0;i<l;i++)
	{
	  fprintf(stderr,"%.2x ",(uint8_t) (b[i]));
	       
	  if (i%16==15)
	    {
	      fprintf(stderr,"\n\t ");
	    }
	}
      fprintf(stderr,"\n");

    }
  fprintf(stderr,"Exiting processSLC %x %d \n",id,l);  
}



void lydaq::TdcWiznet::processEventTdc()
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
      if (_event%100==0)
	INFO_PRINT("Publish %x %d GTC %d %llx channels %d size %d \n",_adr,_event,_lastGTC,_lastABCID,_chlines,idx);
    }
}
