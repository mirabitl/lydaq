#include "TdcMessageHandler.hh"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <boost/format.hpp>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/signal.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <vector>
#include <map>

#include <err.h>
using namespace zdaq;
using namespace lydaq;

#undef DEBUGBUF
#define LINELENGTH 8

std::string lmexec(const char* cmd) {
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
  
}



std::map<uint32_t,std::string> lydaq::TdcMessageHandler::scanNetwork(std::string base)
{
  std::map<uint32_t,std::string> m;
  std::stringstream ss;
  ss<<"echo $(seq 254) | xargs -P255 -I% -d\" \" ping -W 1 -c 1 "<<base<<"% | grep -E \"[0-1].*?:\" | awk '{print $4}' | awk 'BEGIN{FS=\":\"}{print $1}'";
  std::string res=lmexec(ss.str().c_str());
  std::cout<<"On obtinet \n"<<res;
  //getchar();
  std::stringstream ss1(res.c_str());
  std::string to;
  std::vector<std::string> host_list;
  if (res.c_str() != NULL)
  {
    while(std::getline(ss1,to,'\n')){
      host_list.push_back(to);
    }
  }
  
  
  //std::cout<<host_list.size()<<std::endl;
  for (auto x: host_list)
  {
    //std::cout<<x<<std::endl;
    
    struct in_addr ip;
    struct hostent *hp;
    
    if (!inet_aton(x.c_str(), &ip))
      errx(1, "can't parse IP address %s", x.c_str());
    
    if ((hp = gethostbyaddr((const void *)&ip, sizeof ip, AF_INET)) == NULL)
      printf("\t  %s is not known on the DNS \n", x.c_str());
    else
    {
      printf("%s is %x  %s\n", x.c_str(),ip.s_addr, hp->h_name);
      //m.insert(std::pair<uint32_t,std::string>(ip.s_addr,std::string(hp->h_name)));
      m.insert(std::pair<uint32_t,std::string>(ip.s_addr,x));
    }
      
      
      
      
      
  }
  return m;
}


