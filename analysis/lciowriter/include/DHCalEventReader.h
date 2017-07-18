#ifndef _DHCALEVENTREADER_H
#define _DHCALEVENTREADER_H
#include "lcio.h"
#include <stdio.h>
#include <bitset>
#include <vector>
#include "IO/LCReader.h"
#include "IMPL/LCTOOLS.h"
#include "IMPL/LCRunHeaderImpl.h" 
#include "IMPL/LCEventImpl.h" 

#include "EVENT/RawCalorimeterHit.h" 
#include "IMPL/LCGenericObjectImpl.h"
#include <IMPL/LCCollectionVec.h>
#include "IMPL/RawCalorimeterHitImpl.h"
#include "IMPL/LCEventImpl.h"
#include "IMPL/LCCollectionVec.h"
#include "IMPL/LCFlagImpl.h"
// #include "EVENT/SimTrackerHit.h" 
#include <UTIL/LCSplitWriter.h>
#include "UTIL/CellIDDecoder.h"
#include <IO/LCRunListener.h>
#include <IO/LCEventListener.h>

#include <cstdlib>
#include "SdhcalDifAccess.hh"


#include "DCType.h"
#include "DHCALAnalyzer.h"
#include <map>
//#include <ext/hash_map>
#include <dlfcn.h>
using namespace std ;
class LMGeneric: public IMPL::LCGenericObjectImpl
{
 public:
  LMGeneric(){;}
  std::vector<int>& getIntVector(){return _intVec;}

};

/** 
\mainpage



<H1><A NAME="SECTION00010000000000000000">
1 New LCIO format of output data </A>
</H1>

<P>
The data are now stored in LCIO format. Two collections are built
and written:

<UL>
<LI>RU_XDAQ: It is the list of buffer received by the Filter Unit after
the event building. The first buffer is specific to the acquisition,
the next ones are containing the DIF buffer readout after a short
(23 words) XDAQ header.
</LI>
<LI>RawCalorimeterHits: It contains the list of all hits found in the
format described in table <A HREF="#tab:Format-of-the-Raw">1</A>. In addition
an array of six integers containing the DIF counters is added per
DIF as a parameter named DIF_<I>dif-number</I>_Triggers. It contains
DTC,GTC,ATC, TBC in clock unit, and the LBC coded on two integers.
</LI>
</UL>

<BR><P></P>
<DIV ALIGN="CENTER"><A NAME="38"></A>
<TABLE>
<CAPTION><STRONG>Table 1:</STRONG>
Format of the RawCalorimeter hit for
DHCAL</CAPTION>
<TR><TD>
<DIV ALIGN="CENTER">
<TABLE CELLPADDING=3 BORDER="1" ALIGN="CENTER">
<TR><TH ALIGN="CENTER"><B>Field</B></TH>
<TH ALIGN="CENTER"><B>Content</B></TH>
</TR>
<TR><TD ALIGN="CENTER">ID0 bit 0-7</TD>
<TD ALIGN="CENTER">DIF Id</TD>
</TR>
<TR><TD ALIGN="CENTER">ID0 bit 8-15</TD>
<TD ALIGN="CENTER">Asic Id</TD>
</TR>
<TR><TD ALIGN="CENTER">ID0 bit 16-21</TD>
<TD ALIGN="CENTER">Channel</TD>
</TR>
<TR><TD ALIGN="CENTER">ID0 bit 22-31</TD>
<TD ALIGN="CENTER">Module</TD>
</TR>
<TR><TD ALIGN="CENTER">ID1</TD>
<TD ALIGN="CENTER">Bunch crossing time (in clock unit)</TD>
</TR>
<TR><TD ALIGN="CENTER">Amplitude</TD>
<TD ALIGN="CENTER">threshold's bits</TD>
</TR>
<TR><TD ALIGN="CENTER">TimeStamp</TD>
<TD ALIGN="CENTER">distance in clock unit to trigger</TD>
</TR>
</TABLE>
</DIV>
<P>
<DIV ALIGN="CENTER">
</DIV>

<P>

<P>
</TD></TR>
</TABLE>
</DIV><P></P>
<BR>

<P>
The Slow control informations per DIF and Asic are stored in the RunHeader
parameters with the name DIF_<I>#dif_Asic</I>_<I>#asic</I>_<I>parameter</I>.
The parameter names for HardROC2 are described in table yyy.

<P>

<H1><A NAME="SECTION00020000000000000000">
2 DHCAL Analysis code framework</A>
</H1>

<P>
The DHCalEventReader described below, is the main application that
can read or handle DHCAL LCIO events. It first implements basic functionnalities
to open LCIO files, read and write events. It has additional capabilties
to handle and parse Raw data coming from the online. Finally it can
perform analysis. It handles a vector of virtual DHCALAnalyzer 
that are called in each step of the analysis:
initialisation, change of run, new event... Several independant analysis
can be registered and performed simultanously.The histograms handling
is done using ROOT and can be managed with the DCHistogramHandler


*/







