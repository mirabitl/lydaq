#include "OracleDIFDBInterface.hh"
#include <map>
#include <iostream>
#include <sstream>
using namespace std;
int main()
{
  std::string theOracleSetupName_="Dome_42chambres_Reference_v4_164";
  State *s;
  #undef ORACLE_USED
  #ifdef ORACLE_USED
  Setup* theOracleSetup_;

  std::string theXMLFile_="NONE";
  
  std::cout<<"On initialise Oracle "<<std::endl;
  try {
    DBInit::init();
  }
  catch(...)
    {
      LOG4CXX_FATAL(_logOracle,"Cannot initialise Oracle");
      return 0;
    }
 if (theXMLFile_.compare("NONE")==0)
   {
     try {
       theOracleSetup_=Setup::getSetup(theOracleSetupName_); 
       std::cout<<"On initialise Oracle "<<(long)theOracleSetup_<< std::endl;
       LOG4CXX_INFO(_logOracle,"On initialize Oracle "<<theOracleSetupName_);
       
     }
     catch(...)
       {
         LOG4CXX_FATAL(_logOracle,"Setup initialisation failed");
       }
   }
 else
   {
     theOracleSetup_=new Setup(theXMLFile_);
     s= State::createStateFromXML(theXMLFile_,theXMLFile_);
     theOracleSetup_->addState(s);
   }

 s=theOracleSetup_->getStates()[0];
 s->saveToXML(theOracleSetupName_);
 #else
 #undef USE_XML
 #ifdef USE_XML
 try {
 s = State::createStateFromXML_NODB(theOracleSetupName_,"Dome_42chambres_Reference_v4_164.xml");
 }
 catch(...)
   {}
 #else
     //Get the list of existing state names
 vector<string> state_names = State::getStateNames_WebServer();
 for (int i=0;i<state_names.size();i++)
   std::cout<<state_names[i]<<std::endl;
    // Download a State by its name
 try {
     s = State::getState_WebServer(theOracleSetupName_);
 } catch (ILCException::Exception e)
            {
              LOG4CXX_ERROR(_logOracle," Error in ASIC access"<<e.getMessage());
              std::cout<<e.getMessage()<<std::endl;
	    }
 #endif
#endif
    std::cout<<"Download Done "<<endl;
    //   exit(0);
   

 OracleDIFDBInterface oi(s);
 oi.download();
 oi.dumpToTree("/dev/shm/DB",theOracleSetupName_);

}