lydaq::TdcMessageHandler::TdcMessageHandler(std::string directory) : _storeDir(directory)
{
  _sockMap.clear();
  for (int i=0;i<MAX_TDC_NB;i++)
    _tdc[i]=NULL; 
}
void lydaq::TdcMessageHandler::connect(zmq::context_t* c,std::string dest)
{
  for (int i=0;i<MAX_TDC_NB;i++)
     if (_tdc[i]!=NULL) _tdc[i]->connect(c,dest);
  
    
}
void lydaq::TdcMessageHandler::setMezzanine(std::string host)
{
  uint32_t ip_address=lydaq::TdcMessageHandler::convertIP(host);
  uint8_t idx=(ip_address>>24)&0xFF;
  if (_tdc[idx]==NULL)
    {
      uint32_t ip_address=lydaq::TdcMessageHandler::convertIP(host);
      std::cout<<" New TDC Mezzanine "<<idx<<" "<<ip_address<<std::endl;
      _tdc[idx]=new TdcFpga(idx,ip_address);
      _tdc[idx]->setStorage("/tmp");
    }
}
void lydaq::TdcMessageHandler::parseTdcData(NL::Socket* socket,ptrBuf& p) throw (std::string)
{
  // found TDC
  std::stringstream sd;
  sd<<_storeDir<<"/"<<socket->hostTo()<<"/"<<socket->portTo();
  //sd<<_storeDir;
  uint32_t ip_address=lydaq::TdcMessageHandler::convertIP(socket->hostTo());
  uint8_t difidx= (ip_address>>24)&0xFF;
  
  // Number of TDC channels, ie, channel of trigger
  uint32_t chtrg=28;
 
  if (_tdc[difidx]==NULL)
    {
  
      _tdc[difidx]=new TdcFpga(difidx,ip_address);
      _tdc[difidx]->setStorage(sd.str());
    }
  
  std::stringstream ss;
  uint64_t idsock=( (uint64_t) ip_address<<32)|socket->portTo();
  //return;
  //ss<<boost::format("Parsing Tdc Data ================> from %d %lx %d \n") % mezz % idsock % _readout[idsock] ;
  
  //  return;

#define NODUMPCHANNELS
#ifdef DEBUGBUF
  std::cout<<ss.str()<<std::endl;
#endif  
  size_t ier=0;
  uint32_t* iptr=(uint32_t*) &p.second[0];
  uint16_t* sptr=(uint16_t*) &p.second[0];

    // Now read length
  uint8_t linelength=0;
  uint32_t size_remain=linelength;
  
  while (size_remain>0)
  {
    try 
    {
      ier=socket->read(&p.second[p.first+6-size_remain],size_remain);
    }
    catch (NL::Exception e)
    {
      printf("%s Error message when reading block %s \n",__PRETTY_FUNCTION__,e.msg().c_str());
      p.first=0;
      return;
    }
    if (ier<0)
      break;
    size_remain -=ier;
    
  }
  p.first+=linelength;
  #ifdef DUMPCHANNELS
  ss<<"Header \n ";
  #endif
  p.first=LINELENGTH;
  linelength=LINELENGTH;
#ifdef DUMPCHANNELS
  for (int ib=0;ib<linelength-4;ib++)
    {
    ss<<boost::format("%.2x ") % static_cast<int>(p.second[4+ib]);
    }
    
  ss<<"\n";
#endif
  #ifdef DEBUGBUF
  std::cout<<ss.str()<<std::endl;
  getchar();
  #endif
  uint32_t length=(ntohs(sptr[1+linelength/2]))&0xFFFF;
  length=p.second[LINELENGTH-1];
#ifdef DUMPCHANNELS
  ss<<boost::format("Expected length here %d %d %d \n") % length % length % p.first;
#endif
  // Now read payload
  uint8_t byteslen=LINELENGTH;
  uint32_t elen=length*byteslen;
  size_remain=elen;
  //getchar();
#ifdef DEBUGBUF
  std::cout<<ss.str()<<std::endl;
  //getchar();
#endif
  while (size_remain>0)
  {
    try 
    {
      ier=socket->read(&p.second[p.first+elen-size_remain],size_remain);
      //printf("socket read here %d \n",ier);
    }
    catch (NL::Exception e)
    {
      printf("%s Error message when reading block %s \n",__PRETTY_FUNCTION__,e.msg().c_str());
      return;
    }
    if (ier<0)
      break;
    size_remain -=ier;
    
  }
  p.first+=elen;
#ifdef DUMPCHANNELS
  ss<<boost::format("End of data readout %d bytes read \n") % p.first;
  for (int ib=0;ib<p.first;ib++)
    {
    ss<<boost::format("%.2x ") % static_cast<int>(p.second[ib]);
    if (ib%8==7)
      {
	uint8_t f=p.second[ib];
	uint8_t c=p.second[ib-1];
	ss<<boost::format("-> %7.2f  ") % (c*2.5+f*2.5/256);
	ss<<std::endl;
      }
    }
  ss<<"\n";
  std::cout<<ss.str()<<std::endl;
#endif
  // Found the GTC
  uint8_t* buf=p.second;
  uint8_t  ll=8;
  uint32_t nlines=buf[7];
  uint64_t abcid=buf[ll+ll-1]|((uint64_t) buf[ll+ll-2]<<8)|((uint64_t) buf[ll+ll-3]<<16)|((uint64_t) buf[ll+ll-4]<<24)|((uint64_t) buf[ll+ll-5]<<32)|((uint64_t)buf[ll+ll-6]<<40);
  uint32_t gtc= buf[ll+1]|((uint32_t) buf[ll]<<8);
  if (gtc%500==0)
    printf("End of data readout Mezzanine %d ,%d bytes read, ABCID %lx , GTC %d lines %d  \n", difidx,p.first,abcid,gtc,nlines);

  // TEST
  _tdc[difidx]->addChannels(p.second,p.first);
  p.first=0;
  return;

  /// FINIFINIFINI
  /// FINIFINIFINI
  /// FINIFINIFINI


  
  // p.first=0;return;
  bool printed=false;
  uint32_t bcid=0;
   for (int ib=0;ib<elen;ib++)
     {
       //printf("%.2x ",p.second[LINELENGTH+ib]);
       if (ib%LINELENGTH==0 && (p.second[LINELENGTH+ib]==chtrg)) {
	 bcid= (p.second[LINELENGTH+ib+1]<<8|p.second[LINELENGTH+ib+2]);
	 //ss<<boost::format("EXTERNAL %x \n") % bcid;
	 // printed=true;
       }
     }

   uint32_t sbcid=0;
   for (int ib=0;ib<elen;ib++)
     {
       //ss<<boost::format("%.2x ") % static_cast<int>(p.second[LINELENGTH+ib]);
       if (ib>0 && ib%LINELENGTH==LINELENGTH-1)
	 {
	   sbcid=(p.second[LINELENGTH+ib-LINELENGTH-2]<<8|p.second[LINELENGTH+ib-LINELENGTH-3]);
	   if (sbcid>bcid-3 && sbcid<bcid+3 && p.second[LINELENGTH+ib-LINELENGTH-1]!=chtrg  )
	     ss<<"--------------> found \n";
	   else
	     ss<<"\n";
	 }
     }
   ss<<"\n";
  //    #ifdef TOTO
  //  std::cout<<ss.str()<<std::endl;
  // getchar();
  // return;
  // #endif

   if (printed) std::cout<<ss.str()<<std::flush;
   //return;
  _readout[idsock]++;
  // Now store the event
  #ifdef OLDSOFTWARE
  std::stringstream s;
  s<<_storeDir<<"/"<<socket->hostTo()<<"/"<<socket->portTo()<<"/event_"<<_readout[idsock];
  int fd= ::open(s.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  if (fd<0)
  {
    
    //LOG4CXX_FATAL(_logShm," Cannot open shm file "<<s.str());
    perror("No way to store to file :");
    //std::cout<<" No way to store to file"<<std::endl;
    p.first=0;
    return;
  }
  ier=write(fd,&p.second[0],p.first);
  if (ier!=p.first) 
  {
    std::cout<<"pb in write "<<ier<<std::endl;
    p.first=0;
    return;
  }
  ::close(fd);
 
  
  #endif
  //
  p.first=0;
  return;
}
void lydaq::TdcMessageHandler::parseSlowControl(NL::Socket* socket,ptrBuf& p) throw (std::string)
{

  std::cout<<boost::format("Parsing  SLOW CONTROL ================>\n")<<std::flush;
    //return;
  size_t ier=0;
  uint32_t* iptr=(uint32_t*) &p.second[0];
  uint16_t* sptr=(uint16_t*) &p.second[0];
  uint32_t size_remain=1024;
  ier=socket->read(&p.second[p.first],size_remain);
  if (ier<0) return;
  if (ier>1024) return;
  p.first+=ier;

  std::cout<<boost::format("End of Slowcontrol %d %x %x \n") % p.first % (int) p.second[6] % (int) p.second[7] << std::flush;
  std::stringstream sb;
   for (int ib=0;ib<p.first;ib++)
     {
       std::cout<<boost::format("\t %d %x \n") % ib % (int) p.second[ib];
       if (ib>=8) sb<<std::hex<<(int) p.second[ib];
       //if (ib>0 && ib%2==1) printf("\n");
     }
   std::cout<<boost::format("\n")<<std::flush;
   
 
  char name[512];
  memset(name,0,512);
  sprintf(name,"%s/%s/%d/%s",_storeDir.c_str(),socket->hostTo().c_str(),socket->portTo(),sb.str().c_str());
  printf(" removing %s \n",name);
  if (::unlink(name)!=0)
    printf("slowcontrol error\n");
  
  p.first=0;
  return;

}
void lydaq::TdcMessageHandler::processMessage(NL::Socket* socket) throw (std::string)
{
  // build id

  uint64_t id=( (uint64_t) lydaq::TdcMessageHandler::convertIP(socket->hostTo())<<32)|socket->portTo();
  //std::cout<<"Message received from "<<socket->hostTo()<<":"<<socket->portTo()<<" =>"<<std::hex<<id<<std::dec<<std::endl;
  std::map<uint64_t, ptrBuf>::iterator itsock=_sockMap.find(id);

  if (itsock==_sockMap.end())
  {
    ptrBuf p(0,new unsigned char[0x100000]);
    std::pair<uint64_t, ptrBuf> p1(id,p);
    _sockMap.insert(p1);
    std::pair<uint64_t, uint32_t> pr1(id,0);
    _readout.insert(pr1);
    
    itsock=_sockMap.find(id);
    // Build subdir
    struct stat st = {0};
    std::stringstream s;
    s<<_storeDir<<"/"<<socket->hostTo();//<<"/"<<socket->portTo();
    std::cout<<"Creating "<<s.str()<<std::endl;
    if (stat(s.str().c_str(), &st) == -1) {
      mkdir(s.str().c_str(), 0700);
    }
    s<<"/"<<socket->portTo();
    std::cout<<"Creating "<<s.str()<<std::endl;
    if (stat(s.str().c_str(), &st) == -1) {
      mkdir(s.str().c_str(), 0700);
    }
    std::map<uint64_t,uint32_t>::iterator itrd=_readout.find(id);
    // std::stringstream sf;
    // sf<<_storeDir<<"/"<<socket->hostTo()<<"/"<<socket->portTo()<<"/events";
    // _fd= ::open(s.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
    // if (_fd<0)
    //   {
    
    // 	//LOG4CXX_FATAL(_logShm," Cannot open shm file "<<s.str());
    // 	perror("No way to store to file :");
    // //std::cout<<" No way to store to file"<<std::endl;
    // 	return;
    //   }

  }




  
  ptrBuf &p=itsock->second;
  uint32_t* iptr=(uint32_t*) &p.second[0];
  uint16_t* sptr=(uint16_t*) &p.second[0];
  // Check


  
  size_t ier=0;
  uint32_t size_remain=LINELENGTH;
  // uint8_t temp[8];
  // ier=socket->read(temp,8);
  // for (int ib=0;ib<8;ib++)
  //   printf("%.2x ",temp[ib]);
  // printf("\n");
  // //getchar();
  // return;
  while (size_remain>0)
  {
    try 
    {
      ier=socket->read(&p.second[LINELENGTH-size_remain],size_remain);
    }
    catch (NL::Exception e)
    {
      printf("%s Error message when reading block %s \n",__PRETTY_FUNCTION__,e.msg().c_str());
      return;
    }
    if (ier<0)
      break;
    break;
    size_remain -=ier;
    
  }
  p.first+=LINELENGTH;
  if (ntohl(iptr[0]) !=0XCAFEBABE && ntohl(iptr[0]) !=0Xefacebea)
  {
    #ifdef DEBUGBUFFER
    printf("%s Invalid frame header %x  from %lx \n",__PRETTY_FUNCTION__,ntohl(iptr[0]),id);
    //ier=socket->read(&p.second[4-size_remain],1);
  //   uint8_t temp[8];
  // ier=socket->read(temp,7);
   for (int ib=0;ib<ier;ib++)
     printf("%.2x ",p.second[ib]);
   printf("\n");
   #endif
   //getchar();
    p.first=0;
    return;
  }
  if ( ntohl(iptr[0]) ==0Xefacebea)
    {
      this->parseSlowControl(socket,p);
      return;
  }

  if ( ntohl(iptr[0]) ==0XCAFEBABE)
    {

   //     for (int ib=0;ib<ier;ib++)
   //   printf("%.2x ",p.second[ib]);
   // printf("\n");
   // getchar();
      this->parseTdcData(socket,p);
      return;
  }
  
  // Now read length
  size_remain=6;
  while (size_remain>0)
  {
    try 
    {
      ier=socket->read(&p.second[p.first+6-size_remain],size_remain);
    }
    catch (NL::Exception e)
    {
      printf("%s Error message when reading block %s \n",__PRETTY_FUNCTION__,e.msg().c_str());
      p.first=0;
      return;
    }
    if (ier<0)
      break;
    size_remain -=ier;
    
  }
  p.first+=6;
  printf("Header \n ");
  
  for (int ib=0;ib<6;ib++)
    printf("%.2x ",p.second[4+ib]);
  printf("\n");
  //getchar();
  uint32_t length=(ntohs(sptr[4]))&0xFFFF;
  printf("Expected length here %d %d %d \n",length,length,p.first);
  // Now read payload
  uint32_t elen=length*10;
  size_remain=elen;
  //getchar();
  while (size_remain>0)
  {
    try 
    {
      ier=socket->read(&p.second[p.first+elen-size_remain],size_remain);
      printf("socket read here %d \n",ier);
    }
    catch (NL::Exception e)
    {
      printf("%s Error message when reading block %s \n",__PRETTY_FUNCTION__,e.msg().c_str());
      return;
    }
    if (ier<0)
      break;
    size_remain -=ier;
    
  }
  p.first+=elen;
  printf("End of readout %d \n",p.first);
   for (int ib=0;ib<elen;ib++)
     {
    printf("%.2x ",p.second[16+ib]);
   if (ib>0 && ib%10==9) printf("\n");
     }
  printf("\n");
  //  _readout++;
  // Now store the event
#define OLDWAY
#ifdef OLDWAY
  std::stringstream s;
  //s<<_storeDir<<"/"<<socket->hostTo()<<"/"<<socket->portTo()<<"/event_"<<_readout;
  int fd= ::open(s.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  if (fd<0)
  {
    
    //LOG4CXX_FATAL(_logShm," Cannot open shm file "<<s.str());
    perror("No way to store to file :");
    //std::cout<<" No way to store to file"<<std::endl;
    p.first=0;
    return;
  }
  ier=write(fd,p.second,p.first);
  if (ier!=p.first) 
  {
    std::cout<<"pb in write "<<ier<<std::endl;
    p.first=0;
    return;
  }
  ::close(fd);
#else
  unsigned char bup[0x10000];
  memcpy(bup,p.second,p.first);
  // uint32_t* lbuf=(uint32_t*) p.second;
  // uint16_t* sbuf=(uint16_t*) p.second;
  // for (int i=0;i<p.first/4;i++)
  //   {
  //    uint32_t ll=ntohl(lbuf[i]);
  //    //lbuf[i]=ntohl(lbuf[i]);
  //    // printf("%x ntohl %x \n",lbuf[i],ll) ;     
  //   }
  // //getchar();
  // for (int i=0;i<p.first/2;i++)
  //   {
  //    uint16_t ll=ntohs(sbuf[i]);
  //    //lbuf[i]=ntohl(lbuf[i]);
  //    //printf("%x ntohl %x \n",sbuf[i],ll);      
  //   }
  // //getchar();
  // unsigned char* hbuf=(unsigned char*) &sbuf[4];
  std::stringstream s;
  s<<_storeDir<<"/"<<socket->hostTo()<<"/"<<socket->portTo()<<"/events";
  FILE * fp = fopen (s.str().c_str(), "a+");
  for (int i=0;i<p.first;)
    {
     
      uint64_t fr[8];
      for (int j=0;j<8;j++)
	fr[j]=bup[i+8+j]&0XFF;
      
      
      // it=fr[3]|(fr[4]<<8)|(fr[5]<<16)|(fr[6]<<24)|(fr[7]<<32);
      // double rti=it*2.5/256;
      // fprintf(fp,"%d,%llx,%f\n",ch,it,rti);
      // printf("%x,%d,%d,%llx,%f,%llx\n",fr[2],i,ch,it,rti,(fr[7]<<32));
      uint8_t ch=(fr[2]&0XFF)/2;
      uint64_t it=fr[7]|(fr[6]<<8)|(fr[5]<<16)|(fr[4]<<24)|(fr[3]<<32);
      double rti=it*2.5/256;
      //printf("%x,%d,%d,%llx,%f,%llx\n",fr[2],i,ch,it,rti,(fr[7]<<32));
      fprintf(fp,"%d,%llx,%f\n",ch,it,rti);
      i+=8;
    }
  //getchar();
  fclose(fp);
#endif
  p.first=0;
}
void lydaq::TdcMessageHandler::removeSocket(NL::Socket* socket)
{
  uint64_t id=((uint64_t) lydaq::TdcMessageHandler::convertIP(socket->hostTo())<<32)|socket->portTo();
  std::map<uint64_t, ptrBuf>::iterator itsock=_sockMap.find(id);
  if (itsock==_sockMap.end()) return;
  delete itsock->second.second;
  _sockMap.erase(itsock);
}
uint32_t lydaq::TdcMessageHandler::convertIP(std::string hname)
{
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
  char ip[100];
  if ( (he = gethostbyname( hname.c_str() ) ) == NULL) 
  {
    return 0;
  }
  
  addr_list = (struct in_addr **) he->h_addr_list;
  
  for(i = 0; addr_list[i] != NULL; i++) 
  {
    //Return the first one;
    strcpy(ip , inet_ntoa(*addr_list[i]) );
    break;
  }
  
  in_addr_t ls1=inet_addr(ip);
  return (uint32_t) ls1;
}
