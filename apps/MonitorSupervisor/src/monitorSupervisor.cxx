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
  this->fsm()->addState("PAUSED");
  this->fsm()->addState("RUNNING");
  
  this->fsm()->addTransition("INITIALISE","CREATED","PAUSED",boost::bind(&lydaq::monitorSupervisor::initialise, this,_1));
  this->fsm()->addTransition("START","PAUSED","RUNNING",boost::bind(&lydaq::monitorSupervisor::start, this,_1));
  this->fsm()->addTransition("STOP","RUNNING","PAUSED",boost::bind(&lydaq::monitorSupervisor::start, this,_1));
  this->fsm()->addTransition("DESTROY","PAUSED","CREATED",boost::bind(&lydaq::monitorSupervisor::destroy, this,_1));

  
  this->fsm()->addCommand("STATUS",boost::bind(&lydaq::monitorSupervisor::c_status,this,_1,_2));
  this->fsm()->addCommand("ITEM",boost::bind(&lydaq::monitorSupervisor::c_item,this,_1,_2));

  

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
  _subscribers.clear();
  delete _context;
  _context=0;
  if (_collitems)
    {mongoc_collection_destroy (_collitems);_collitems=0;}
  if (_database)
    {mongoc_database_destroy (_database);_database=0;}
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
int32_t lydaq:::monitorSupervisor::connectDb(std::string dbaccount)
{
  bson_t *command, reply;
  bson_error_t error;
  char *str;
  bool retval;
  // Find loggin


  std::string login;
  char* wp=getenv("MGDBLOGIN");
  if (wp!=NULL)  
    login=std::string(wp);
  else
    {
      LOG4CXX_ERROR(_logLdaq," MGDBLOGIN is not set");
      return EXIT_FAILURE;
    }
  std::size_t current, previous = 0;
  current = str.find("@");
  std::vector<string> cont;
  while (current != std::string::npos) {
    cont.push_back(str.substr(previous, current - previous));
    previous = current + 1;
    current = str.find("@", previous);
  }

  cont.push_back(str.substr(previous, current - previous));
  std::string userinfo=cont[0];
  std::stringstream hostinfo("");
  hostinfo<<"mongodb://"<<cont[1];
  std::string uri_string=hostinfo.str();
  _dbname=cont[2];
  /*
    userinfo=login.split("@")[0]
    hostinfo=login.split("@")[1]
    dbname=login.split("@")[2]
    user=userinfo.split("/")[0]
    pwd=userinfo.split("/")[1]
    host=hostinfo.split(":")[0]
    port=int(hostinfo.split(":")[1])

  */
  // mongoc internal
  mongoc_init ();

  //find uri and dbname

  
  _uri = mongoc_uri_new_with_error (uri_string, &error);
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
  _client = mongoc_client_new_from_uri (_uri);
  if (!_client) {
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
  _collitems = mongoc_client_get_collection (_client,_dbname,"MONITORED_ITEMS");

  /*
   * Do work. This example pings the database, prints the result as JSON and
   * performs an insert
   */
  command = BCON_NEW ("ping", BCON_INT32 (1));

  int32_t retval = mongoc_client_command_simple (
						 _client, "admin", command, NULL, &reply, &error);

  if (!retval) {
    fprintf (stderr, "%s\n", error.message);
    LOG4CXX_ERROR(_logLdaq," Cannot ping to dbaccount :"<<error.message);
    return EXIT_FAILURE;
  }

  str = bson_as_json (&reply, NULL);
  LOG4CXX_INFO(_logLdaq," Ping to "<<dbaccount<<" => "<<std::string(str));
  bson_destroy (&reply);
  bson_destroy (command);
  bson_free (str);
  return retval;

}
void lydaq::monitorSupervisor::start(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());
  _subscribers.clear();
  if (!_collitems)
    _collitems = mongoc_client_get_collection (_client,_dbname,"MONITORED_ITEMS");
  // Find running items
  Json::Value jitems=this->listItems();
  
  for (...)
    {
      Json::Value jitem;
      if (jitem["state"].asString().compare("PUBLISHING")!=0)
	continue;
      std::stringstream sit;
      sit<<"tcp://"<<jitem["host"]<<":"<<jitem["pubport"];
      _subscribers.addStream(sit.str());
    }
  _subscribers->addHandler(boost::bind(&lydaq::monitorSupervisor::processItems, this,_1));
  _subscribers->start();
}
void lydaq::monitorSupervisor::stop(zdaq::fsmmessage* m)
{
  _subscribers->stop();
}
void lydaq::monitorSupervisor::processItems(std::vector<zdaq::mon::publishedItem*>& vitems)
{
  if (!_measitems)
      _measitems = mongoc_client_get_collection (_client,_dbname,"MEASURED_ITEMS");

  for (auto x:vitems)
    {
      std::cout<<x->location()<<" "<<x->hardware()<<" "<<x->status()<<std::endl;
    }
}
void lydaq::monitorSupervisor::initialise(zdaq::fsmmessage* m)
{
  LOG4CXX_INFO(_logLdaq," CMD: "<<m->command());

  
  if (m->content().isMember("dbaccount"))
    { 
      this->parameters()["dbaccount"]=m->content()["dbaccount"];
    }
  if (this->parameters().isMember("dbaccount"))
    {
      _dbaccount=this->parameters()["dbaccount"].asString();
      int32_t ier=this->connectDb(_dbaccount);
	
    }
  else
    LOG4CXX_ERROR(_logLdaq," dbaccount not set. Nothing initialised ");

  
}
void lydaq::monitorSupervisor::c_status(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  //  LOG4CXX_INFO(_logLdaq," Trig ext setting called ");

  response["items"]=this->listItems();
} 
void lydaq::monitorSupervisor::c_item(Mongoose::Request &request, Mongoose::JsonResponse &response)
{
  LOG4CXX_INFO(_logLdaq," Registering item status ");

  std::string location=request.get("location","");
  std::string host=request.get("host","");
  std::string hardware=request.get("hardware","");
  uint32_t webport=atol(request.get("webport","0").c_str());
  uint32_t pubport=atol(request.get("pubport","0").c_str());
  std::string state=request.get("state","");
  if (location.length()==0 || hardware.length()==0 || host.length()==0 || state.length()==0 || webport==0|| pubport==0)
    {
      LOG4CXX_ERROR(_logLdaq," Missing item infos ");
      response["STATUS"]="FAILED";
      return;
    }

  // Now query the item in the DB and find the last one
    if (!_collitems)
      _collitems = mongoc_client_get_collection (_client,_dbname,"MONITORED_ITEMS");
  std::stringstream sal("");
  sal<<"{\"location\":\""<<location<<"\""
     <<",\"host\":\""<<host<<"\""
     <<",\"hardware\":\""<<hardware<<"\""
     <<",\"webport\":"<<webport
     <<",\"pubport\":"<<pubport<<"}";
  bson_t* queryitems=bson_new_from_json ((const uint8_t *)sal.str().c_str(), -1, &error);
  mongoc_cursor_t *citem = mongoc_collection_find_with_opts (_collitems, queryitems, NULL, NULL);
  const bson_t *docitems;
  Json::Value vcitem,vclast;
  time_t tlast=0;
  while (mongoc_cursor_next (citem, &docitems))
    {
      char* stritem = bson_as_relaxed_extended_json (docitem, NULL);
      printf ("\t ===>\n");
      Json::Reader readera;
      bool parsinga =reader.parse(stritem,vcitem);
      std::cout<<vcitem["time"].asUInt()<<":"<<vcitem["host"]<<":"<<vcitem["state"]<<std::endl;
      if (vcitem["time"].asUInt()>tlast)
	{tlast=vcitem["time"].asUInt();vclast=vcitem;}
      bson_free(stritem);
    }
  bson_destroy(queryitems);
  mongoc_cursor_destroy(citem);
  if (tlast!=0)
    {
      if (vclast["state"].asString().compare(state)==0)
	{
	  LOG4CXX_ERROR(_logLdaq," Already registered "<<vcitem);
	  response["STATUS"]="ALREADYDONE";
	  return;
	}
      // Update
      std::stringstream sal("");
      sal<<"{\"_id\":{\"$in\":"<<vclast["_id"]<<"}}";
      bson_t* querya=bson_new_from_json ((const uint8_t *)sal.str().c_str(), -1, &error);
      bson_t *update = BCON_NEW ("$set",
				 "{",
				 "state",
				 BCON_UTF8 (state.c_str()),
				 "time",
				 BCON_INT32 (time(0)),
				 "}");

      if (!mongoc_collection_update_one (
					 _collitems, query, update, NULL, NULL, &error)) {
	fprintf (stderr, "%s\n", error.message);
	if (querya)
	  bson_destroy (querya);
	if (update)
	  bson_destroy (update);
	LOG4CXX_ERROR(_logLdaq," Update failed "<<error.message);
	response["STATUS"]="UPDATEFAILED";
	return;

      }
      if (querya)
	bson_destroy (querya);
      if (update)
	bson_destroy (update);

      response["STATUS"]="DONE";
      return;
    }
  else
    {
      // Create the item
      vcitem["time"]=time(0);
      vcitem["state"]=state;
      vcitem["location"]=location;
      vcitem["hardware"]=hardware;
      vcitem["host"]=host;
      vcitem["webport"]=webport;
      vcitem["pubport"]=pubport;
      Json::FastWriter fastWriter;
      std::string scont= fastWriter.write(vcitem);
      
      bson_t      *doc;
      char        *string;
      doc = bson_new_from_json ((const uint8_t *)scont.c_str(), -1, &error);
      if (!doc) {
	LOG4CXX_ERROR(_logLdaq,"Cannot create doc "<<error.message);
	response["STATUS"]="CANNOTPARSE";
	return;
      }
      if (!mongoc_collection_insert_one (_collitems, doc, NULL, NULL, &error)) {
	LOG4CXX_ERROR(_logLdaq,"Cannot insert doc "<<error.message);
	response["STATUS"]="CANNOTINSERT";
	return;
      }
      bson_destroy (doc);

      response["VALUE"]=vcitem;
      response["STATUS"]="DONE";
      return;
    }
  response["STATUS"]=this->listItems();

 } 
Json::Value lydaq::monitorSupervisor::listItems()
{

  bson_t* queryitems=bson_new_from_json ("{}", -1, &error);
  mongoc_cursor_t *citem = mongoc_collection_find_with_opts (_collitems, queryitems, NULL, NULL);
  const bson_t *docitems;
  Json::Value array_keys;
  Json::Reader readera;
  while (mongoc_cursor_next (citem, &docitems))
    {
      char* stritem = bson_as_relaxed_extended_json (docitem, NULL);
      Json::Value vcitem;
      bool parsinga =reader.parse(stritem,vcitem);
      if (parsinga)
	array_keys.append(vcitem);
      bson_free(stritem);
    }
  bson_destroy(queryitems);
  mongoc_cursor_destroy(citem);
  
  return array_keys;
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
