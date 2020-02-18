
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <sstream>
#include <string.h>
#include <stdio.h>
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

std::string lmexec(const char* cmd) {

  //fprintf(stderr,"Executing %s \n");
  FILE* pipe = popen(cmd, "r");
  if (!pipe) return "ERROR";
  char buffer[128];
  std::string result = "";
  while (!feof(pipe)) {
    if (fgets(buffer, 128, pipe) != NULL)
      result += buffer;
  }
  pclose(pipe);
  //fprintf(stderr,"On obtient: %s\n",result.c_str());
  return result;
  
}



std::map<uint32_t,std::string> scanNetwork(std::string base)
{
  std::map<uint32_t,std::string> m;
  std::stringstream ss;
  ss<<"echo $(seq 254) | xargs -P255 -I% -d\" \" ping -W 1 -c 1 "<<base<<"% | grep -E \"[0-1].*?:\" | awk '{print $4}' | awk 'BEGIN{FS=\":\"}{print $1}'";
  std::string res=lmexec(ss.str().c_str());
  std::cout<<"Return from scan of "<<base<<" : \n"<<res;
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
  
  
  std::cout<<host_list.size()<<" Hosts found:"<<std::endl;
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
int main(int argc, char *argv[])
{
  if (argc<2)
    exit(0);
  std::string base(argv[1]);
  std::map<uint32_t,std::string> m=scanNetwork(base);
 
}
