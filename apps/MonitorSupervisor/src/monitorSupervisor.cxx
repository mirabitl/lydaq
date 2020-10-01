#include "monitorSupervisor.hh"
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "zhelpers.hpp"
#include <boost/algorithm/string.hpp>
using namespace zdaq;
using namespace zdaq::mon;

lydaq::monitorSupervisor::monitorSupervisor(std::string name) : zdaq::baseApplication(name),_dbaccount(""),_context(0),_uri(0),_client(0),_database(0),_collitems(0)
{
  //_fsm=this->fsm();
  this->fsm()->addState("RUNNING");
  
  this->fsm()->addTransition("INITIALISE","CREATED","RUNNING",boost::bind(&lydaq::monitorSupervisor::initialise, this,_1));
  this->fsm()->addTransition("DESTROY","PAUSED","CREATED",boost::bind(&lydaq::monitorSupervisor::destroy, this,_1));

  
 this->fsm()->addCommand("STATUS",boost::bind(&lydaq::monitorSupervisor::c_status,this,_1,_2));
 this->fsm()->addCommand("ITEM",boost::bind(&lydaq::monitorSupervisor::c_item,this,_1,_2));
 this->fsm()->addCommand("MONITOR",boost::bind(&lydaq::monitorSupervisor::c_monitor,this,_1,_2)); 
  

  char* wp=getenv("WEBPORT");
  if (wp!=NULL)
    {
      std::cout<<"Service "<<name<<" started on port "<<atoi(wp)<<std::endl;
    this->fsm()->start(atoi(wp));
    }
}
void lydaq::monitorSupervisor::clear()
{
  if (_context==0) return;
  for (auto x:_subscribers)
    delete x;
  _subscribers.clear();
  delete _context;
  _context=0;
  if (_collitems)
    mongoc_collection_destroy (_collitems);
  if (_database)
    mongoc_database_destroy (_database);
  if (_uri)
    mongoc_uri_destroy (_uri);
  if (_client)
    mongoc_client_destroy (_client);
  _collitems=0;
  _database=0;
  _uri=0;
  _client=0;
  mongoc_cleanup ();
  
}
void lydaq:::monitorSupervisor::connectDb(std::string dbaccount)
{
  bson_t *command, reply;
  bson_error_t error;
  char *str;
  bool retval;

  // mongoc internal
  mongoc_init ();

  //find uri and dbname

  
    uri = mongoc_uri_new_with_error (uri_string, &error);
  if (!_uri) {
    fprintf (stderr,
	     "failed to parse URI: %s\n"
	     "error message:       %s\n",
	     uri_string,
	     error.message);
    return EXIT_FAILURE;
  }

  /*
   * Create a new client instance
   */
  client = mongoc_client_new_from_uri (_uri);
  if (!client) {
    return EXIT_FAILURE;
  }

  /*
   * Register the application name so we can track it in the profile logs
   * on the server. This can also be done from the URI (see other examples).
   */
  mongoc_client_set_appname (_client, "connect-slowcontrol");

  /*
   * Get a handle on the database "db_name" and collection "coll_name"
   */
  _database = mongoc_client_get_database (_client,_dbname);
  _collitems = mongoc_client_get_collection (_client,_dbname,"ITEMS");

  /*
   * Do work. This example pings the database, prints the result as JSON and
   * performs an insert
   */
  command = BCON_NEW ("ping", BCON_INT32 (1));

  int retval = mongoc_client_command_simple (
					 _client, "admin", command, NULL, &reply, &error);

  if (!retval) {
    fprintf (stderr, "%s\n", error.message);
    return EXIT_FAILURE;
  }

  str = bson_as_json (&reply, NULL);
  printf ("%s\n", str);
  /*
    insert = BCON_NEW ("hello", BCON_UTF8 ("switzerland"));

    if (!mongoc_collection_insert_one (collection, insert, NULL, NULL, &error)) {
    fprintf (stderr, "%s\n", error.message);
    }

    bson_destroy (insert);
  */
  bson_destroy (&reply);
  bson_destroy (command);
  bson_free (str);

}
void lydaq::monitorSupervisor::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  
  if (m->content().isMember("dbaccount"))
    { 
      _dbaccount=m->content()["dbaccount"].asString();
      this->parameters()["dbaccount"]=m->content()["dbaccount"];
    }
  else
    _dbaccount=this->parameters()["dbaccount"].asString();

  this->connectDb(_dbaccount);

  if (m->content().isMember("streams"))
    { 
      this->parameters()["streams"]=m->content()["streams"];
    }
 
  if (this->parameters().isMember("streams"))
    {
      this->clear();
      _context=new zmq::context_t(1);
      
      const Json::Value& streams = this->parameters()["streams"];
      Json::Value array_keys;
      for (Json::ValueConstIterator it = streams.begin(); it != streams.end(); ++it)
	{
	  const Json::Value& book = *it;
	  std::cout<<"Registering "<<(*it).asString()<<std::endl;

	  lydaq::monitorItem* item=new  lydaq::monitorItem((*it).asString(),(*_context));
	  _items.push_back(item);
	  array_keys.append((*it).asString());

	}
    }
  
}
void lydaq::monitorSupervisor::openSqlite(std::string dbname)
{
 char *zErrMsg = 0;
 int rc;
   std::string sql;
   _dbname=dbname;
   /* Open database */
   rc = sqlite3_open(_dbname.c_str(), &_db);
   
   if( rc ) {
      fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(_db));
      return;
   } else {
      fprintf(stdout, "Opened database successfully\n");
   }

   /* Create SQL statement */
   sql = "CREATE TABLE RESULTS (ID INTEGER PRIMARY KEY     AUTOINCREMENT,"\
     "HARDWARE           TEXT    NOT NULL,"\
     "LOCATION           TEXT NOT NULL,"\
     "CTIME              INT     NOT NULL,"\
     "STATUS             TEXT NOT NULL );";

   /* Execute SQL statement */
   rc = sqlite3_exec(_db, sql.c_str(), sqliteCallback, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
   fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Table created successfully\n");
   }
   sqlite3_close(_db);
}
void lydaq::monitorSupervisor::start(zdaq::fsmmessage* m)
{
   LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  _running=true;
  g_d.create_thread(boost::bind(&lydaq::monitorSupervisor::poll,this));
}

