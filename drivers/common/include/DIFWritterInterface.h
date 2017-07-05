#ifndef _DIFWritterInterface_h_
#define _DIFWritterInterface_h_
#include <string>
#include <stdint.h>
#include <vector>
class DIFWritterInterface
{
public:
  DIFWritterInterface();
  virtual void openFile(uint32_t run=0,std::string dir="/tmp",std::string setup="default")=0;
  virtual void writeEvent(uint32_t gtc,std::vector<unsigned char*> vbuf)=0;
  virtual void closeFile()=0;
 protected:
  uint32_t theRun_;
  std::string theDirectory_;
};
#endif
