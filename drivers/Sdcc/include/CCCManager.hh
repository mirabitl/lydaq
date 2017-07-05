#ifndef _CCCManager_H
#define _CCCManager_H
#include <stdint.h>
#include <iostream>
#include "CCCReadout.hh"

namespace lydaq
{
  class CCCManager 
  {
  public:
    CCCManager(std::string CCCName = "DCCCCC01",std::string CCCType="DCC_CCC");
    static std::string discover();
    void initialise();
    void destroy();
    void configure();
    void start();
    void stop();
    void test();
  
  
    inline lydaq::CCCReadout* getCCCReadout(){return theCCC_;}
  private:
    lydaq::CCCReadout* theCCC_;

    std::string theCCCType_;
    std::string theCCCName_;
  };
};
#endif
