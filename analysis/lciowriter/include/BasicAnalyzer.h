#ifndef _BASICANALYZER_H
#define _BASICANALYZER_H
#include "DHCALAnalyzer.h"
#include "DHCalEventReader.h"
#include <iostream>
/** 
\class BasicAnalyzer
  \author  L.Mirabito 
  \date March 2010
  \version 1.0
  
 \brief Simplest impelentation of a DHCALAnalyzer. User's class can inherit from it and reimplement virtual methods

*/
class BasicAnalyzer : public DHCALAnalyzer
{
 public:
  //!Constructor
  /** 
      @param r Pointer to a DHCalEventReader*
   */
  BasicAnalyzer(DHCalEventReader* r) { reader_=r;} 
  virtual void processEvent() /// get the event and print a message
  { if (reader_->getEvent()!=0) std::cout<<" BasicAnalyzer New Event"<<std::endl; getchar();}
  virtual void processRunHeader(){;} /// empty implementation
  virtual void initJob(){;}  /// empty implementation
  virtual void endJob(){;}  /// empty implementation
  virtual void initRun(){;}  /// empty implementation
  virtual void endRun(){;}  /// empty implementation
 private:
  DHCalEventReader* reader_; /// Pointer to the DHCALEventReader
};
#endif
