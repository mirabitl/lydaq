#ifndef _NetMessage_h
#define _NetMessage_h
#include <boost/asio.hpp>
#include <string>
#include <vector>
#include <list>
#include <boost/cstdint.hpp>

//-----------------------------------------------------------------------------

using boost::uint64_t;
using boost::uint32_t;
using boost::uint16_t;
using boost::uint8_t;

using boost::int64_t;
using boost::int32_t;
using boost::int16_t;
using boost::int8_t;

typedef struct
{

 uint32_t type_;
  uint32_t size_;
  char name_[256];
} NetMessageHeader;

class NetMessage
{
 public:
  enum {COMMAND=1,COMMAND_ACKNOWLEDGE=2,SERVICE=3};

  NetMessage(std::string name,uint32_t type,uint32_t payLoadsize=4);
  NetMessage(NetMessage &n);
  NetMessage(const boost::asio::mutable_buffer &b);
  ~NetMessage();
  
  NetMessageHeader* getHeader();
  std::string getName();

  uint32_t getType();
  uint32_t getPayloadSize();
  uint32_t getSize();
  unsigned char* getPayload();
  unsigned char* getMessage();
  void setPayload(unsigned char* b,uint32_t s=0);
  void setName(std::string n);
  void setType(uint32_t t);

 private:
  
  unsigned char* theMessage_;
  

};
#endif
