#ifndef __MONITORSUPERVISOR_HH__
#define __MONITORSUPERVISOR_HH__
#include <zmq.hpp>
#include <iostream>
#include <sstream>
#include <stdio.h>
#include <dirent.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <json/json.h>
#include <time.h>
#include "baseApplication.hh"
#include "zSubscriber.hh"
#include <sstream>
#include <map>
#include <vector>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/bind.hpp>
#include <stdlib.h>
#include "ReadoutLogger.hh"
#include <mongoc.h>
#include <json/json.h>

namespace lydaq
{
 
    class monitorSupervisor : public zdaq::baseApplication
    {
    public:
      monitorSupervisor(std::string name);
      void clear();
      void connectDb(std::string dbaccount);
      void initialise(zdaq::fsmmessage* m);
      void destroy(zdaq::fsmmessage* m);
      void c_status(Mongoose::Request &request, Mongoose::JsonResponse &response);
      void c_item(Mongoose::Request &request, Mongoose::JsonResponse &response);
      void c_monitor(Mongoose::Request &request, Mongoose::JsonResponse &response);
      void processItems(std::vector<zdaq::mon::publishedItem*>& v);
    private:
      std::map<std::string,zdaq::mon::zSubscriber*> _subscribers;
      std::string _dbaccount,_dbname;
      zmq::context_t* _context;

      mongoc_uri_t *_uri;
      mongoc_client_t *_client;
      mongoc_database_t *_database;
      mongoc_collection_t *_collitems;
      //boost::interprocess::interprocess_mutex _sem;

    };
  



  };


#endif

  #ifdef ORIGINALEXAMPLE
//
//  Weather update client in C++
//  Connects SUB socket to tcp://localhost:5556
//  Collects weather updates and finds avg temp in zipcode
//
//  Olivier Chamoux <olivier.chamoux@fr.thalesgroup.com>
//
//#include "zmPuller.hh"
#include <regex>



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
    while (1) {
      zmq::poll (&items [0], 3, -1);
        
        if (items [0].revents & ZMQ_POLLIN) {


	  std::string address = s_recv (subscriber);
	  //  Read message contents
	  zmq::message_t message;
	  subscriber.recv(&message);
	  //std::cout<<"Message size is "<<message.size()<<std::endl;
	  //char buffer[65536];
	  

	  std::string contents ;
	  contents.assign((char*) message.data());
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
