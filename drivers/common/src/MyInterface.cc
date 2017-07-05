#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "MyInterface.h"
MyInterface::MyInterface(std::string account) : theAccount_(account)
{
  MyInterface::decodeAccount(account,theName_,thePwd_,theHost_,theDatabase_);
  theMysqlRes_=NULL;
  connected_=false;
}
void MyInterface::decodeAccount(std::string account,std::string &Name,std::string &Pwd,std::string &Host,std::string &Database)
{
  // "root/monpasswd@localhost:DHCAL_TEST"
  int ipass = account.find("/");
  int ipath = account.find("@");
  int idb = account.find(":");
  Name.clear();
  Name=account.substr(0,ipass); 
  Pwd.clear();
  Pwd=account.substr(ipass+1,ipath-ipass-1); 
  Host.clear();
  Host=account.substr(ipath+1,idb-ipath-1); 
  Database.clear();
  Database=account.substr(idb+1,account.size()-idb); 
  std::cout<<Name<<std::endl;
  std::cout<<Pwd<<std::endl;
  std::cout<<Host<<std::endl;
  std::cout<<Database<<std::endl;
  
}
void MyInterface::connect()
{
   mysql_init (&theMysql_);

  if (!mysql_real_connect(&theMysql_,theHost_.c_str(),theName_.c_str(),thePwd_.c_str(),theDatabase_.c_str(),0,NULL,0))
    {
      LOG4CXX_FATAL(_logMy,"Failed to connect to database:"<<mysql_error(&theMysql_));
      fprintf(stderr, "Failed to connect to database: Error: %s\n",
	      mysql_error(&theMysql_));
      return;
    }
  connected_=true;
}

void MyInterface::disconnect()
{
  if (connected_)
    {
      mysql_close(&theMysql_);
      connected_=false;
    }
}
int32_t MyInterface::executeQuery(std::string stmt)
{
  int32_t retval =-1;
  if (!connected_) return retval;
  
      do {
	retval=mysql_query (&theMysql_,stmt.c_str());
	if (retval!=0)
	  {
	    LOG4CXX_FATAL(_logMy,"Error during query: Command "<<stmt<<" Error: "<<mysql_error(&theMysql_));
	    fprintf(stderr, "Error during query: Command %s Error: %s\n",
		    stmt.c_str(),mysql_error(&theMysql_));
	    char c;c=getchar();putchar(c); if (c=='.') exit(0);;
	  } 
      }
      while (retval!=0);
      // get Last Id
      //ID_=mysql_insert_id(&mysql_);
	
  return retval;
}

int32_t MyInterface::executeSelect(std::string stmt)
{
  // Retrieve mainframe information (address, username, meteo, ...)
  int32_t retval=-1;
  if (!connected_) return retval;
  if (theMysqlRes_!=NULL)
    mysql_free_result(theMysqlRes_);
  if (retval=mysql_query(&theMysql_,stmt.c_str())) {
    LOG4CXX_FATAL(_logMy,"Error during select: Command "<<stmt<<" Error: "<<mysql_error(&theMysql_));

        fprintf(stderr, "%s\n", mysql_error(&theMysql_));
        exit(1);
    }

    theMysqlRes_ = mysql_use_result(&theMysql_);

    return retval;
}

MYSQL_ROW MyInterface::getNextRow()
{
  return mysql_fetch_row(theMysqlRes_);
}
