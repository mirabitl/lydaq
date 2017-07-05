
#undef DEBUG
#include "DIFReadoutConstant.h"
#include "ShmProxy.h"
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <unistd.h>  
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
extern  int alphasort(); //Inbuilt sorting function  
#define FALSE 0  
#define TRUE !FALSE      
int file_select_1(const struct direct *entry)  
{  
  if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))  
    return (FALSE);  
  else  
    return (TRUE);  
}  

ShmProxy::ShmProxy(uint32_t nbdif,bool save,DIFWritterInterface* w) : theNumberOfDIF_(nbdif),theSave_(save),theWritter_(w),theSetupName_("default"),theDirectoryName_("/tmp")
{
  theBufferMap_.clear();
  theSync_.unlock();
  theSave_=(theWritter_!=NULL);
}
uint32_t ShmProxy::getRunNumber(){return theRunNumber_;}
uint32_t ShmProxy::getEventNumber(){return theEventNumber_;}
uint32_t ShmProxy::getNumberOfDIF(){return theNumberOfDIF_;}
void ShmProxy::setNumberOfDIF(int32_t t)
{ theNumberOfDIF_=t;
  printf("NNNNNNNNNNNNNNNNNNNNNNNNN   %d %x %x \n",theNumberOfDIF_,&theNumberOfDIF_,this);}
  
void ShmProxy::setDirectoryName(std::string s){theDirectoryName_=s;}
void ShmProxy::setSetupName(std::string s){theSetupName_=s;}
void ShmProxy::Initialise(bool purge)
{
  LOG4CXX_INFO(_logShm,"Initialising "<<purge);
  theRunIsStopped_=true;
  //printf("avant buffermap clear \n");

  theBufferMap_.clear();
  //printf("avant purgeshm clear \n");
  if (purge)
    ShmProxy::purgeShm();
  printf("avant mkdir  clear \n");
  //int32_t ier=system("mkdir -p /dev/shm/closed");
  int status = mkdir("/dev/shm/closed", S_IRWXU | S_IRWXG | S_IRWXO );
	
}
void ShmProxy::Configure()
{
  theRunIsStopped_=true;
  printf("%s  Configure End \n",__PRETTY_FUNCTION__);
  LOG4CXX_INFO(_logShm,"Configuring ");
}
void ShmProxy::svc()
{
  std::cout<< "Je rentre"<<std::endl;
  LOG4CXX_INFO(_logShm," Starting loop ");
  while (true)
    {
      if (theRunIsStopped_) {usleep(100000); continue;}
      //std::cout<< "on va lire"<<std::endl;
      if (!performReadFiles()) break;
      //std::cout<< "on va ecrire"<<std::endl;
      //if (theSave_) 
      if (!performWrite()) break;
      //std::cout<< "on y retourne "<<std::endl;
    }
}

