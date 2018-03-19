#include "TmvAccessSql.h"
#include <mysql/mysql.h>
#include <string>
#include <sstream>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef MAINP
int main()
{
  TmvAccessSql a;
  std::stringstream sac;
  sac<<getenv("MYTMV");
  a.connect(sac.str());
	    //"root/@localhost:mirabito");
  a.initRunTable(334457);
  sleep((unsigned int) 5);
  a.endRunTable();
}
#endif

void TmvAccessSql::openSqlite(std::string fileName)
{
  int32_t rc = sqlite3_open(fileName.c_str(), &theDb_);
  if( rc ){
    fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(theDb_));
    sqlite3_close(theDb_);
    exit(1);
  }
  useMysql_=false;
  useSqlite_=true;

}
int32_t TmvAccessSql::initRunTable(int32_t run)
{
  std::string start="(?,";
  if (useMysql_) start="(NULL,";
  //    char cmd[2048];
  //sprintf(cmd,"INSERT INTO EVENT ('RUN','EVENT','BCID','TIME') VALUES('%d','%d','%ld','%d')",run,event,bcid,time);
  std::stringstream s;
  s<<"insert into run set detector = (select identifier from detector order by datetime desc limit 0,1), type=0, run="<<run<<";";
  std::cout<<s.str()<<std::endl;
  
  int32_t retval =executeQuery(s.str());
  //printf("return value from executeQuery %d\n",retval);
  if (retval==0)
    {
      theRun_=run;
      theRunRowId_=getLastInsertId();
      theRunStartTime_=time(0);
    }
  return retval;
}
int32_t TmvAccessSql::endRunTable()
{
  std::string start="(?,";
  if (useMysql_) start="(NULL,";
  //    char cmd[2048];
  //sprintf(cmd,"INSERT INTO EVENT ('RUN','EVENT','BCID','TIME') VALUES('%d','%d','%ld','%d')",run,event,bcid,time);
  theRunEndTime_=time(0);
  int duration=int(theRunEndTime_-theRunStartTime_);
  std::stringstream s;
 
  s<<"update run set duration="<<duration<<" where run="<<theRun_<<";";
  std::cout<<s.str()<<std::endl;
  int32_t retval =executeQuery(s.str());
  if (retval!=0)
    {
      std::cout <<"insert  failed"<<std::endl;
    }
  return retval;
}

int32_t TmvAccessSql::executeQuery(std::string stmt)
{
  int32_t retval =-1;
  if (useSqlite_)
    {
      do 
	{
	  retval=sqlite3_exec(theDb_,stmt.c_str(),0,0,0);
	  if (retval!=SQLITE_OK)
	    {std::cout<<"SQLITE ERROR =>"<<stmt<<std::endl;getchar();}
	} while (retval!=SQLITE_OK);
    }
  if (useMysql_)
    {
      do {
	retval=mysql_query (&theMysql_,stmt.c_str());
	if (retval!=0)
	  {
	    fprintf(stderr, "Error during query: Command %s Error: %s\n",
		    stmt.c_str(),mysql_error(&theMysql_));
	    char c;c=getchar();putchar(c); if (c=='.') exit(0);;
	  } 
      }
      while (retval!=0);
      // get Last Id
      //ID_=mysql_insert_id(&mysql_);



    }
	
  return retval;
}

uint32_t TmvAccessSql::getLastInsertId()
{
  uint32_t id=0;
  if (useMysql_)
    id=mysql_insert_id(&theMysql_);
  if (useSqlite_)
    id=sqlite3_last_insert_rowid(theDb_);
  return id;
}

void TmvAccessSql::decodeAccount(std::string account)
{
  // "root/monpasswd@localhost:DHCAL_TEST"
  int ipass = account.find("/");
  int ipath = account.find("@");
  int idb = account.find(":");
  myName_.clear();
  myName_=account.substr(0,ipass); 
  myPwd_.clear();
  myPwd_=account.substr(ipass+1,ipath-ipass-1); 
  myHost_.clear();
  myHost_=account.substr(ipath+1,idb-ipath-1); 
  myDatabase_.clear();
  myDatabase_=account.substr(idb+1,account.size()-idb); 
  std::cout<<myName_<<std::endl;
  std::cout<<myPwd_<<std::endl;
  std::cout<<myHost_<<std::endl;
  std::cout<<myDatabase_<<std::endl;

}
void TmvAccessSql::connect(std::string account)
{
  std::cout<<"Connecting to :"<<account<<std::endl;
  decodeAccount(account);
  mysql_init (&theMysql_);

  if (!mysql_real_connect(&theMysql_,myHost_.c_str(),myName_.c_str(),myPwd_.c_str(),myDatabase_.c_str(),0,NULL,0))
    {
      fprintf(stderr, "Failed to connect to database: Error: %s\n",
	      mysql_error(&theMysql_));
      exit(0);
    }
  useMysql_=true;
  useSqlite_=false;
}
