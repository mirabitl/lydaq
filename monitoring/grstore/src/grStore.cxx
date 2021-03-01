
#include "grStore.hh"
#include "graphite-client.h"
using namespace zdaq;
using namespace lydaq;
lydaq::grStore::grStore() {} 
void lydaq::grStore::loadParameters(Json::Value params)
{
  if (params.isMember("grstore"))
    _params=params["grstore"];
}

void lydaq::grStore::connect()
{

  graphite_init(_params["host"].asString().c_str(),_params["port"].asUInt());
}

void lydaq::grStore::store(std::string loc,std::string hw,uint32_t ti,Json::Value status)
{
  
  std::stringstream spath("");


  // BMP
  if (status["name"].asString().compare("BMP")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".T";
      graphite_send_plain(spath.str().c_str(),status["temperature"].asDouble(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".P";
      graphite_send_plain(spath.str().c_str(),status["pressure"].asDouble(),ti);
      return;
    }
  // HIH8000
  if (status["name"].asString().compare("HIH")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".H0";
      graphite_send_plain(spath.str().c_str(),status["humidity0"].asDouble(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".T0";
      graphite_send_plain(spath.str().c_str(),status["temperature0"].asDouble(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".H1";
      graphite_send_plain(spath.str().c_str(),status["humidity1"].asDouble(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".T1";
      graphite_send_plain(spath.str().c_str(),status["temperature1"].asDouble(),ti);
      
      return;
    }
  // Genesys
  if (status["name"].asString().compare("GENESYS")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".VSET";
      graphite_send_plain(spath.str().c_str(),status["vset"].asDouble(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".VOUT";
      graphite_send_plain(spath.str().c_str(),status["vout"].asDouble(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".IOUT";
      graphite_send_plain(spath.str().c_str(),status["iout"].asDouble(),ti);


      float ist=0;
      if (status["status"].compare("ON")==0) ist=1;
	
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".ON";
      graphite_send_plain(spath.str().c_str(),1,ti);
      return;
    }
  // Genesys
  if (status["name"].asString().compare("ZUP")==0)
    {
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".VSET";
      graphite_send_plain(spath.str().c_str(),status["vset"].asDouble(),ti);
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".VOUT";
      graphite_send_plain(spath.str().c_str(),status["vout"].asDouble(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".ISET";
      graphite_send_plain(spath.str().c_str(),status["iset"].asDouble(),ti);

      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".PWR";
      graphite_send_plain(spath.str().c_str(),status["pwrstatus"].asUInt(),ti);
      
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".IOUT";
      graphite_send_plain(spath.str().c_str(),status["iout"].asDouble(),ti);


      float ist=0;
      if (status["status"].compare("ON")==0) ist=1;
	
      spath.str(std::string());
      spath.clear();
      spath<<loc<<"."<<hw<<".ON";
      graphite_send_plain(spath.str().c_str(),1,ti);
      return;
    }

  // WIENER
  if (status["name"].asString().compare("ISEG")==0)
    {
      const Json::Value& jchannels = status["channels"];
      for (Json::ValueConstIterator it = jchannels.begin(); it != jchannels.end(); ++it)
	{
	  const Json::Value& jsch = *it;

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].asUInt()<<".VSET";
	  graphite_send_plain(spath.str().c_str(),jsch["vset"].asDouble(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].asUInt()<<".VOUT";
	  graphite_send_plain(spath.str().c_str(),jsch["vout"].asDouble(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].asUInt()<<".ISET";
	  graphite_send_plain(spath.str().c_str(),jsch["vset"].asDouble(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].asUInt()<<".IOUT";
	  //std::cout<<"IOUT"<<jsch["iout"].asDouble()<<std::endl;
	  graphite_send_plain(spath.str().c_str(),jsch["iout"].asDouble()*1E6,ti);

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["id"].asUInt()<<".RAMP";
	  graphite_send_plain(spath.str().c_str(),jsch["rampup"].asDouble(),ti);

	 }
      return;
    }

  // SYX27
  if (status["name"].asString().compare("SYX27")==0)
    {
      const Json::Value& jchannels = status["channels"];
      for (Json::ValueConstIterator it = jchannels.begin(); it != jchannels.end(); ++it)
	{
	  const Json::Value& jsch = *it;

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].asUInt()<<"."<<jsch["channel"].asUInt()<<".VSET";
	  graphite_send_plain(spath.str().c_str(),jsch["vset"].asDouble(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].asUInt()<<"."<<jsch["channel"].asUInt()<<".VOUT";
	  graphite_send_plain(spath.str().c_str(),jsch["vout"].asDouble(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].asUInt()<<"."<<jsch["channel"].asUInt()<<".ISET";
	  graphite_send_plain(spath.str().c_str(),jsch["vset"].asDouble(),ti);
	  
	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].asUInt()<<"."<<jsch["channel"].asUInt()<<".IOUT";
	  graphite_send_plain(spath.str().c_str(),jsch["iout"].asDouble(),ti);

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].asUInt()<<"."<<jsch["channel"].asUInt()<<".RAMP";
	  graphite_send_plain(spath.str().c_str(),jsch["rampup"].asDouble(),ti);

	  spath.str(std::string());
	  spath.clear();
	  spath<<loc<<"."<<hw<<"."<<jsch["slot"].asUInt()<<"."<<jsch["channel"].asUInt()<<".STATUS";
	  graphite_send_plain(spath.str().c_str(),jsch["status"].asDouble(),ti);

	 }
      return;
    }
}
extern "C" 
{
    // loadDHCALAnalyzer function creates new LowPassDHCALAnalyzer object and returns it.  
  zdaq::zmonStore* loadStore(void)
    {
      return (new lydaq::grStore);
    }
    // The deleteDHCALAnalyzer function deletes the LowPassDHCALAnalyzer that is passed 
    // to it.  This isn't a very safe function, since there's no 
    // way to ensure that the object provided is indeed a LowPassDHCALAnalyzer.
  void deleteStore(zdaq::zmonStore* obj)
    {
      delete obj;
    }
}

