#include "WiznetMessageHandler.hh"
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
#include "ReadoutLogger.hh"

#include <err.h>

using namespace lydaq;

#undef DEBUGBUF
#define LINELENGTH 8

/* C functions to execute command */
std::string lmexec(const char *cmd)
{
  FILE *pipe = popen(cmd, "r");
  if (!pipe)
    return "ERROR";
  char buffer[128];
  std::string result = "";
  while (!feof(pipe))
  {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  return result;
}

std::map<uint32_t, std::string> lydaq::WiznetMessageHandler::scanNetwork(std::string base)
{
  std::map<uint32_t, std::string> m;
  std::stringstream ss;
  ss << "echo $(seq 254) | xargs -P255 -I% -d\" \" ping -W 1 -c 1 " << base << "% | grep -E \"[0-1].*?:\" | awk '{print $4}' | awk 'BEGIN{FS=\":\"}{print $1}'";
  std::string res = lmexec(ss.str().c_str());
  LOG4CXX_INFO(_logFeb, "Ethernet board on " << base << " \n"
                                             << res);
  //getchar();
  std::stringstream ss1(res.c_str());
  std::string to;
  std::vector<std::string> host_list;
  if (res.c_str() != NULL)
  {
    while (std::getline(ss1, to, '\n'))
    {
      host_list.push_back(to);
    }
  }

  for (auto x : host_list)
  {
    struct in_addr ip;
    struct hostent *hp;

    if (!inet_aton(x.c_str(), &ip))
      errx(1, "can't parse IP address %s", x.c_str());

    if ((hp = gethostbyaddr((const void *)&ip, sizeof ip, AF_INET)) == NULL)
    {
      LOG4CXX_ERROR(_logFeb, x << " is unknown on the DNS");
    }
    else
    {
      LOG4CXX_INFO(_logFeb, x.c_str() << " is " << ip.s_addr << " name=" << hp->h_name);
      m.insert(std::pair<uint32_t, std::string>(ip.s_addr, x));
    }
  }
  return m;
}

lydaq::WiznetMessageHandler::WiznetMessageHandler(std::string directory) : _storeDir(directory), _npacket(0)
{
  _sockMap.clear();
}
void lydaq::WiznetMessageHandler::dumpShm(NL::Socket* socket,ptrBuf p)
{
  std::stringstream s;
  s<<_storeDir<<"/"<<socket->hostTo()<<"/"<<socket->portTo()<<"/data";
  int fd= ::open(s.str().c_str(),O_CREAT| O_RDWR | O_NONBLOCK,S_IRWXU);
  if (fd<0)
  {
    
    LOG4CXX_FATAL(_logFeb," Cannot open shm file "<<s.str());
    perror("No way to store to file :");
    return;
  }
  int ier=write(fd,&p.second[0],p.first);
  if (ier!=p.first) 
  {
    std::cout<<"pb in write "<<ier<<std::endl;
  }
  ::close(fd);

}
void lydaq::WiznetMessageHandler::processMessage(NL::Socket *socket)
{
  // build id

  uint64_t id = ((uint64_t)lydaq::WiznetMessageHandler::convertIP(socket->hostTo()) << 32) | socket->portTo();
  LOG4CXX_DEBUG(_logFeb, "Message received from " << socket->hostTo() << ":" << socket->portTo() << " =>" << std::hex << id);

  // Is the socket in the Map
  std::map<uint64_t, ptrBuf>::iterator itsock = _sockMap.find(id);

  // Add it
  if (itsock == _sockMap.end())
  {
    ptrBuf p(0, new unsigned char[0x100000]);
    std::pair<uint64_t, ptrBuf> p1(id, p);
    _sockMap.insert(p1);

    itsock = _sockMap.find(id);
    // Build subdir
    struct stat st = {0};
    std::stringstream s;
    s << _storeDir << "/" << socket->hostTo(); //<<"/"<<socket->portTo();
    std::cout << "Creating " << s.str() << std::endl;
    if (stat(s.str().c_str(), &st) == -1)
    {
      mkdir(s.str().c_str(), 0700);
    }
    s << "/" << socket->portTo();
    std::cout << "Creating " << s.str() << std::endl;
    if (stat(s.str().c_str(), &st) == -1)
    {
      mkdir(s.str().c_str(), 0700);
    }
  }

  // Pointers to the buffer
  ptrBuf &p = itsock->second;
  uint32_t *iptr = (uint32_t *)&p.second[0];
  uint16_t *sptr = (uint16_t *)&p.second[0];
  // Check

  // maximal size 16K
  size_t ier = 0;
  uint32_t size_remain = 16 * 1024;

  // read the socket
  while (size_remain > 0)
  {
    try
    {
      ier = socket->read(&p.second[0], size_remain);
    }
    catch (NL::Exception e)
    {
      LOG4CXX_ERROR(_logFeb, "Error reading socket " << e.msg().c_str());
      return;
    }
    if (ier < 0)
      break;
    _npacket++;
    //    if (_npacket%1000 ==1)
    LOG4CXX_DEBUG(_logFeb, " End of packet " << _npacket << " readout on socket " << std::hex << id << std::dec << " Bytes received" << ier);
    break;
    size_remain -= ier;
  }

  // Process  the buffer
  // Find the buffer handler
  p.first = ier;
  std::map<uint64_t, FEBFunctor>::iterator icmd = _handlers.find(id);
  if (icmd == _handlers.end())
  {
    LOG4CXX_ERROR(_logFeb, " No data handler  for socket id " << id << " readout on socket " << std::hex << id << std::dec);
    p.first = 0;
    return;
  }
  /// Call the functor
  icmd->second(id, p.first, (char *)p.second);
  /// Dmp to Shm
  this->dumpShm(socket,p);
  /// Reset the buffer
  p.first = 0;
  return;
}

void lydaq::WiznetMessageHandler::removeSocket(NL::Socket *socket)
{
  uint64_t id = ((uint64_t)lydaq::WiznetMessageHandler::convertIP(socket->hostTo()) << 32) | socket->portTo();
  std::map<uint64_t, ptrBuf>::iterator itsock = _sockMap.find(id);
  if (itsock == _sockMap.end())
    return;
  delete itsock->second.second;
  _sockMap.erase(itsock);
}

uint32_t lydaq::WiznetMessageHandler::convertIP(std::string hname)
{
  struct hostent *he;
  struct in_addr **addr_list;
  int i;
  char ip[100];
  if ((he = gethostbyname(hname.c_str())) == NULL)
  {
    return 0;
  }

  addr_list = (struct in_addr **)he->h_addr_list;

  for (i = 0; addr_list[i] != NULL; i++)
  {
    //Return the first one;
    strcpy(ip, inet_ntoa(*addr_list[i]));
    break;
  }

  in_addr_t ls1 = inet_addr(ip);
  return (uint32_t)ls1;
}
void lydaq::WiznetMessageHandler::addHandler(uint64_t id, FEBFunctor f)
{
  std::pair<uint64_t, FEBFunctor> p(id, f);
  _handlers.insert(p);
}
