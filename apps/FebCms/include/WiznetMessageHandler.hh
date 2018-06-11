#ifndef _WIZNET_MESSAGE_HANDLER_HH
#define _WIZNET_MESSAGE_HANDLER_HH
#include "MessageHandler.hh"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>


#include <map>
#include <string>
typedef std::pair<uint32_t,unsigned char*> ptrBuf;
typedef boost::function<void (uint64_t,uint16_t,char*)> FEBFunctor;

namespace lydaq
{
class WiznetMessageHandler : public lytdc::MessageHandler
  {
  public:
    WiznetMessageHandler(std::string directory);
    virtual void processMessage(NL::Socket* socket) throw (std::string);
    static uint32_t convertIP(std::string hname);
    void addHandler(uint64_t id,FEBFunctor f);
    void removeSocket(NL::Socket* socket);
    static std::map<uint32_t,std::string> scanNetwork(std::string base);
  private:
    std::string _storeDir;
    std::map<uint64_t, ptrBuf> _sockMap;
    std::map<uint64_t,FEBFunctor> _handlers;
    uint64_t _npacket;
  };
};
#endif
