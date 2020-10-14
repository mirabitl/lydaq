#include "C3iMpi.hh"
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
#define CHBYTES 6
using namespace lydaq;
using namespace zdaq;

int16_t lydaq::C3iMpi::checkBuffer(uint8_t* b,uint32_t maxidx)
{

  uint32_t elen=0;
  if (b[0]!='(')  return -2;
 // Check TAG coherency
 if (b[0]=='(')
   {
     uint16_t* _sBuf= (uint16_t*) &b[1];
     elen=ntohs(_sBuf[0]); // Header

     LOG4CXX_WARN(_logFeb,__PRETTY_FUNCTION__<<"CheckBuf header ELEN "<<elen<<" MAXID "<<maxidx);
     //fprintf(stderr,"d %d %c\n",__LINE__,b[elen-1]);
     if (elen>maxidx)
       {
	 LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"CheckBuf header:Not enough data ELEN "<<elen<<" MAXID "<<maxidx);
	 return -1;
       }
     if (b[elen-1]==')')
       {

       return elen;
       }
     else
       {
	 LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"CheckBuf header :Missing  end tag ");
	 return -1;
       }
   }
 return -3; 
}


lydaq::C3iMpi::C3iMpi(uint32_t adr) : _ntrg(0),_adr(adr),_dsData(NULL),_triggerId(0),_idx(0)
{
   // initialise answer storage
  for (int i=0;i<255;i++)
    {
      uint8_t* b=new uint8_t[0x4000];
      std::pair<uint8_t,uint8_t*> p(i,b);
      _answ.insert(p);
    }
  this->clear();
  _id=(adr>>16)&0xFFFF; // Last byte of IP address
  _detid=C3I_VERSION&0xFF;
  memset(_bufReg,0,MBSIZE);
  memset(_bufSlc,0,MBSIZE);
  memset(_bufData,0,MBSIZE);
  _buf=0;
  _idx=0;

}
void lydaq::C3iMpi::connect(zmq::context_t* c,std::string dest)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new zdaq::zmSender(c,_detid,_id);
  _dsData->connect(dest);
}


void lydaq::C3iMpi::autoRegister(zmq::context_t* c,Json::Value config,std::string appname,std::string portname)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new zdaq::zmSender(c,_detid,_id);
  _dsData->autoDiscover(config,appname,portname);//
      //for (uint32_t i=0;i<_mStream.size();i++)
      //	ds->connect(_mStream[i]);
  _dsData->collectorRegister();

}
void lydaq::C3iMpi::clear()
{
  _nProcessed=0;
  _lastGTC=0;
  _lastABCID=0;
  _event=0;
  _chlines=0;


  for (int i=0;i<255;i++)
    {
      memset(_answ[i],0,0x4000);
    }
}




bool lydaq::C3iMpi::isHardrocData(){ return ((_ipid&0xFFFF)==lydaq::c3i::MpiInterface::PORT::DATA);}
bool lydaq::C3iMpi::isSlcData(){ return ((_ipid&0xFFFF)==lydaq::c3i::MpiInterface::PORT::SLC);}
bool lydaq::C3iMpi::isRegisterData(){ return ((_ipid&0xFFFF)==lydaq::c3i::MpiInterface::PORT::REGISTER);}

bool lydaq::C3iMpi::processPacket()
{
 uint32_t* _lBuf= (uint32_t*) &_buf[1];
 uint16_t* _sBuf= (uint16_t*) &_buf[1];


 int16_t tag=1;

 tag=checkBuffer(_buf,_idx);
 LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"After checkBuffer "<<tag<<" CMD "<<(int) _buf[4]);
 if (tag<0) return false;
 
 if (_buf[0]=='(')
   {
     if (isHardrocData())
       {
	 _lastGTC=((uint32_t) _buf[5] <<24)|((uint32_t) _buf[6] <<16)|((uint32_t) _buf[7] <<8)|((uint32_t) _buf[8]);
	 _lastABCID = ((uint64_t) _buf[9] <<48)|((uint64_t) _buf[10] <<32)|((uint64_t) _buf[11] <<24)|((uint64_t) _buf[12] <<16)|((uint64_t) _buf[13] <<8)|((uint64_t) _buf[14]);

	 // Write Event
	 _chlines=(ntohs(_sBuf[0])-16)/20;
	 if (_event%100==0)
	   LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Writing completed Event"<<_event<<" GTC "<<_lastGTC<<" ABCID "<<_lastABCID<<" Lines "<<_chlines<<" ID "<<std::hex<<_id<<std::dec);
	 // To be done
	 this->processEventData();
	 //fprintf(stderr,"Event send \n");
	 // Reset lines number
	 _chlines=0;
	 //_lastABCID=_lBuf[0];
	 //_lastGTC++;
       }

     if (isSlcData())
       {
	 // Write Event
	 LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Processing Sensor data"<<_event<<" GTC "<<_lastGTC<<" ABCID "<<_lastABCID<<" Lines "<<_chlines<<" ID "<<_id);
	 // To be done
	 this->processSlcData();
	     
       }
     if (isRegisterData())
       {
	 // Write Event
	 LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<"Processing Register data");
	 // To be done
	 this->processRegisterData();
	 
       }

   }
   

     

 //temporaire

 return true;

 
}
void lydaq::C3iMpi::processBuffer(uint64_t id,uint16_t l,char* bb)
{

  _ipid=id;
  if (this->isHardrocData()) _buf=_bufData;
  if (this->isSlcData()) _buf=_bufSlc;
  if (this->isRegisterData()) _buf=_bufReg;
  if (_buf==0)
    {
    LOG4CXX_ERROR(_logFeb,__PRETTY_FUNCTION__<<" Unknown socket "<<id);
    return;
    }
  LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<"Entering procesBuffer "<<std::hex<<id<<std::dec<<" Length "<<l);
  //if (l>16) getchar();
  //memcpy(_b,bb,l);
  memcpy(&_buf[_idx],bb,l);
  _idx+=l;
