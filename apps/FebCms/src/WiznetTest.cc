#include "WiznetTest.hh"
lydaq::WiznetTest::WiznetTest(std::string address,uint16_t portslc,uint16_t porttdc)
  : _address(address),_portslc(portslc),_porttdc(porttdc),_idx(0),_sBuf(0),_lBuf(0),_currentLength(0)
{
  _wiznet= new lydaq::WiznetInterface();
  _msg=new lydaq::WiznetMessage();
  memset(_buf,0,32*1024);
}

void lydaq::WiznetTest::initialise()
{
  _wiznet->initialise();
  _wiznet->addCommunication(_address,_portslc);
  _wiznet->addDataTransfer(_address,_porttdc);
  _wiznet->registerDataHandler(_address,_porttdc,boost::bind(&lydaq::WiznetTest::processBuffer, this,_1,_2));
  _wiznet->listen();
}
void lydaq::WiznetTest::start()
{
  _msg->_address=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(_address)<<32)|_portslc;
  _msg->_length =4;
  _msg->_buf[0]=htons(0xFF00);
  _msg->_buf[1]=htons(1);
  _msg->_buf[2]=htons(0x221);
  _msg->_buf[3]=htons(1);
  _wiznet->sendMessage(_msg);
}
void lydaq::WiznetTest::stop()
{
  _msg->_address=( (uint64_t) lydaq::WiznetMessageHandler::convertIP(_address)<<32)|_portslc;
  _msg->_length =4;
  _msg->_buf[0]=htons(0xFF00);
  _msg->_buf[1]=htons(1);
  _msg->_buf[2]=htons(0x221);
  _msg->_buf[3]=htons(0);
  _wiznet->sendMessage(_msg);
}
void lydaq::WiznetTest::processPacket()
{
 _lBuf= (uint32_t*) &_buf[1];
 printf ("packet # %d with payload length %d  and tottal size %d \n",ntohl(_lBuf[0]),ntohl(_lBuf[1]),_idx);
}
void lydaq::WiznetTest::processBuffer(uint16_t l,char* b)
{
  uint16_t* sptr=(uint16_t*) b;
  uint16_t lines=l/2;
  if (l>2042)
    {
    printf("\t \t ==> ERRROR %d bytes %x  %x %d lines First= %d Last= %d \n",l,ntohs(sptr[0]),ntohs(sptr[1]),ntohs(sptr[2]),ntohs(sptr[3]),ntohs(sptr[l/2-1]));
    int lines=l/2;
    for (int i=0;i<lines;i++)
      if (ntohs(sptr[i])==0xCAFE)
	{
	  printf("Line %d ",i);
	  for (int j=0;j<8;j++)
	    printf("%x ",ntohs(sptr[i+j]));
	  printf("\n");
	}
    }
  int header_len=10;
  for (uint16_t ibx=0;ibx<lines;ibx++)
    {
      _sBuf= (uint16_t*) &sptr[ibx];
      _lBuf= (uint32_t*) &sptr[ibx+1];

      if (ntohs(_sBuf[0])==0xCAFE)
	{
	  if (_currentLength>0) this->processPacket();
	  _idx=0;
	  _packetNb=ntohl(_lBuf[0]);
	  _currentLength=ntohl(_lBuf[1]);

	  printf("START %d %d \n",_packetNb,_currentLength);
	  
	}
      else
	
      _buf[_idx]= sptr[ibx];
      _idx++;
    }
  

  /*  
   else
    {
      
      printf("\t \t ==> GOOD %d bytes %x  %x %x %x %x \n",l,ntohs(sptr[0]),ntohs(sptr[1]),ntohs(sptr[2]),ntohs(sptr[3]),ntohs(sptr[4]));
    }
  */
}
