#include "WiznetTest.hh"
#include "TdcConfigAccess.hh"
#include <iostream>
lydaq::WiznetTest::WiznetTest(std::string address,uint16_t portslc,uint16_t porttdc)
  :  zdaq::baseApplication("WIZNET"),_address(address),_portslc(portslc),_porttdc(porttdc),_idx(0),_sBuf(0),_lBuf(0),_expectedLength(0)
{
  _wiznet= new lydaq::WiznetInterface();
  _msg=new lydaq::WiznetMessage();
  memset(_buf,0,32*1024);
  this->fsm()->addCommand("START",boost::bind(&lydaq::WiznetTest::c_start,this,_1,_2));
  this->fsm()->addCommand("CONFIGURE",boost::bind(&lydaq::WiznetTest::c_configure,this,_1,_2));
  this->fsm()->addCommand("STOP",boost::bind(&lydaq::WiznetTest::c_stop,this,_1,_2));
  this->fsm()->addCommand("STATUS",boost::bind(&lydaq::WiznetTest::c_status,this,_1,_2));
  this->fsm()->addCommand("PACKET",boost::bind(&lydaq::WiznetTest::c_packet,this,_1,_2));
  this->fsm()->addCommand("MODE",boost::bind(&lydaq::WiznetTest::c_mode,this,_1,_2));

  this->fsm()->start(30000);
}
void lydaq::WiznetTest::c_start(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"Start CMD called ");
  uint32_t nc=atol(request.get("value","32").c_str());
  this->start(nc);
  response["STATUS"]="CA MARCHE !";
}
void lydaq::WiznetTest::c_mode(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"MODE CMD called ");
  uint32_t nc=atol(request.get("value","0").c_str());
  this->mode(nc);
  response["STATUS"]="CA MARCHE !";
}
void lydaq::WiznetTest::c_configure(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"Configure CMD called ");
  //  uint32_t nc=atol(request.get("value","32").c_str());
  //this->start(nc);
  this->configure("/home/acqilc/PR2_TDC7.json");
  response["STATUS"]="CA MARCHE !";
}
void lydaq::WiznetTest::c_stop(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"Stop CMD called ");
  this->stop();
  response["STATUS"]="CA MARCHE Et en plus on s'arrete";
}
void lydaq::WiznetTest::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"Status CMD called ");
  this->stop();
  Json::Value jrep;
  jrep["packets"]=_packetNb;
  jrep["packetLength"]=_expectedLength;
  
  response["STATUS"]=jrep;
}
void lydaq::WiznetTest::c_packet(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq,"Status CMD called ");
  //this->stop();
  Json::Value jrep;
  Json::Value jbuf((const char*)_cpacket,(const char*)&_cpacket[_cpos]);
  std::string srep;
  srep.assign((const char*) _cpacket,_cpos);
  jrep["Bytes"]=_cpos;
  jrep["Buffer"]=jbuf;
  std::cout<<srep<<std::endl;
  response["STATUS"]=jrep;
}

