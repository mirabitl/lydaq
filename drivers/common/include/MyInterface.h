#ifndef MYINTERFACE_H
#define MYINTERFACE_H
#include <string>
#include <mysql/mysql.h>
#include "CommonLogger.h"
class MyInterface {
 public:
  MyInterface(std::string account);
  static void decodeAccount(std::string account,std::string &Name,std::string &Pwd,std::string &Host,std::string &Database);
  void connect();
  void disconnect();
  int32_t executeQuery(std::string stmt);
  int32_t executeSelect(std::string stmt);
  MYSQL_ROW getNextRow();
 private:
  std::string theAccount_,theName_,thePwd_,theHost_,theDatabase_;
  MYSQL theMysql_;
  MYSQL_RES* theMysqlRes_;
  bool connected_;
    };

#endif
