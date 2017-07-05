#include "NetMessage.h"
NetMessage::NetMessage(std::string name,uint32_t type,uint32_t payloadsize)
{
  theMessage_ =  new unsigned char[sizeof(NetMessageHeader)+payloadsize];
  memset(theMessage_,0,sizeof(NetMessageHeader)+payloadsize);
  NetMessageHeader* hdr= (NetMessageHeader*) theMessage_;
  memset(hdr->name_,0,256);
  memcpy(hdr->name_,name.c_str(),name.size());
  hdr->type_=type;
  hdr->size_=payloadsize;
}
NetMessage::NetMessage(NetMessage& n)
{
  theMessage_ =  new unsigned char[n.getSize()];
  memcpy(theMessage_,n.getMessage(),n.getSize());
}
NetMessage::NetMessage(const boost::asio::mutable_buffer &b)
{
  unsigned char* bc=boost::asio::buffer_cast<unsigned char*>(b);
  uint32_t l=boost::asio::buffer_size(b);
  theMessage_ =  new unsigned char[l];
  memcpy(theMessage_,bc,l);
}
NetMessage::~NetMessage()
{
  delete theMessage_;
}
NetMessageHeader*  NetMessage::getHeader()
{return  (NetMessageHeader*) theMessage_;}

std::string NetMessage::getName()
{
  return std::string(this->getHeader()->name_);
}

void NetMessage::setName(std::string name)
{
  NetMessageHeader* hdr= (NetMessageHeader*) theMessage_;
  memset(hdr->name_,0,256);
  memcpy(hdr->name_,name.c_str(),name.size());

}

uint32_t NetMessage::getType()
{
  return (this->getHeader()->type_);
}

void NetMessage::setType(uint32_t type)
{
  NetMessageHeader* hdr= (NetMessageHeader*) theMessage_;
  hdr->type_=type;
}
uint32_t NetMessage::getPayloadSize()
{
  return (this->getHeader()->size_);
}

uint32_t NetMessage::getSize()
{
  return (this->getPayloadSize()+sizeof(NetMessageHeader));
}
unsigned char* NetMessage::getMessage()
{
  return theMessage_;
}
unsigned char* NetMessage::getPayload()
{
  return &theMessage_[sizeof(NetMessageHeader)];
}

void NetMessage::setPayload(unsigned char* b,uint32_t s)
{
  if (s!=0)
    this->getHeader()->size_=s;
  memcpy(this->getPayload(),b,this->getHeader()->size_);
}
