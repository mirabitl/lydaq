#ifndef _DHCALANALYZER_H
#define _DHCALANALYZER_H
/** 
\class DHCALAnalyzer
  \author  L.Mirabito 
  \date March 2010
  \version 1.0
  
 \brief Purely abstract class that defines the analysis method

*/
class DHCalEventReader;
class DHCALAnalyzer
{
 public:
  virtual void processEvent()=0; /// Process the event 
  virtual void processRunHeader()=0; /// Process the Run header
  virtual void initJob()=0; /// Init the job
  virtual void endJob()=0; /// end of job callback
  virtual void initRun()=0; /// Begin of run  call back
  virtual void endRun()=0; /// Ened of run call back
};
#endif
