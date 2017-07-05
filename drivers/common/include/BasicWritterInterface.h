#ifndef _BasicWritterInterface_h_
#define _BasicWritterInterface_h_
#include "DIFWritterInterface.h"
#include <stdint.h>
class BasicWritterInterface : public DIFWritterInterface
{
public:
  BasicWritterInterface();
  virtual void openFile(uint32_t run=0,std::string dir="/tmp",std::string setup="default");
  virtual void writeEvent(uint32_t gtc,std::vector<unsigned char*> vbuf);
  virtual void closeFile();
 private:
  uint32_t theEventNumber_,theTotalSize_;
  int fdOut_;
};
#endif
