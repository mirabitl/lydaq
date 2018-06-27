#ifndef _DBCACHE_HH
#define _DBCACHE_HH
#include "OracleDIFDBInterface.hh"
#include "baseApplication.hh"
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;
static LoggerPtr _logDbCache(Logger::getLogger("configDbCache"));
#include <zmq.hpp>
#include <zhelpers.hpp>
namespace lydaq {
  class DbCache : public zdaq::baseApplication
  {
  public:
    DbCache(std::string name);
    void initialise(zdaq::fsmmessage* m);

    void configure(zdaq::fsmmessage* m);
    void destroy(zdaq::fsmmessage* m);
    // action
    void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
    void c_publish(Mongoose::Request &request, Mongoose::JsonResponse &response);

  private:
    void ls(std::string sourcedir,std::vector<std::string>& res);
  
    std::string _mode;
    std::string _stateName;
    std::string _path,_publish;
    State* _state;
    zmq::context_t* _context;
    zmq::socket_t *_publisher;

  };
};
#endif