#ifdef OLDSTUFF
void ShmProxy::openFile(uint32_t run)
{
  std::stringstream filename("");    
  char dateStr [64];
            
  time_t tm= time(NULL);
  strftime(dateStr,20,"SMM_%d%m%y_%H%M%S",localtime(&tm));
  filename<<"/tmp/"<<dateStr<<".dat";
  fdOut_= ::open(filename.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  if (fdOut_<0)
    {
      perror("No way to store to file :");
      //std::cout<<" No way to store to file"<<std::endl;
      return;
    }  
}
#endif
void ShmProxy::Start(uint32_t run,std::string dir,uint32_t nd)
{
  theBufferMap_.clear();
  printf("avant purgeshm clear \n");
  
  ShmProxy::purgeShm();

  if (nd!=0) theNumberOfDIF_=nd;
  printf("Starting run %d on directory %s setup %s for %d DIF %x %x\n",run,theDirectoryName_.c_str(),theSetupName_.c_str(),theNumberOfDIF_,&theNumberOfDIF_,this);
  LOG4CXX_INFO(_logShm,"Run "<<run<<" on "<<theDirectoryName_<<" Db:"<<theSetupName_<<" with "<<theNumberOfDIF_<<" DIFs");
  theBufferMap_.clear();
  printf("1 \n");
  theRunNumber_=run;
  printf("2 \n");
  theRunIsStopped_=false;
  printf("3 \n");
  lastGTCWrite_=0;
  if (theSave_) theWritter_->openFile(run,theDirectoryName_,theSetupName_);
  printf("4 \n");
  theEventNumber_=0;


  printf("5 \n");
  theTotalSize_=0;
  theTime_=time(0);
  theThread_ = boost::thread(&ShmProxy::svc, this);

  printf("6 \n");


}


void ShmProxy::Stop()
{
  theRunIsStopped_=true;
  LOG4CXX_INFO(_logShm,"Stopping loops");
  sleep((unsigned int) 1);
  //theThread_.join();
  //   if (theSave_)
  if (theSave_) theWritter_->closeFile();

  if (fdOut_>0)
    {
      ::close(fdOut_);
      fdOut_=-1;
    }
  LOG4CXX_INFO(_logShm,"Loops are stopped");
}



bool ShmProxy::performWrite()
{
  //  int32_t data[0x100000];
  if (theRunIsStopped_) {usleep((uint32_t) 10); return false;}
  theSync_.lock();
  if (theSave_)
    {
      LOG4CXX_DEBUG(_logShm,"Size of Map#"<<theBufferMap_.size())
      for (std::map<uint64_t,std::vector<unsigned char*> >::iterator it=theBufferMap_.begin();it!=theBufferMap_.end();it++)
	{
	  
	  if (it->second.size()!=theNumberOfDIF_) continue;
	  std::vector<unsigned char*>::iterator iv=it->second.begin();
	  uint32_t* idata=(uint32_t*) (*iv);
	  uint32_t gtc=idata[SHM_BX_NUMBER];
	  /* if (theEventNumber_!=(gtc-1))
	    {
	      printf("Wrong event number %d GTC  %d \n",theEventNumber_,getBufferGTC(it->second[0]));
	      continue;
	    }
	  */
	  lastGTCWrite_=it->first;
	  printf("%x writing %d \n",this,theEventNumber_);
	  LOG4CXX_INFO(_logShm,"Writing event #"<<theEventNumber_<<" GTC "<<gtc);
	  theWritter_->writeEvent(theEventNumber_,it->second);

	  if (theEventNumber_%100 == 0)
	    for (std::vector<unsigned char*>::iterator iv=it->second.begin();iv!=it->second.end();iv++) 
	      {
		unsigned char* cdata=(*iv);
		uint32_t* idata=(uint32_t*) cdata;
		printf("\t DIF %d writing %d bytes \n",idata[SHM_APV_ADDRESS],idata[SHM_BUFFER_SIZE]);
		

	      }
	  struct direct **files;     
	  int count = scandir("/dev/shm/monitor/closed/", &files, file_select_1, alphasort);  
	  if (count<10*theNumberOfDIF_)
	    {
	      //ShmProxy::run2DevShm(theRunNumber_,"/dev/shm/monitor");
	      for (std::vector<unsigned char*>::iterator iv=it->second.begin();iv!=it->second.end();iv++) 
		{
		  unsigned char* cdata=(*iv);
		  uint32_t* idata=(uint32_t*) cdata;
		  ShmProxy::save2DevShm(cdata,idata[SHM_BUFFER_SIZE],SHM_EVENT_ADDRESS*sizeof(uint32_t),"/dev/shm/monitor");
		}
	    }
      // if (theEventNumber_%100==0) 
      // 	std::cout<<"Standard completion "<<theEventNumber_<<" GTC "<<it->first<<" Time "<<time(0)-theTime_<<" size "<<theTotalSize_<<std::endl;
      // // To be implemented
      // if (fdOut_>0)
      // 	{
      // 	  int ier=write(fdOut_,&theEventNumber_,sizeof(uint32_t));
      // 	  ier=write(fdOut_,&theNumberOfDIF_,sizeof(uint32_t));
      // 	  for (std::vector<unsigned char*>::iterator iv=it->second.begin();iv!=it->second.end();iv++) 
      // 	    {
      // 	      unsigned char* cdata=(*iv);
      // 	      uint32_t* idata=(uint32_t*) cdata;
      // 	      //printf("\t writing %d bytes",idata[SHM_BUFFER_SIZE]);
      // 	      theTotalSize_+=idata[SHM_BUFFER_SIZE];
      // 	      ier=write(fdOut_,&idata[SHM_BUFFER_SIZE],sizeof(uint32_t));
      // 	      ier=write(fdOut_,cdata,idata[SHM_BUFFER_SIZE]);
      // 	    }
		
		

      	  theEventNumber_++;
	}
      //
    }
	
  for (std::map<uint64_t,std::vector<unsigned char*> >::iterator it=theBufferMap_.begin();it!=theBufferMap_.end();)
    {
		
      if (it->second.size()==theNumberOfDIF_)
	{
	  for (std::vector<unsigned char*>::iterator iv=it->second.begin();iv!=it->second.end();iv++) delete (*iv);
	  it->second.clear();
	  theBufferMap_.erase(it++);
	}
      else
	it++;
    }
	
  theSync_.unlock();
  // if (theTotalSize_>500*1024*1024)
  //   {
  //     ::close(fdOut_);
  //     theTotalSize_=0;
  //     openFile();
  //   }

  usleep(10000);
  return true;
	
}


void ShmProxy::purgeShm(std::string memory_dir)
{
  int count,i;  
  struct direct **files;
 std::stringstream s("");
 s<<memory_dir<<"/closed/";
  
 count = scandir(s.str().c_str(), &files, file_select_1, alphasort);  
	
  /* If no files found, make a non-selectable menu item */  
  for (i=1; i<count+1; ++i)
    {
      uint32_t dtc,gtc,dif;
      unsigned long long abcid;
      sscanf(files[i-1]->d_name,"%lld_%d_%d_%d",&abcid,&dtc,&gtc,&dif);
      //printf("dif %d DTC %d GTC %d \n",dif,dtc,gtc);
		
      char fname[256];
      sprintf(fname,"%s/Event_%lld_%d_%d_%d",s.str().c_str(),abcid,dtc,gtc,dif);
      unlink(fname);
      sprintf(fname,"%s/closed/%lld_%d_%d_%d",s.str().c_str(),abcid,dtc,gtc,dif);
      unlink(fname);
    }
	
}
void ShmProxy::transferToFile(unsigned char* cbuf,uint32_t size_buf,uint64_t bcid,uint32_t detector_event,uint32_t global_event,uint32_t detid,std::string memory_dir)
{

  std::stringstream s("");
  s<<memory_dir<<"/Event_"
   << bcid<<"_"
   << detector_event<<"_"
   <<global_event<<"_"
   <<detid;
  int fd= ::open(s.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  if (fd<0)
    {
      LOG4CXX_FATAL(_logShm," Cannot open shm file "<<s.str());
      perror("No way to store to file :");
      //std::cout<<" No way to store to file"<<std::endl;
      return;
    }
  int ier=write(fd,cbuf,size_buf);
  if (ier!=size_buf) 
    {
      LOG4CXX_FATAL(_logShm," Cannot write shm file "<<s.str());
      std::cout<<"pb in write "<<ier<<std::endl;
      return;
    }
  ::close(fd);
  std::stringstream st("");
  st<<memory_dir<<"/closed/"
    << bcid<<"_"
    << detector_event<<"_"
    <<global_event<<"_"
    <<detid;
  fd= ::open(st.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  //std::cout<<st.str().c_str()<<" "<<fd<<std::endl;
  //write(fd,b,1);
  ::close(fd);

}
uint32_t ShmProxy::getBufferDIF(unsigned char* cb,uint32_t idx)
{
  return cb[idx+DIF_ID_SHIFT];
}
uint32_t ShmProxy::getBufferDTC(unsigned char* cb,uint32_t idx)
{
  return (cb[idx+DIF_DTC_SHIFT]<<24)+(cb[idx+DIF_DTC_SHIFT+1]<<16)+(cb[idx+DIF_DTC_SHIFT+2]<<8)+cb[idx+DIF_DTC_SHIFT+3];
}
uint32_t ShmProxy::getBufferGTC(unsigned char* cb,uint32_t idx)
{
  return (cb[idx+DIF_GTC_SHIFT]<<24)+(cb[idx+DIF_GTC_SHIFT+1]<<16)+(cb[idx+DIF_GTC_SHIFT+2]<<8)+cb[idx+DIF_GTC_SHIFT+3];
}
unsigned long long ShmProxy::getBufferABCID(unsigned char* cb,uint32_t idx)
{
  unsigned long long Shift=16777216ULL;//to shift the value from the 24 first bits
  unsigned long long LBC= ( (cb[idx+DIF_BCID_SHIFT]<<16) | (cb[idx+DIF_BCID_SHIFT+1]<<8) | (cb[idx+DIF_BCID_SHIFT+2]))*Shift+( (cb[idx+DIF_BCID_SHIFT+3]<<16) | (cb[idx+DIF_BCID_SHIFT+4]<<8) | (cb[idx+DIF_BCID_SHIFT+5]));
  return LBC;
}
		  
uint32_t ShmProxy::getBufferDIFTemp(unsigned char* cb,uint32_t idx)
{
  if (cb[idx+DIF_GLOBAL_HEADER_SHIFT]==0xBB)
    {
      for (int i=0;i<40;i++)
	printf ("(%d)=%02x  ",i,cb[idx+i]);
      printf ("\n");
			    
      printf ("cb[%d]=%02X\n",idx+DIF_GLOBAL_HEADER_SHIFT,cb[idx+DIF_GLOBAL_HEADER_SHIFT]);
      printf ("cb[%d]=%02x\n",idx+DIF_DIFTEMP_SHIFT,cb[idx+DIF_DIFTEMP_SHIFT]);
				
    }
  //printf ("*********** %02x %02x **********************\n",cb[idx+DIF_GLOBAL_HEADER_SHIFT],cb[idx+DIF_DIFTEMP_SHIFT]);
  if (cb[idx+DIF_GLOBAL_HEADER_SHIFT]==0xBB)
    return cb[idx+DIF_DIFTEMP_SHIFT];
  else
    return 0;
}
			

unsigned char*  ShmProxy::packDIFData(unsigned char* cbuf,uint32_t size,uint32_t gtc,uint32_t dtc,uint32_t id)
{
  uint32_t OffsetHeader=SHM_EVENT_ADDRESS*sizeof(uint32_t);
  uint32_t TotalSize=OffsetHeader+size;
  unsigned char* cdata= new unsigned char[TotalSize];
  uint32_t* idata= (uint32_t*) cdata;


  memcpy(&cdata[OffsetHeader],cbuf,size);
  //Store fullSize

  idata[SHM_BUFFER_SIZE]=TotalSize;
  idata[SHM_READ_NUMBER]=dtc;
  idata[SHM_BX_NUMBER]=gtc;
  idata[SHM_APV_ADDRESS]=id;
  idata[SHM_EVENT_NUMBER]=dtc;
  return cdata;
}
bool ShmProxy::performReadFiles()
{
  //std::cout<<"Dans performReadout" <<theRunIsStopped_<<std::endl;

  if (theRunIsStopped_) {usleep((uint32_t) 10);return false;}

  // Build a select from all 
  //  uint32_t OffsetHeader=SHM_EVENT_ADDRESS*sizeof(uint32_t);
  unsigned char cbuf[0x20000];
	
  //        usleep(run->delay_usr);       
  // Now pack the data and find the GTC

  int count,i;  
  struct direct **files;  
		
  count = scandir("/dev/shm/closed/", &files, file_select_1, alphasort);  
		
  /* If no files found, make a non-selectable menu item */  
  if(count <= 0)  
    {::sleep(1);return true;}
  theSync_.lock();	      
		
  //printf("Number of files = %d\n",count);  
  for (i=1; i<count+1; ++i)  
    {
      //printf("%s  \n",files[i-1]->d_name);  
      int32_t dif,dtc,gtc;
      unsigned long long abcid;
			
      sscanf(files[i-1]->d_name,"%lld_%d_%d_%d",&abcid,&dtc,&gtc,&dif);
      //printf("dif %d DTC %d GTC %d \n",dif,dtc,gtc);
      if (gtc<=0 || dtc<=0 || (gtc!=dtc)) continue;
      char fname[256];
      sprintf(fname,"/dev/shm/Event_%lld_%d_%d_%d",abcid,dtc,gtc,dif);
      int fd=::open(fname,O_RDONLY);
      if (fd<0) 
	{
	  printf("%s  Cannot open file %s : return code %d \n",__PRETTY_FUNCTION__,fname,fd);
	  LOG4CXX_FATAL(_logShm," Cannot open shm file "<<fname);
	  theSync_.unlock();	
	  return false;
	}
      int size_buf=::read(fd,cbuf,0x20000);
      //printf("%d bytes read %x %d \n",size_buf,cbuf[0],cbuf[1]);
      ::close(fd);
      ::unlink(fname);
      sprintf(fname,"/dev/shm/closed/%lld_%d_%d_%d",abcid,dtc,gtc,dif);
      ::unlink(fname);
			

      unsigned char*  cdata=ShmProxy::packDIFData(cbuf,size_buf,gtc,dtc,dif);
      //memcpy(cbuf,cdata,size_buf);
      uint64_t idx_storage=gtc; // usually abcid

      std::map<uint64_t,std::vector<unsigned char*> >::iterator it_gtc=theBufferMap_.find(idx_storage);
      if (it_gtc!=theBufferMap_.end())
	it_gtc->second.push_back(cdata);
      else
	{
#define ONE_ENTRY_PER_BCID
#ifdef ONE_ENTRY_PER_BCID
          std::vector<unsigned char*> v;
          v.clear();
          v.push_back(cdata);
          
          std::pair<uint64_t,std::vector<unsigned char*> > p(idx_storage,v);
          theBufferMap_.insert(p);
          it_gtc=theBufferMap_.find(gtc);
#else
	

	  it_gtc=theBufferMap_.find(abcid-1);
	  if (it_gtc!=theBufferMap_.end())
	    it_gtc->second.push_back(cdata);
	  else
	    {
	      it_gtc=theBufferMap_.find(abcid+1);
	      if (it_gtc!=theBufferMap_.end())
		it_gtc->second.push_back(cdata);
	      else
		{
		  std::vector<unsigned char*> v;
		  v.clear();
		  v.push_back(cdata);
						
		  std::pair<uint64_t,std::vector<unsigned char*> > p(abcid,v);
		  theBufferMap_.insert(p);
		  it_gtc=theBufferMap_.find(gtc);
		}
	    }
#endif
	}
       if (it_gtc->second.size()==theNumberOfDIF_)
         printf("GTC %d %d  %\n",it_gtc->first,it_gtc->second.size(),theNumberOfDIF_);

    }
  theSync_.unlock();	
  usleep(500);
	

	
	
	


	
  return true;
}



void ShmProxy::save2DevShm(unsigned char* cbuf,uint32_t size_buf,uint32_t dif_shift,std::string memory_dir)
{
  std::stringstream s("");
  s<<memory_dir<<"/Event_"
    << ShmProxy::getBufferABCID(cbuf,dif_shift)<<"_"
    << ShmProxy::getBufferDTC(cbuf,dif_shift)<<"_"
    <<ShmProxy::getBufferGTC(cbuf,dif_shift)<<"_"
    <<ShmProxy::getBufferDIF(cbuf,dif_shift);
   int fd= ::open(s.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
   if (fd<0)
     {
   
       LOG4CXX_FATAL(_logShm," Cannot open shm file "<<s.str());
       perror("No way to store to file :");
       //std::cout<<" No way to store to file"<<std::endl;
       return;
     }
   int ier=write(fd,cbuf,size_buf);
   if (ier!=size_buf) 
     {
       LOG4CXX_FATAL(_logShm," Cannot write shm file "<<s.str());
       std::cout<<"pb in write "<<ier<<std::endl;
       return;
     }
   ::close(fd);
   std::stringstream st("");
   st<<memory_dir<<"/closed/"
    << ShmProxy::getBufferABCID(cbuf,dif_shift)<<"_"
    << ShmProxy::getBufferDTC(cbuf,dif_shift)<<"_"
    <<ShmProxy::getBufferGTC(cbuf,dif_shift)<<"_"
    <<ShmProxy::getBufferDIF(cbuf,dif_shift);


   fd= ::open(st.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
   //std::cout<<st.str().c_str()<<" "<<fd<<std::endl;
   //write(fd,b,1);
   ::close(fd);
}
void ShmProxy::run2DevShm(uint32_t &run,std::string memory_dir)
{
  std::stringstream s("");
  s<<memory_dir<<"/RUN";
  int fd= ::open(s.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
   if (fd<0)
     {
       perror("No way to store to file :");
       LOG4CXX_FATAL(_logShm," Cannot open shm file "<<s.str());
       //std::cout<<" No way to store to file"<<std::endl;
       return;
     }
   int ier=write(fd,&run,4);
   if (ier!=4) 
     {
       LOG4CXX_FATAL(_logShm," Cannot write shm file "<<s.str());
       std::cout<<"pb in write "<<ier<<std::endl;
       return;
     }
   ::close(fd);
}







#ifdef USE_DIM
void ShmProxy::configureDimServer()
{
  char name[80];
  memset(name,0,80);
  sprintf(name,"FU0/DAQ/Update");
  theUpdateCmd_= new DimCommand(name,"C",this);

  memset(name,0,80);
  sprintf(name,"FU0_SERVER");
  start(name);
  printf("DIM Server started %s \n",name);

}
void ShmProxy::updateDimServices()
{
  std::cout<<"Updating DIM services "<<theEventNumber_<<std::endl;
  for (std::map<uint32_t,DimService*>::iterator it=theDimServiceMap_.begin();it!=theDimServiceMap_.end();it++)
    {
      std::map<uint32_t,uint32_t*>::iterator itdim=theDimBufferMap_.find(it->first);
      if (itdim!=theDimBufferMap_.end())
	it->second->updateService(itdim->second,(itdim->second[0]+1)*sizeof(int32_t));
#undef DEBUG
#ifdef DEBUG
      unsigned char* cdif = (unsigned char*) itdim->second;
      uint32_t ib0=98;
      uint32_t idx=ib0;
	// for (int i=0;i<100;i++)
	//   printf("%.2x",cdif[ib0+i]);
	// printf("\n");
      idx++;
      uint32_t dif_id=cdif[idx];

  //if (dif_id==89) for (int i=0;i<max_size;i++) {printf("%02x",cdif[i]); if ((i+1)%100==0) 	printf("\n");}

  //if (dif_id==89)
  //printf("DIF ID %d \n",dif_id);

	idx++;
	uint32_t dtc,atc,gtc;
	dtc= (cdif[idx]<<24);
	dtc= dtc | (cdif[idx+1]<<16);
	dtc= dtc | (cdif[idx+2]<<8);
	dtc= dtc | (cdif[idx+3]);
	idx=idx+4;
	atc= (cdif[idx]<<24);
	atc= atc | (cdif[idx+1]<<16);
	atc= atc | (cdif[idx+2]<<8);
	atc= atc | (cdif[idx+3]);
	idx=idx+4;
	gtc= (cdif[idx]<<24);
	gtc= gtc | (cdif[idx+1]<<16);
	gtc= gtc | (cdif[idx+2]<<8);
	gtc= gtc | (cdif[idx+3]);
	std::cout<<"updating "<<dif_id<<" "<<dtc<<" "<<gtc<<std::endl;
#endif
    }
  serveEvents_=false;
}

std::map<uint32_t,uint32_t*>::iterator ShmProxy::findDimBufferIterator(uint32_t dif_id)
{
  std::map<uint32_t,uint32_t*>::iterator itdim=theDimBufferMap_.find(dif_id);
  if (itdim==theDimBufferMap_.end())
    {
      uint32_t* dbuf = new uint32_t[0x8000];
      char name[80];
      memset(name,0,80);
      sprintf(name,"FU%d/DAQ/DIF_%1d",0,dif_id);
      std::cout<<name <<" is created"<<std::endl;
      DimService* difack =  new DimService(name,"C",dbuf,0x20000);
      std::pair<uint32_t,DimService*> p(dif_id,difack);
      theDimServiceMap_.insert(p);
      std::pair<uint32_t,uint32_t*> pb(dif_id,dbuf);
      theDimBufferMap_.insert(pb);
      itdim=theDimBufferMap_.find(dif_id);
    }
  return itdim;
}

#endif