#define DEBUGPACKET
#ifdef DEBUGPACKET
  fprintf(stderr,"\n DEBUG PACKET IDX %d L %d  ID %lx \n",_idx,l,id);
     for (int i=0;i<_idx;i++)
       {
	 fprintf(stderr,"%.2x ",((uint8_t) _buf[i]));
	 
	 if (i%16==15)
	   {
	     fprintf(stderr,"\n");
	   }
       }
     fprintf(stderr,"\n END PACKET \n");
#endif
     uint32_t a=l;
 checkpoint:
     int16_t tag = checkBuffer(_buf,_idx);
     if (tag>0)
       {
	 _ipid=id;
	 bool ok=processPacket();
	 if (tag<_idx)
	   {
	     memcpy(_b,&_buf[tag],_idx-tag);
	     _idx=_idx-tag;
	     memset(_buf,0,MBSIZE);
	     memcpy(_buf,_b,_idx);
	     goto checkpoint;
	   }
	 if (tag==_idx) _idx=0;
	   
	  return;
       }

  

     //DEBUG_PRINT("Exiting procesBuffer %llx %d  IDX %d \n",id,l,_idx);
  return;
  

}

void lydaq::C3iMpi::purgeBuffer()
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









 void lydaq::C3iMpi::processRegisterData()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[C3I_FMT_LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[C3I_FMT_TRANS];
  _sBuf=(uint16_t*) &_buf[C3I_FMT_PAYLOAD];
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
void lydaq::C3iMpi::processSlcData()
{
  uint16_t* _sBuf= (uint16_t*) &_buf[C3I_FMT_LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint8_t transaction=_buf[C3I_FMT_TRANS];

  
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

 void lydaq::C3iMpi::processEventData()
{

  uint16_t* _sBuf= (uint16_t*) &_buf[C3I_FMT_LEN];
  uint16_t length=ntohs(_sBuf[0]); // Header
  uint16_t trame=_buf[C3I_FMT_TRANS];
  uint16_t command=_buf[C3I_FMT_CMD];

  uint32_t* idb=(uint32_t*) &_buf[C3I_FMT_PAYLOAD];
  uint8_t* cdb=(uint8_t*) &_buf[C3I_FMT_PAYLOAD];
  
  _lastGTC=((uint32_t) cdb[0] <<24)|((uint32_t) cdb[1] <<16)|((uint32_t) cdb[2] <<8)|((uint32_t) cdb[3]);
  _lastABCID = ((uint64_t) cdb[4] <<48)|((uint64_t) cdb[5] <<32)|((uint64_t) cdb[6] <<24)|((uint64_t) cdb[7] <<16)|((uint64_t) cdb[8] <<8)|((uint64_t) cdb[9]);
  _lastBCID=((uint32_t) cdb[10] <<24)|((uint32_t) cdb[11] <<16)|((uint32_t) cdb[12] <<8)|((uint32_t) cdb[13]);
  LOG4CXX_DEBUG(_logFeb,__PRETTY_FUNCTION__<<_id<<" Command answer="<<command<<" length="<<length<<" trame id="<<trame<<" buffer length "<<_idx);

#define DEBUGEVENTN
#ifdef DEBUGEVENT  
  fprintf(stderr,"Length %d \n==> ",_idx);
  for (int i=C3I_FMT_PAYLOAD;i<_idx;i++)
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
  itemp[5]=_adr;
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
	LOG4CXX_INFO(_logFeb,__PRETTY_FUNCTION__<<_id<<"Publish  Event="<<_event<<" GTC="<<_lastGTC<<" ABCID="<<_lastABCID<<" size="<<idx);
    }
  _event++;
}

