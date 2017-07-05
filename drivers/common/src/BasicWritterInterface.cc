#include "BasicWritterInterface.h"
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
#include "DIFReadoutConstant.h"
#include <iostream>
#include <sstream>

BasicWritterInterface::BasicWritterInterface() : theEventNumber_(0),theTotalSize_(0)
{
}
 
void BasicWritterInterface::openFile(uint32_t run,std::string dir,std::string setup)
{
  theDirectory_=dir;
 std::stringstream filename("");    
  char dateStr [64];
            
  time_t tm= time(NULL);
  strftime(dateStr,20,"SMM_%d%m%y_%H%M%S",localtime(&tm));
  filename<<theDirectory_<<"/"<<dateStr<<"_"<<setup<<".dat";
  fdOut_= ::open(filename.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  if (fdOut_<0)
    {
      perror("No way to store to file :");
      //std::cout<<" No way to store to file"<<std::endl;
      return;
    }  
}
void BasicWritterInterface::writeEvent(uint32_t gtc,std::vector<unsigned char*> vbuf)
 {
   uint32_t theNumberOfDIF=vbuf.size();
   if (theEventNumber_%100==0) 
     std::cout<<"Standard completion "<<theEventNumber_<<" GTC "<<gtc<<" size "<<theTotalSize_<<std::endl;
      // To be implemented
      if (fdOut_>0)
	{
	  int ier=write(fdOut_,&theEventNumber_,sizeof(uint32_t));
	  ier=write(fdOut_,&theNumberOfDIF,sizeof(uint32_t));
	  for (std::vector<unsigned char*>::iterator iv=vbuf.begin();iv!=vbuf.end();iv++) 
	    {
	      unsigned char* cdata=(*iv);
	      uint32_t* idata=(uint32_t*) cdata;
	      //printf("\t writing %d bytes",idata[SHM_BUFFER_SIZE]);
	      theTotalSize_+=idata[SHM_BUFFER_SIZE];
	      ier=write(fdOut_,&idata[SHM_BUFFER_SIZE],sizeof(uint32_t));
	      ier=write(fdOut_,cdata,idata[SHM_BUFFER_SIZE]);
	    }
		
		

	  theEventNumber_++;
	}

      if (theTotalSize_>500*1024*1024)
	{
	  ::close(fdOut_);
	  theTotalSize_=0;
	  openFile(0,theDirectory_);
	}

}

void BasicWritterInterface::closeFile()
{

 if (fdOut_>0)
    {
      ::close(fdOut_);
      fdOut_=-1;
    }


}
