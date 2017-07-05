#ifndef _Common_Logger_
#define _Common_Logger_
#include <log4cxx/logger.h>
#include <log4cxx/xml/domconfigurator.h>
using namespace log4cxx;
using namespace log4cxx::xml;
using namespace log4cxx::helpers;
using namespace std;
static LoggerPtr _logShm(Logger::getLogger("CommonShm"));
static LoggerPtr _logMy(Logger::getLogger("CommonMysql"));
#endif
