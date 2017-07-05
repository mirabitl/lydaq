#include "DIFDBInterface.hh"
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/dir.h>  
#include <sys/param.h>  
#include <stdio.h>  
#include <stdlib.h>  
#include <fcntl.h>
#include <string.h>

using namespace lydaq;

lydaq::DIFDBInterface::DIFDBInterface()
{
  theDIFs_.clear();
  theDifMap_.clear();
  theAsicsMap_.clear();
  theAsicKeyMap_.clear();
  theNumberOfAsicsMap_.clear();

  
  //std::cout<<tlist<<" gives "<<this->getNumberOfDIFs()<<std::endl;
}


void lydaq::DIFDBInterface::clearMaps()
{
  for (std::map<uint32_t,UsbDIFSettings*>::iterator it=theDifMap_.begin();it!=theDifMap_.end();it++)
    delete it->second;
  theDifMap_.clear();
  for (std::map<uint32_t,SingleHardrocV2ConfigurationFrame*>::iterator it=theAsicsMap_.begin();it!=theAsicsMap_.end();it++)
    delete it->second;
  for (std::map<uint32_t,unsigned char*>::iterator it=theAsicKeyMap_.begin();it!=theAsicKeyMap_.end();it++)
    delete it->second;


  theAsicsMap_.clear();
  theAsicKeyMap_.clear();
  theNumberOfAsicsMap_.clear();
    
}

void lydaq::DIFDBInterface::dumpAsicMap()
{
  for (std::map<uint32_t,unsigned char*>::iterator it=theAsicKeyMap_.begin();it!=theAsicKeyMap_.end();it++)
    {
      uint32_t d,a,s;
      d=(it->first>>8)&0xff;
      a=(it->first)&0xff;
      unsigned char* buf =it->second;
      s=buf[0];
      printf("DIF %d ASIC %d Size %d \n",d,a,s);
      for (int i=0;i<s;i++)
	printf ("%02x",buf[1+i]);
      printf ("\n");
    }
}

void lydaq::DIFDBInterface::download(std::string asicType)
{
  uint8_t nasics=0; // cc02032012
  uint8_t nasicsl1=0;
  uint8_t nasicsl2=0;
  uint8_t nasicsl3=0;
  uint8_t nasicsl4=0;
  uint8_t tamponl=0;

  this->clearMaps();
  std::cout<<"Full download "<<std::endl;
  this->LoadAsicParameters();
  std::cout<<"Specific DIF download "<<std::endl;
  printf ("nb difs = %d \n",this->getNumberOfDIFs());

  LOG4CXX_INFO(_logDb," Download nb DIF="<<this->getNumberOfDIFs());

  for (uint8_t idx=0;idx<this->getNumberOfDIFs();idx++)
    {
      // Create and fill DIF settings
      uint32_t difid= this->getDIFId(idx);
      UsbDIFSettings* ud=new UsbDIFSettings(difid);
      this->LoadDIFDefaultParameters(this->getDIFId(idx),ud);
      if (!ud->Masked)
	{
	  std::pair<uint32_t,UsbDIFSettings*> p(difid,ud);
	  theDifMap_.insert(p);
	  // Fill Asics for this DIF
	  SingleHardrocV2ConfigurationFrame* v = new  SingleHardrocV2ConfigurationFrame[MAX_NB_OF_ASICS];
	  //#define OLDWAY

	  nasics=0; // cc02032012
	  nasicsl1=0;
	  nasicsl2=0;
	  nasicsl3=0;
	  nasicsl4=0;
	  tamponl=0;

	  for (std::map<uint32_t,unsigned char*>::iterator ik=theAsicKeyMap_.begin();ik!=theAsicKeyMap_.end();ik++)
	    {
	      // printf("New key %x \n",ik->first);
	      if (((ik->first>>8)&0xFF)==difid) 
		{
		  //			printf ("ik->second[0] = %d\n",ik->second[0]);
		  if (ik->second[0]==109) //HR2
		    {
		      nasics++;
		      nasicsl1++;
		    }	
		  else if (ik->second[0]==74) //MR
		    {
		      nasics++;
		      if (nasics%8==0) tamponl=1;
		      else if (nasics%8<5) tamponl=(nasics%8);
		      else tamponl=9-nasics%8;
		      if (tamponl==1) nasicsl1++;
		      else if (tamponl==2) nasicsl2++;
		      else if (tamponl==3) nasicsl3++;
		      else if (tamponl==4) nasicsl4++;
		    }
		}
	      //				nasics++;
	    }
	  std::pair<uint32_t,SingleHardrocV2ConfigurationFrame*> pa(difid,v);
	  theAsicsMap_.insert(pa);
	  printf ("difdbmanager difid =%d  nasics=%d %d %d %d %d\n",difid,nasics,nasicsl1,nasicsl2,nasicsl3,nasicsl4);
	  LOG4CXX_INFO(_logDb," Found "<<difid<<" Asic "<<nasics<<" "<<nasicsl1<<" "<<nasicsl2<<" "<<nasicsl3<<" "<<nasicsl4);
	  std::pair<uint32_t,uint32_t> pn(difid,nasicsl1+(nasicsl2<<8)+(nasicsl3<<16)+(nasicsl4<<24));
	  theNumberOfAsicsMap_.insert(pn);
	}
    }
}