/**

\class DHCalEventReader
  \author  L.Mirabito 
  \date March 2010
  \version 1.0

   \brief Main control class of the Digital Hcal analysis 

   <h2> Description</h2>
   The <i>DHCalEventReader </i> is the main class to read/write/analyze events from the DHCAL. It allows to
   <ul>
   <li> Open,read , write LCIO files containing DHCAL events
   <li> Handle and parse raw data from the online and build the corresponding event
   <li> Run any number of analysis sequentially
   
   </ul>

    <h2> Analysis registration</h2>
    The object handles a vector of <i>DHCALAnalyzer*</i>, a virtual analysis class that defines the following methods:
    <ul>
    <li> initJob() 
    <li> endJob() 
    <li> initRun()
    <li> endRun()
    <li> processEvent()
    </ul>
    The DHCalEventReader defines analog methods (initJob,endJonb....analyzeEvent) in which it loops over all registered analysis and calls the
    corresponding methods. A typical implementation of a DHCALAnalyzer would be 
    <pre>
    class MyAnalysis : public DHCALAnalyzer
    {
    public:
         MyAnalysis(DHCalEventReader* rdr) { reader_=r;}
	 void processEvent()  { LCTOOLS::DumpEvent(reader_->getEvent());}
	 void initJob() {;}
....
    private:
        DHCalEventReader* reader_;
     }
     </pre>
     and the main program will be:

     <pre>
     DHCalEventReader a;  // Create the DHCALEventReader
     MyAnalysis ana(&a); // Create an analysis
     a.registerAnalysis(&ana); // Register it
     a.open("Myfile.lcio");  // Open LCIO file
     a.readRun(); // ReadNextRun
     int ier=a.readEvent(); // Read First Event
     while (ier>=0)
     {
       a.buildEvent();  // Build LCIO event if not yet
       a.analyzeEvent();  // call the registered analysis
       ier=a.readEvent(); // read next event
     }
     a.close();
     </pre>

  
 */

class DHCalEventReader : public LCRunListener, public LCEventListener
{
public:
  //! Constructor

  DHCalEventReader();
  //! Destructor 
 
  ~DHCalEventReader();

  // singleton access
  static DHCalEventReader* instance();
 
  //! Get the current event. It does not read it 
  /** 
      @return an IMPL::LCEventImpl ptr
  */
  IMPL::LCEventImpl* getEvent() { return evt_;}
 
  //! Get the current Run Header. It does not read it 
  /** 
      @return an IMPL::LCRunHeaderImpl ptr
  */
  IMPL::LCRunHeaderImpl* getRunHeader(){return runh_;}

  //! Create an empty LCIO event. 
  /** The RU_XDAQ collection is also created and attached to it  
      @param run Run number
      @param det Name of the detector
  */
  void createEvent(int run,std::string det,bool del=true);

  //! Create an empty LCIO Run header  
  /** 
      @param run Run number
      @param det Name of the detector
  */

  void createRunHeader(int run,std::string det);
  
  //! Set the current Event 
  /** 
      @param evt pointer to an IMPL::LCEventImpl
  */
  void setEvent(IMPL::LCEventImpl* evt) {evt_=evt;}
 
  //! Set the current run header 
  /** 
      @param runh pointer to an IMPL::LCRunHeaderImpl
  */
  void setRunHeader(IMPL::LCRunHeaderImpl* runh) { runh_ = runh;}
  
  //! open the given file
  /** 
      @param name file name
  */
  void open(std::string name);

  void open (std::vector< std::string > &filenames);

  //! close the current file
  void close();

  //! Clear internal buffera
  void clear();


  //! Read the stream and call handlers
  /** 
      @return  ier < 0 if end of file
  */
  int readStream(int max_record=0);



  //! Read next event
  /** 
      @return  ier > 0 if new event found
  */
  int readEvent();
int readOneEvent(int run,int event);
  //! Read next run
  void readRun();

  //! Add a RU_XDAQ entry
  /** 
      @param buf int* pointer to the data
      @param rusize size of the buffer in int
  */
  void addRawOnlineRU(int* buf,int rusize);
  
