#include "MpiMessageHandler.hh"
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



std::map<uint32_t,std::string> lydaq::MpiMessageHandler::scanNetwork(std::string base)
{
  std::map<uint32_t,std::string> m;
  std::stringstream ss;
  ss<<"echo $(seq 254) | xargs -P255 -I% -d\" \" ping -W 1 -c 1 "<<base<<"% | grep -E \"[0-1].*?:\" | awk '{print $4}' | awk 'BEGIN{FS=\":\"}{print $1}'";
  std::string res=lmexec(ss.str().c_str());
  std::cout<<"Ethernet board on "<<base <<" \n"<<res;
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


lydaq::MpiMessageHandler::MpiMessageHandler(std::string directory) : _storeDir(directory),_npacket(0)
{
  _sockMap.clear();

}

void lydaq::MpiMessageHandler::processMessage(NL::Socket* socket) throw (std::string)
{
  // build id

  uint64_t id=( (uint64_t) lydaq::MpiMessageHandler::convertIP(socket->hostTo())<<32)|socket->portTo();
  std::cout<<"Message received from "<<socket->hostTo()<<":"<<socket->portTo()<<" =>"<<std::hex<<id<<std::dec<<std::endl;
  std::map<uint64_t, ptrBuf>::iterator itsock=_sockMap.find(id);

  if (itsock==_sockMap.end())
  {
    ptrBuf p(0,new unsigned char[0x100000]);
    std::pair<uint64_t, ptrBuf> p1(id,p);
    _sockMap.insert(p1);
    
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
  uint32_t size_remain=16*1024;
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
      ier=socket->read(&p.second[0],size_remain);
    }
    catch (NL::Exception e)
    {
      printf("%s Error message when reading block %s \n",__PRETTY_FUNCTION__,e.msg().c_str());
      return;
    }
    if (ier<0)
      break;
    _npacket++;
    // if (_npacket%1000 ==1)
    fprintf(stderr,"%s Packet %d Receive %d bytes from %lx \n",__PRETTY_FUNCTION__,_npacket,ier,id);
    break;
    size_remain -=ier;
    
  }
  p.first=ier;
  std::map<uint64_t,FEBFunctor >::iterator icmd=_handlers.find(id);
  if (icmd==_handlers.end())
    {
      printf("%s No data handler for socket id %ld \n",__PRETTY_FUNCTION__,id);
      p.first=0;
      return;
          
    }
  icmd->second(id,p.first,(char*) p.second);
  p.first=0;
  return;
}
void lydaq::MpiMessageHandler::removeSocket(NL::Socket* socket)
{
  uint64_t id=((uint64_t) lydaq::MpiMessageHandler::convertIP(socket->hostTo())<<32)|socket->portTo();
  std::map<uint64_t, ptrBuf>::iterator itsock=_sockMap.find(id);
  if (itsock==_sockMap.end()) return;
  delete itsock->second.second;
  _sockMap.erase(itsock);
}

uint32_t lydaq::MpiMessageHandler::convertIP(std::string hname)
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
void lydaq::MpiMessageHandler::addHandler(uint64_t id,FEBFunctor f)
{
  std::pair<uint64_t,FEBFunctor> p(id,f);
  _handlers.insert(p);
}
