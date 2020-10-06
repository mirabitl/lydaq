#ifndef _MPI_MESSAGE_HANDLER_HH
#define _MPI_MESSAGE_HANDLER_HH
#include "MessageHandler.hh"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>


#include <map>
#include <string>
typedef std::pair<uint32_t,unsigned char*> ptrBuf;
typedef boost::function<void (uint64_t,uint16_t,char*)> MPIFunctor;

namespace lydaq
{
class MpiMessageHandler : public mpi::MessageHandler
  {
  public:
    MpiMessageHandler(std::string directory);
    virtual void processMessage(NL::Socket* socket);// throw (mpi::MpiException);
    static uint32_t convertIP(std::string hname);
    void addHandler(uint64_t id,MPIFunctor f);
    void removeSocket(NL::Socket* socket);
    static std::map<uint32_t,std::string> scanNetwork(std::string base);
  private:
    std::string _storeDir;
    std::map<uint64_t, ptrBuf> _sockMap;
    std::map<uint64_t,MPIFunctor> _handlers;
    uint64_t _npacket;
  };
};
#endif