  //! Open the output  file
  /** 
      @param filename file name
  */
  void openOutput(std::string filename);

  //! Write the RunHeader
  void writeRunHeader();


  //! Fill the RunHeader with Slow control informations
  void fillRunHeader();


  //! write the event
  /** 
      @param dropRaw  if true the RU_XDAQ collection is dropped
  */
  void writeEvent(bool dropRaw);

  void write(IMPL::LCEventImpl* evt);

  
  //! close the output file
  void closeOutput();


  //! Register a virtual analysis
  /** 
      @param a  a pointer to a DHCALAnalyzer
  */
  void registerAnalysis(DHCALAnalyzer* a);
  /** 
      @param name, name of a  DHCALAnalyzer library
  */
  void registerAnalysis(std::string name,std::string path="./");
  
  //! Loops on all DHCALanalyzer and call processEvent()
  void analyzeEvent();

  //! Loops on all DHCALanalyzer and call initJob()
  void initJob();

  //! Loops on all DHCALanalyzer and call endJob()
  void endJob();

  //! Loops on all DHCALanalyzer and call initRun()
  void initRun();

  //! Loops on all DHCALanalyzer and call endRun()
  void endRun();

  //! return the data format version 
  /**
     @return  version number read from the first DIF buffer
  */
  unsigned int getVersion() { return version_;}

  
  //! Access to the analysis vector
  /** 
      @return  vector of ptr to DHCALAnalyzer
  */
  std::vector<DHCALAnalyzer*> getAnalysis(){ return vProcess_;}


  //! Interface for readStream
  void processEvent( LCEvent * evt )  ;
  //! Interface for readStream
  void processRunHeader( LCRunHeader* run) ;
  //! Interface for readStream
  void modifyEvent( LCEvent * evt ) { /* not needed */ ;}
  //! Interface for readStream
  void modifyRunHeader(LCRunHeader* run){ /* not needed */ ;}
  //! <i>Internal</i> parse the DIF buffer 
  int parseSDHCALEvent();
  //! <i>Internal</i> parse the DIF buffer 
  int parseEvent();
 

  void findDIFSeeds(int32_t ndifmin,std::string colname="DHCALRawHits");
  void findTimeSeeds(int32_t nasic_min,std::string colname);

  //! <i>Internal</i> create the DHCALRawHits collection 
//
  IMPL::LCCollectionVec* createRawCalorimeterHits(std::vector<uint32_t> seeds);


  void setDebug(uint32_t l){debugLevel_=l;}
  void setDropFirstRU(bool t){dropFirstRU_=t;}
  void setXdaqShift(unsigned int s){theXdaqShift_=s;}
  //void addFile(std::string s) {filenames_.push_back(s);}

  void addFile(std::string s) {filenames_.push_back(s);}
  uint32_t getNumberOfEvents(){return lcReader_->getNumberOfEvents();}
  void openFiles(){this->open(filenames_);}
  void findEvent(int run,int event);
  std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> >& getPhysicsEventMap(){return thePhysicsEventMap_;}
  std::vector<uint32_t>& getTimeSeeds(){return theTimeSeeds_;}
  std::vector<sdhcal::DIFPtr*>& getDIFList(){return  theDIFPtrList_;}
  bool isBad(){return _badEvent;}
 private:
  LCReader* lcReader_; /// LCIO Reader
  //  LCSplitWriter* lcWriter_; /// LCIO Writer
  LCWriter* lcWriter_; /// LCIO Writer
  IMPL::LCEventImpl* evt_; /// LCIO Event ptr
  IMPL::LCEventImpl* evtOutput_; /// LCIO Event output ptr
  IMPL::LCRunHeaderImpl* runh_; /// LCIO Run Heder ptr
 




  std::vector<std::string> filenames_;
  std::vector<LCReader*> readers_;
  std::vector<DHCALAnalyzer*> vProcess_; /// Vector of DHCALAnalyzer
  
  uint32_t version_; /// version of data
  bool newRunHeader_;/// True if new run header found


  uint32_t debugLevel_;
  bool dropFirstRU_;
  uint32_t theXdaqShift_;
  
  std::vector<sdhcal::DIFPtr*> theDIFPtrList_;

  std::string currentFileName_;
  static DHCalEventReader* _me;
  std::vector<uint32_t> theTimeSeeds_;
  
  std::map<uint32_t,std::vector<IMPL::RawCalorimeterHitImpl*> > thePhysicsEventMap_;

  bool _badEvent;
};

#endif
