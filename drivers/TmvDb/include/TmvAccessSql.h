#ifndef _TMVACCESSSQL_
#define _TMVACCESSSQL_
#include <iostream>
#include <sys/timeb.h>
#include "sqlite3.h"
#include <mysql/mysql.h>
#include <string>
#include <sstream>
#include <stdint.h>
class TmvAccessSql
{
public:
  TmvAccessSql(){useMysql_=false;useSqlite_=false;}
  void openSqlite(std::string fileName);
  int32_t initRunTable(int32_t run);
  int32_t endRunTable();
  int32_t executeQuery(std::string stmt);
  uint32_t getLastInsertId();
  void decodeAccount(std::string account);
  void connect(std::string account);
private:
  bool useMysql_;
  bool useSqlite_;
  sqlite3* theDb_;
  MYSQL theMysql_;
  std::string myName_,myPwd_,myHost_,myDatabase_;

  uint32_t theRun_;
  uint32_t theRunRowId_;
  uint32_t theRunStartTime_,theRunEndTime_;
};

#endif
