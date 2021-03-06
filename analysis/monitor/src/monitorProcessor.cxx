#include "monitorProcessor.hh"
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

//#include "DIFReadoutConstant.h"
#include <iostream>
#include <sstream>
using namespace zdaq;
extern  int alphasort(); //Inbuilt sorting function  
#define FALSE 0  
#define TRUE !FALSE      
int file_select_3(const struct direct *entry)  
{  
  if ((strcmp(entry->d_name, ".") == 0) || (strcmp(entry->d_name, "..") == 0))  
    return (FALSE);  
  else  
    return (TRUE);  
}  

monitorProcessor::monitorProcessor() :_filepath("/dev/shm/monitor"),_started(false)
{


}


void monitorProcessor::start(uint32_t run)//,std::string dir,std::string setup)
{

  _run=run;
  _nheader=0;
  _started=true;
}
void monitorProcessor::processRunHeader(std::vector<uint32_t> header)
{
 if (!_started) return;
 
 uint32_t ibuf[256];
 for (int i=0;i<header.size();i++) ibuf[i]=header[i];
 // Construct one zdaq buffer with header content
 zdaq::buffer b(128+header.size());
 b.setDetectorId(255);
 b.setDataSourceId(_run);
 b.setEventId(_nheader++);
 b.setBxId(_run);
 b.setPayload(ibuf,header.size()*sizeof(uint32_t));
 unsigned char* cdata=(unsigned char*) b.ptr();
 int32_t* idata=(int32_t*) cdata;
 int difsize=b.size();

 this->store(b.detectorId(),b.dataSourceId(),
	     b.eventId(),b.bxId(),b.ptr(),b.size(),_filepath);

}
 
void monitorProcessor::loadParameters(Json::Value params)
{
      _filepath=params["shmpath"].asString();
}
void monitorProcessor::processEvent(uint32_t gtc,std::vector<zdaq::buffer*> vbuf)//writeEvent(uint32_t gtc,std::vector<unsigned char*> vbuf)
{
  if (!_started) return;
  uint32_t theNumberOfDIF=vbuf.size();

  std::vector<std::string> vnames;
  
  // list files in shm directory
  this->ls(_filepath,vnames);
  if (vnames.size()>20*theNumberOfDIF) return;

  if (gtc%100==0) 
    std::cout<<"Standard completion GTC "<<gtc<<std::endl;
  for (std::vector<zdaq::buffer*>::iterator iv=vbuf.begin();iv!=vbuf.end();iv++) 
    {
      (*iv)->uncompress();
      this->store((*iv)->detectorId(),(*iv)->dataSourceId(),
		   (*iv)->eventId(),(*iv)->bxId(),(*iv)->ptr(),(*iv)->size(),_filepath);
    }
   

}

void monitorProcessor::stop()
{

}

void monitorProcessor::ls(std::string sourcedir,std::vector<std::string>& res)
{
 
  res.clear();
  int count,i;  
  struct direct **files;  
  std::stringstream sc;
  sc.str(std::string());
  sc<<sourcedir<<"/closed/";
  
  count = scandir(sc.str().c_str(), &files, file_select_3, alphasort); 		
  /* If no files found, make a non-selectable menu item */  
  if(count <= 0)    {return ;}
       
  std::stringstream sd;		
  //printf("Number of files = %d\n",count);  
  for (i=1; i<count+1; ++i)  
    {
      // file name
      std::string fName;
      fName.assign(files[i-1]->d_name);
      res.push_back(fName);
       free(files[i-1]);
    }
  free(files);
  return;
}

void monitorProcessor::store(uint32_t detid,uint32_t sourceid,uint32_t eventid,uint64_t bxid,void* ptr,uint32_t size,std::string destdir)
{
  std::stringstream sc,sd;
  sc.str(std::string());
  sc<<destdir<<"/closed/";
  char name[512];
  memset(name,0,512);
  sprintf(name,"%s/Event_%u_%u_%u_%lu",destdir.c_str(),detid,sourceid,eventid,bxid);
  int fd= ::open(name,O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  if (fd<0)
    {
   
      //LOG4CXX_FATAL(_logShm," Cannot open shm file "<<s.str());
      perror("No way to store to file :");
      //std::cout<<" No way to store to file"<<std::endl;
      return;
    }
  int ier=write(fd,ptr,size);
  if (ier!=size) 
    {
      std::cout<<"pb in write "<<ier<<std::endl;
      return;
    }
  ::close(fd);
  memset(name,0,512);
  sprintf(name,"%s/closed/Event_%u_%u_%u_%lu",destdir.c_str(),detid,sourceid,eventid,bxid);
  fd= ::open(name,O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  //std::cout<<st.str().c_str()<<" "<<fd<<std::endl;
  //write(fd,b,1);
  ::close(fd);

}


extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  zdaq::zmprocessor* loadProcessor(void)
    {
      return (new monitorProcessor);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteProcessor(zdaq::zmprocessor* obj)
    {
      delete obj;
    }
}
