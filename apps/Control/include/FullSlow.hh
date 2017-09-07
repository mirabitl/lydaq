#ifndef _FullSlow_h
#define _FullSlow_h
#include "fsmwebCaller.hh"
#include "baseApplication.hh"
#include <string>
#include <vector>
#include <json/json.h>

namespace lydaq
{
class FullSlow : public zdaq::baseApplication
{
public:
  FullSlow(std::string name);
  ~FullSlow();
  // Virtual from baseAPplication
  virtual void  userCreate(zdaq::fsmmessage* m);
  void  destroy(zdaq::fsmmessage* m); 
  void  discover(zdaq::fsmmessage* m);
  void  configure(zdaq::fsmmessage* m);
  void  start(zdaq::fsmmessage* m);
  void  stop(zdaq::fsmmessage* m);


  void HVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void HVON(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void CLEARALARM(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void HVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setVoltage(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setCurrentLimit(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setRampUp(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void  LVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void LVON(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void LVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void  PTStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  std::string state(){return _fsm->state();}
  void forceState(std::string s){_fsm->setState(s);}
private:
  zdaq::fsmweb* _fsm;
  fsmwebCaller* _caenClient,*_zupClient,*_genesysClient,*_bmpClient,*_isegClient,*_gpioClient;
  Json::Value _jConfigContent;
};
};
#endif