void lydaq::WiznetTest::initialise()
{
  _wiznet->initialise();
  _wiznet->addCommunication(_address,_portslc);
  _wiznet->addDataTransfer(_address,_porttdc);
  _wiznet->registerDataHandler(_address,_porttdc,boost::bind(&lydaq::WiznetTest::processBuffer, this,_1,_2));
  _wiznet->registerDataHandler(_address,_portslc,boost::bind(&lydaq::WiznetTest::processSlc, this,_1,_2));
  _wiznet->listen();
}
void lydaq::WiznetTest::configure(std::string name)
{
  _msg->_address=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(_address)<<32)|_portslc;
  _msg->_length =4;
  TdcConfigAccess _tca;
  _tca.parseJsonFile(name);
  for (auto x:_wiznet->controlSockets())
    {
        
	_tca.prepareSlowControl(x.second->hostTo());

	_wiznet->writeRamAvm(x.second,_tca.slcAddr(),_tca.slcBuffer(),_tca.slcBytes());

    }

}
void lydaq::WiznetTest::start(uint16_t nc)
{
  _msg->_address=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(_address)<<32)|_portslc;
  _msg->_length =4;
  _msg->_buf[0]=htons(0xFF00);
  _msg->_buf[1]=htons(1);
  _msg->_buf[2]=htons(0x220);
  _msg->_buf[3]=htons(nc);
  _wiznet->sendMessage(_msg);
  _nProcessed=0;
  _lastGTC=0;
  _lastABCID=0;
  _event=0;
}
void lydaq::WiznetTest::mode(uint16_t nc)
{
  _msg->_address=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(_address)<<32)|_portslc;
  _msg->_length =4;
  _msg->_buf[0]=htons(0xFF00);
  _msg->_buf[1]=htons(1);
  _msg->_buf[2]=htons(0x219);
  _msg->_buf[3]=htons(nc);
  _wiznet->sendMessage(_msg);
}
void lydaq::WiznetTest::stop()
{
  _msg->_address=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(_address)<<32)|_portslc;
  _msg->_length =4;
  _msg->_buf[0]=htons(0xFF00);
  _msg->_buf[1]=htons(1);
  _msg->_buf[2]=htons(0x220);
  _msg->_buf[3]=htons(0);
  _wiznet->sendMessage(_msg);
}
#define CHBYTES 6
bool lydaq::WiznetTest::processPacket()
{
 _lBuf= (uint32_t*) &_buf[0];
 _sBuf= (uint16_t*) &_buf[0];

 if (ntohl(_lBuf[0])==0xcafedade)
   {
     _expectedLength=18;
   }
 else
   {
     _expectedLength=ntohs(_sBuf[5])*CHBYTES+16;
   }
 printf("Current length %d  ExpectedLength %d \n",_idx,_expectedLength);
 if (_idx!=_expectedLength) return false;
if (ntohl(_lBuf[0])==0xcafedade)
   {
     if (_lastABCID!=0)
       {
	 // Write Event
	 printf("Writing completed Event %d GTC %d ABCID %llu  Lines %d written\n",_event,_lastGTC,_lastABCID,_chlines);
	 _chlines=0;
       }

     
     uint32_t gtc= (_buf[7]|(_buf[6]<<8)|(_buf[5]<<16)|(_buf[4]<<24));
     uint64_t abcid=(_buf[13]|(_buf[12]<<8)|(_buf[11]<<16)|(_buf[10]<<24)|(_buf[9]<<32));
     if (abcid==_lastABCID)
       {
	 printf("HEADER ERROR \n");
       }
     _nProcessed++;
     _event++;
     _lastGTC=gtc;
     _lastABCID=abcid;
     printf("Header for new Event %d Packets %d GTC %d ABCID %llu Size %d\n",_event,_nProcessed,gtc,abcid,_idx);
#define DEBUGLINES

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

 
 _nProcessed++;
 

 uint16_t channel = ntohs(_sBuf[2]);
 uint16_t* tmp= (uint16_t*) &_buf[7];
 uint32_t gtc= (_buf[9]|(_buf[8]<<8)|(_buf[7]<<16)|(_buf[6]<<24));
 uint16_t nlines= ntohs(_sBuf[5]);
 printf ("%d packet %x # %d for channel %d with  lines %d and byte size %d \n",_nProcessed,ntohl(_lBuf[0]),gtc,channel,nlines,_idx);
 //  printf ("packet %x # %d with payload length %d  and tottal size %d \n",(_lBuf[0]),(_lBuf[1]),(_lBuf[2]),_idx);

#ifdef DEBUGLINES
 uint8_t* cl= (uint8_t*) &_buf[12];

 for (int i=0;i<nlines;i++)
   {
     // for (int j=0;j<CHBYTES;j++)
     //   printf("%.2x ",(cl[i*CHBYTES+j]));
     // printf("\n");
     _chlines++;
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
}
void lydaq::WiznetTest::processBuffer(uint16_t l,char* b)
{
  uint16_t* sptr=(uint16_t*) b;
  uint16_t lines=l/2;
  if (l>811192)
    {
    printf("\t \t ==> ERRROR %d bytes %x  %x %d lines First= %d Last= %d \n",l,ntohs(sptr[0]),ntohs(sptr[1]),ntohs(sptr[2]),ntohs(sptr[3]),ntohs(sptr[l/2-1]));
    int lines=l/2;
    for (int i=0;i<lines;i++)
      if (ntohs(sptr[i])==0xCAFE &&ntohs(sptr[i+1])==0xBABE )
	{
	  printf("Line %d ",i);
	  for (int j=0;j<8;j++)
	    printf("%x ",ntohs(sptr[i+j]));
	  printf("\n");
	}
    }
   else
    {
      
      //printf("\t \t ==> GOOD %d bytes |%x|%x|%x|%x|%x| \n",l,ntohs(sptr[0]),ntohs(sptr[1]),ntohs(sptr[2]),ntohs(sptr[3]),ntohs(sptr[4]));
      if (l<2)
	{
	   printf("\n\t ");

	   for (int i=0;i<32;i++)
	     {
	       printf("%.2x ",(uint8_t) (b[i]));
	       
	       if (i%16==15)
		 {
		   printf("\n\t ");
		 }
	     }
	   printf("\n");

	}
    }
  // return;

  int header_len=10;
  for (uint16_t ibx=0;ibx<l;ibx++)
    {
      _sBuf= (uint16_t*) &b[ibx];
      _lBuf= (uint32_t*) &b[ibx];
      uint8_t* _cBuf= (uint8_t*) &b[ibx];

      //if (ntohs(_sBuf[ibx])==0xCAFE && ntohs(_sBuf[ibx+1])==0xBABE  )
      if (ntohl(_lBuf[0]) ==0xcafebabe || ntohl(_lBuf[0]) ==0xcafedade  )
	{
	  if (_expectedLength>0 )
	    if (this->processPacket())
	      {
		_idx=0;
		_packetNb=ntohl(_lBuf[1]);
		//_lBuf[0]=htonl(++_packetNb);
		//_expectedLength=ntohl(_lBuf[2]);
		if (ntohl(_lBuf[0]) ==0xcafebabe)
		  _expectedLength=ntohs(_sBuf[5])*CHBYTES+16;
		else
		  _expectedLength=18;
	      }
	  if (_expectedLength==0)
	    {
	      _idx=0;
	      _packetNb=ntohl(_lBuf[1]);
	      //_lBuf[0]=htonl(++_packetNb);
	      //_expectedLength=ntohl(_lBuf[2]);
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
  

}
void lydaq::WiznetTest::processSlc(uint16_t l,char* b)
{
  uint16_t* sptr=(uint16_t*) b;
  uint16_t lines=l/2;
      
  printf("\t \t ==> SLC %d bytes |%x|%x|%x|%x|%x| \n",l,ntohs(sptr[0]),ntohs(sptr[1]),ntohs(sptr[2]),ntohs(sptr[3]),ntohs(sptr[4]));
  if (l<800)
    {
      printf("\n\t ");

      for (int i=0;i<l;i++)
	{
	  printf("%.2x ",(uint8_t) (b[i]));
	       
	  if (i%16==15)
	    {
	      printf("\n\t ");
	    }
	}
      printf("\n");

    }
}