void lydaq::monitorSupervisor::stop(zdaq::fsmmessage* m)
{
  _running=false;
  g_d.join_all();
}
void lydaq::monitorSupervisor::destroy(zdaq::fsmmessage* m)
{
  _running=false;
  this->clear();
}
void lydaq::monitorSupervisor::poll()
{
  char *zErrMsg = 0;
  int rc;
  //Initialise pollitems
  _nItems= _items.size();
 
  for (int i=0;i<_nItems;i++)
    {
      //memset(&_pollitems[i],0,sizeof(zmq::pollitem_t));
      _pollitems[i]=_items[i]->item();
      /*
      _pollitems[i].socket=_items[i]->socket();
      _pollitems[i].fd=0;
      _pollitems[i].events=ZMQ_POLLIN;
      _pollitems[i].revents=0;
      */
    }
  // Loop
  std::vector<std::string> strs;
  LOG4CXX_INFO(_logLdaq," Polling started: "<<_nItems);
    while (_running)
    {
      LOG4CXX_DEBUG(_logLdaq," Polling loop: "<<_nItems);

      rc=zmq::poll (&_pollitems [0], _nItems, 3000);

      LOG4CXX_DEBUG(_logLdaq," Polling results: "<<rc);
      if (rc==0) continue;
      for (uint16_t i=0;i<_nItems;i++)
        if (_pollitems[i].revents & ZMQ_POLLIN) {


	  std::string address = s_recv (_items[i]->socket());

	  // split address hardware@location@time
	  strs.clear();
	  boost::split(strs,address, boost::is_any_of("@"));
	  //  Read message contents
	  zmq::message_t message;
	  _items[i]->socket().recv(&message);
	  //std::cout<<"Message size is "<<message.size()<<std::endl;
	  //char buffer[65536];


	  std::string contents ;
	  contents.clear();
	  contents.assign((char*) message.data(),message.size());

	  _items[i]->processData(address,contents);
	  
	  time_t tr;
	  sscanf(strs[2].c_str(),"%lld",(long long *) &tr);
	  //printf("Message receive :\n \t Hardware: %s \n \t Location %s \n \t Time: %lld \n %s \n",strs[0].c_str(),strs[1].c_str(),(long long) tr,contents.c_str());
	  // Fill DB
	  if (_dbname.length()>0)
	    {
	      rc = sqlite3_open(_dbname.c_str(), &_db);
		  
	      if( rc ) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(_db));
		return;
	      }
	      // else {
	      // 	fprintf(stdout, "Opened database successfully\n");
	      // }

	      std::stringstream sql;   
	      /* Create SQL statement */
	      sql <<"INSERT INTO RESULTS (HARDWARE,LOCATION,CTIME,STATUS) " \
		"VALUES ('"<<strs[0]<<"','"<<strs[1]<<"',"<<tr<<",'"<<contents<<"');"; 
	      /* Execute SQL statement */
	      rc = sqlite3_exec(_db, sql.str().c_str(), sqliteCallback, 0, &zErrMsg);
	      if( rc != SQLITE_OK ){
		fprintf(stderr, "SQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	      }
	      // else {
	      // 	fprintf(stdout, "Records created successfully\n");
	      // }

	      sqlite3_close(_db);
	    }

	}

    }
    
}

void lydaq::monitorSupervisor::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  //  LOG4CXX_INFO(_logLdaq," Trig ext setting called ");

  Json::Value array_keys;

  for (auto x:_items)
    {
       Json::Value rc;
      
       rc["VALUE"]=x->status();
       rc["HARDWARE"]=x->hardware();
       rc["LOCATION"]=x->location();
       rc["TIME"]=(uint32_t) x->time();
       array_keys.append(rc);

    }
  response["STATUS"]=array_keys;
} 