uint8_t lydaq::DIFDBInterface::getAsicsNumber(uint32_t difid)
{
  uint8_t tnbl1, tnbl2,tnbl3,tnbl4;
  tnbl1=theNumberOfAsicsMap_[difid]&0xFF;
  tnbl2=(theNumberOfAsicsMap_[difid]>>8)&0xFF;
  tnbl3=(theNumberOfAsicsMap_[difid]>>16)&0xFF;
  tnbl4=(theNumberOfAsicsMap_[difid]>>24)&0xFF;
  printf ("in DIFDBInterface theNumberOfAsicsMap_[%d]=%d \n",difid,theNumberOfAsicsMap_[difid]);
  return tnbl1+tnbl2+tnbl3+tnbl4;
}
	
uint8_t lydaq::DIFDBInterface::getAsicsNumberl1(uint32_t difid)
{
  return theNumberOfAsicsMap_[difid]&0xFF;
}
uint8_t lydaq::DIFDBInterface::getAsicsNumberl2(uint32_t difid)
{
  return (theNumberOfAsicsMap_[difid]>>8)&0xFF;
}
uint8_t lydaq::DIFDBInterface::getAsicsNumberl3(uint32_t difid)
{
  return (theNumberOfAsicsMap_[difid]>>16)&0xFF;
}
uint8_t lydaq::DIFDBInterface::getAsicsNumberl4(uint32_t difid)
{
  return (theNumberOfAsicsMap_[difid]>>24)&0xFF;
}


void lydaq::DIFDBInterface::dumpToTree(std::string rootPath,std::string setupName)
{
  DIFDbInfo _difInfos[255];
  memset(_difInfos,0,255*sizeof(DIFDbInfo));
  std::map<uint32_t,unsigned char*> dbm=this->getAsicKeyMap();
  std::stringstream s("");
  s<<rootPath<<"/"<<setupName;
  int status = mkdir(s.str().c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
  std::cout<<status<<" "<<s.str()<<std::endl;

  for (std::map<uint32_t,unsigned char*>::iterator idb=dbm.begin();idb!=dbm.end();idb++)
    {
      uint32_t id = (idb->first>>8)&0xFF;
      _difInfos[id].id=id;
    }
  for (uint32_t i=0;i<255;i++)
    {

      if (_difInfos[i].id==0) continue;
      uint32_t id=i;
      _difInfos[id].nbasic=0;
      for (uint32_t iasic=1;iasic<=48;iasic++)
        {
          uint32_t key=(id<<8)|iasic;
          std::map<uint32_t,unsigned char*>::iterator it=dbm.find(key);
          if (it==dbm.end()) continue;
          unsigned char* bframe=it->second;
          uint32_t       framesize=bframe[0];
          memcpy(&_difInfos[id].slow[_difInfos[id].nbasic],&bframe[1],framesize);
          _difInfos[id].nbasic++;
        }
      std::stringstream sf("");
      sf<<s.str()<<"/"<<id;
      int fd= ::open(sf.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
      if (fd<0)
	{
	  perror("No way to store to file :");
	  //std::cout<<" No way to store to file"<<std::endl;
	  return;
	}
      int ier=write(fd,&_difInfos[id],2*sizeof(uint32_t)+_difInfos[id].nbasic*sizeof(SingleHardrocV2ConfigurationFrame));
      if (ier<=0) 
	{
	  LOG4CXX_ERROR(_logDb,"pb in write");
	  std::cout<<"pb in write "<<ier<<std::endl;
	  return;
	}
      else
	std::cout<<"Dumping "<<_difInfos[id].id<<" to "<<sf.str()<<std::endl;
      ::close(fd);

    }



				
	 
}

