#include "TdcFpga.hh"
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

using namespace lydaq;
using namespace zdaq;

lydaq::TdcFpga::TdcFpga(uint32_t m,uint32_t adr) :_abcid(0),_gtc(0),_mezzanine(m),_lastGtc(0),_startIdx(0),_event(0),_adr(adr),_dsData(NULL),_triggerId(16)
{
  _channels.clear();
  _nBuffers=0;
  _id=(adr>>16)&0xFFFF; // Last byte of IP address
  _detid=TDC_VERSION;

}
void lydaq::TdcFpga::connect(zmq::context_t* c,std::string dest)
{
  if (_dsData!=NULL)
    delete _dsData;
  _dsData = new zdaq::zmPusher(c,_detid,_id);
  _dsData->connect(dest);
}
void lydaq::TdcFpga::clear()
{
  _gtc=0;
  _event=0;
  _lastGtc=0;
}
void lydaq::TdcFpga::addChannels(uint8_t* buf,uint32_t size_buf)
{
  _nBuffers++;
  uint8_t ll=8;
  uint64_t rabcid=buf[ll+ll-1]|((uint64_t) buf[ll+ll-2]<<8)|((uint64_t) buf[ll+ll-3]<<16)|((uint64_t) buf[ll+ll-4]<<24)|((uint64_t) buf[ll+ll-5]<<32)|((uint64_t)buf[ll+ll-6]<<40);
  uint32_t rgtc= buf[ll+1]|((uint32_t) buf[ll]<<8);
  if (rgtc==_lastGtc)
    {
      _abcid=rabcid;
      _gtc=rgtc;
      uint32_t nlines=buf[7];
      memcpy(&_buf[_startIdx],buf,size_buf);
      for (int i=1;i<nlines;i++)
	{     
	  uint8_t *fr=(uint8_t*) &_buf[_startIdx+ll+i*ll];
	  TdcChannel tc(fr);
	  _channels.push_back(tc);
	}
      _startIdx+=size_buf;
      return;
    }
  else
    {
      _event++;
      this->processEventTdc();
      _channels.clear();
      _startIdx=0;
      _nBuffers=0;
      _lastGtc=rgtc;
      this->addChannels(buf,size_buf);
      return;
    }
}
void lydaq::TdcFpga::processEventTdc()
{
  uint8_t temp[0x100000];
  uint32_t* itemp=(uint32_t*) temp;
  uint64_t* ltemp=(uint64_t*) temp;
  itemp[0]=_event;

  itemp[1]=_gtc;
  ltemp[1]=_abcid;
  itemp[4]=_mezzanine;
  itemp[5]=_adr;
  itemp[6]=_channels.size();
  uint32_t idx=28;
  uint32_t trbcid=0;
#define DUMPTRIGGER
#ifdef DUMPTRIGGER
  for (auto x:_channels)
    {
      if (x.channel()==_triggerId)
	{
	  trbcid=x.bcid();
	  printf("Trigger %x %d %d %ld %d %x \n ", _adr,_mezzanine,_gtc,_abcid,_channels.size(),trbcid);
	  //  break;
	}
    }
  if (trbcid>0)
  {
  printf("Trigger %x %d %d %ld %d \n ", _adr,_mezzanine,_gtc,_abcid,_channels.size());
  std::stringstream ss;
        ss<<boost::format("Trigger %x %d %d %ld %d \n ") % _adr % _mezzanine % _gtc % _abcid % _channels.size();
  int nch=0;
  for (auto x:_channels)
  	{
   ss<<boost::format("\t %d %d %f ") % (int) x.channel() % (int) x.bcid() % x.tdcTime();
  if (x.channel()!=16 && (x.bcid()>(trbcid-4) && x.bcid()<(trbcid+4)))
   {
   ss<<"---> found\n";
   }
  else
   ss<<"\n";
  nch++;
  if (nch>40) {ss<<" and more.. \n";break;}
  }
  std::cout<<ss.str()<<std::flush;

  }
#endif
  if (_dsData!=NULL)
    {
      for (vector<TdcChannel>::iterator it=_channels.begin();it!=_channels.end();it++)
	{
	  memcpy(&temp[idx],it->frame(),it->length());
	  idx+=8;
      
	}
 
      memcpy((unsigned char*) _dsData->payload(),temp,idx);
      _dsData->publish(_abcid,_gtc,idx);
    }
  if (_event%100==0)
    std::cout<<_mezzanine<<" "<<_event<<" "<<_gtc<<" "<<_abcid<<" "<<_channels.size()<<std::endl<<std::flush;
}