#ifdef OLDEXAMPLE
#include "zhelpers.hpp"
Json::Value toJson(std::string s)
{
  Json::Reader reader;
  Json::Value jsta;
  bool parsingSuccessful = reader.parse(s,jsta);
  return jsta;
}
int main () {
    //  Prepare our context and subscriber
    zmq::context_t context(1);
    zmq::socket_t subscriber (context, ZMQ_SUB);
    subscriber.connect("tcp://lyosdhcal7:5600");
    subscriber.setsockopt( ZMQ_SUBSCRIBE, "", 0);
    zmq::socket_t subscriber1 (context, ZMQ_SUB);
    subscriber1.connect("tcp://lyoilcrpi13:5600");
    subscriber1.setsockopt( ZMQ_SUBSCRIBE, "", 0);
    zmq::socket_t subscriber2 (context, ZMQ_SUB);
    subscriber2.connect("tcp://lyoilcrpi24:5600");
    subscriber2.setsockopt( ZMQ_SUBSCRIBE, "", 0);
//  Initialize poll set
    zmq::pollitem_t items [] = {
        { subscriber, 0, ZMQ_POLLIN, 0 },
	{ subscriber1, 0, ZMQ_POLLIN, 0 },
	{ subscriber2, 0, ZMQ_POLLIN, 0 }
    };
    std::vector<std::string> strs;
    while (1) {
  

	  
	  //std::cout<<contents.length()<<" string len "<<std::endl;
	  //= s_recv (subscriber);
 	  std::stringstream meas;
	  meas << "|" <<time(0)<<"|"<< address;


	  
	  Json::Value jcaen=toJson(contents);
	  const Json::Value& jdevs= jcaen["channels"];
	  for (Json::ValueConstIterator jt = jdevs.begin(); jt != jdevs.end(); ++jt)
	    {
	           //TB_DIFStatus_->elementAt(irow, 0)->addWidget();
	      meas<<"|"<< (*jt)["id"].asUInt();
	      meas<<"|"<< (*jt)["status"].asUInt();
	      meas<<"|"<< (*jt)["vset"].asFloat();
	      meas<<"|"<< (*jt)["iset"].asFloat();
	      meas<<"|"<< (*jt)["vout"].asFloat();
	      meas<<"|"<< (*jt)["iout"].asFloat();
	      // std::string status=(*jt)["status"].asString();
	      // std::size_t found = status.find("=");
	      
	      // printf("%3d %5.1f %5.1e %5.1f %7.2f %5.1f %s \n",
	      // 	     (*jt)["id"].asUInt(),
	      // 	     (*jt)["vset"].asFloat(),
	      // 	     (*jt)["iset"].asFloat(),
	      // 	     (*jt)["vout"].asFloat(),
	      // 	     (*jt)["iout"].asFloat(),
	      // 	     (*jt)["rampup"].asFloat(),
	      // 	     status.substr(found+1,status.length()-found-2).c_str());

	    }
	  std::cout<<meas.str()<<"|"<<std::endl;
	  fflush(stdout);
            //  Process task
        }
        if (items [1].revents & ZMQ_POLLIN) {
	  std::string address = s_recv (subscriber1);
	  //  Read message contents
	  std::string contents = s_recv (subscriber1);
	  
	  //std::cout << "|" << time(0)<<"|"<<address << "|" << contents ;
	  std::stringstream meas;
	  meas << "|" <<time(0)<<"|"<< address;
	  Json::Value jhum=toJson(contents);
	  meas<<"|"<<jhum["humidity0"].asFloat();
	  meas<<"|"<<jhum["temperature0"].asFloat();
	  meas<<"|"<<jhum["humidity11"].asFloat();
	  meas<<"|"<<jhum["temperature1"].asFloat();
	  std::cout<<meas.str()<<"|"<<std::endl;
	  fflush(stdout);
            //  Process weather update
        }
        if (items [2].revents & ZMQ_POLLIN) {
	  std::string address = s_recv (subscriber2);
	  //  Read message contents
	  std::string contents = s_recv (subscriber2);
	  //std::cout << "|" << time(0)<<"|"<<address << "|" << contents;

	  std::stringstream meas;
	  meas << "|" <<time(0)<<"|"<< address;
	  Json::Value jbmp=toJson(contents);
	  meas<<"|"<<jbmp["pressure"].asFloat();
	  meas<<"|"<<jbmp["temperature"].asFloat()+273.15;
	  std::cout<<meas.str()<<"|"<<std::endl;

	  
	  /*std::cout << "BMP->[" << address << "] " << std::endl;
	  Json::Value jbmp=toJson(contents);
	  printf("%7.2f %7.2f \n",
		 jbmp["pressure"].asFloat(),jbmp["temperature"].asFloat()+273.15);
	  */
	  fflush(stdout);
            //  Process weather update
        }
        //  Read envelope with address
        
    }
    return 0;
}
#endif
