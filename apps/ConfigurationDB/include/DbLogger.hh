#ifndef _Db_Logger_
#define _Db_Logger_
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;
static LoggerPtr _logDb(Logger::getLogger("DbManager"));
static LoggerPtr _logOracle(Logger::getLogger("DbOracle"));
#endif
