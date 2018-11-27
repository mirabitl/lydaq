#ifndef __LYDAQ_BUILDER_
#define __LYDAQ_BUILDER_
#include "zmMerger.hh"

#include "baseApplication.hh"

#include "zhelpers.hpp"
#include "ReadoutLogger.hh"
#include <boost/interprocess/sync/interprocess_mutex.hpp>
namespace lydaq{ 
  class dsBuilder  : public zdaq::baseApplication
  {
  public:
    dsBuilder(std::string name);
    void configure(zdaq::fsmmessage* m);
    void start(zdaq::fsmmessage* m);
    void stop(zdaq::fsmmessage* m);
    void halt(zdaq::fsmmessage* m);
    void status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void registerds(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_setheader(Mongoose::Request &request, Mongoose::JsonResponse &response);

  private:
    zdaq::fsmweb* _fsm;
    zdaq::zmMerger* _merger;
    bool _running,_readout;
    zmq::context_t* _context;

  };
};
#endif
