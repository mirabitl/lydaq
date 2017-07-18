#include "SdhcalDifAccess.hh"

#include "FilterAnalyzer.h"
std::map<uint32_t,IMPL::LCEventImpl*> evtmap;
void FilterAnalyzer::initHistograms()
{

}
FilterAnalyzer::FilterAnalyzer(DHCalEventReader* r,DCHistogramHandler* h)  :useSynchronized_(false),minChambersInTime_(3),lastEvent_(0)
{
  reader_=r;
  handler_ =h;
  headerWritten_=false;
}
void FilterAnalyzer::endJob()
{
  for (std::map<uint32_t,IMPL::LCEventImpl*>::iterator it=evtmap.begin();it!=evtmap.end();it++)
    {
      std::cout<<" Eritting"<<it->first;
      reader_->write(it->second);
    }
}

void FilterAnalyzer::processEvent()
{
  
  if (reader_->getEvent()==0) return;
  if (reader_->getRunHeader()!=0 && !headerWritten_) 
    {
      reader_->writeRunHeader();
      headerWritten_=true;
    }

  try {

    //reader_->buildEvent(rebuild);
    //LCTOOLS::dumpEvent( reader_->getEvent() ) ;
    //    getchar();
    // reader_->analyzeEvent();
      // getchar();
    std::cout<<"event "<<reader_->getEvent()->getEventNumber()<<std::endl;
    IMPL::LCCollectionVec* HitVec;
    std::vector<uint32_t> seed;

    rebuild_=true;
    if (rebuild_)
      {
      reader_->parseEvent();
   
       
  //
  // getchar();
     printf("Calling CreateRaw %d\n",(int) seed.size());

     HitVec=reader_->createRawCalorimeterHits(seed);
      
      //printf("End of CreaetRaw %d \n",rhcol->getNumberOfElements());



      }
    else
      {
	HitVec=(IMPL::LCCollectionVec*) reader_->getEvent()->takeCollection("DHCALRawHits");
	HitVec->setTransient(false);
      }
      IMPL::LCEventImpl* evt_=reader_->getEvent();
      if (evt_->getEventNumber()%1==0)
	std::cout<<evt_->getEventNumber()<<" Number of Hit "<<HitVec->getNumberOfElements()<<std::endl;

      IMPL::LCEventImpl* evtOutput_ =new IMPL::LCEventImpl();
      evtOutput_->setRunNumber(evt_->getRunNumber());
      evtOutput_->setEventNumber(evt_->getEventNumber());
      evtOutput_->setTimeStamp(evt_->getTimeStamp());
      evtOutput_->setDetectorName(evt_->getDetectorName());
      evtOutput_->setWeight(evt_->getWeight());
      evtOutput_->addCollection(HitVec,"DHCALRawHits");
      //LCTOOLS::printRawCalorimeterHits(HitVec);
      if (writing_)
#undef NOSORT
#ifdef NOSORT
	reader_->write(evtOutput_);
#else
	{
	  std::pair<uint32_t,IMPL::LCEventImpl*> p(evt_->getEventNumber(),evtOutput_);
	  evtmap.insert(p);
	  std::map<uint32_t,IMPL::LCEventImpl*>::iterator nextevt=evtmap.find(lastEvent_+1);
	  while (nextevt!=evtmap.end())
	    {
	      printf(" Writting %d \n",nextevt->first);
	      reader_->write(nextevt->second);
	      evtmap.erase(nextevt);
	      lastEvent_=lastEvent_+1;
	      nextevt=evtmap.find(lastEvent_+1); 
	    }
	  
	}
#endif
      else
	delete evtOutput_;
  }
  catch (std::string e )
    {
      std::cout<<e<<std::endl;
    }


}
