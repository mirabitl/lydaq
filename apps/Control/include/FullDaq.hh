#ifndef _FullDaq_h
#define _FullDaq_h
#include "fsmwebCaller.hh"
#include "baseApplication.hh"
#include <string>
#include <vector>
#include <json/json.h>
#include "TmvAccessSql.hh"
namespace lydaq
{
class FullDaq : public zdaq::baseApplication
{
public:
  FullDaq(std::string name);
  ~FullDaq();
 
  void  discover(zdaq::fsmmessage* m);
  void  prepare(zdaq::fsmmessage* m);
  std::string  difstatus();
  void  singlescan(fsmwebCaller* d);
  void  singleinit(fsmwebCaller* d);
  void  singleregisterdb(fsmwebCaller* d);
  void  singleconfigure(fsmwebCaller* d);
  void  singlestart(fsmwebCaller* d);
  void  singlestop(fsmwebCaller* d);
  Json::Value  toJson(std::string s);
  void  initialise(zdaq::fsmmessage* m);
  void  configure(zdaq::fsmmessage* m);
  void  start(zdaq::fsmmessage* m);
  void  stop(zdaq::fsmmessage* m);
  void  destroy(zdaq::fsmmessage* m);

  void listProcess(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void forceState(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void doubleSwitchZup(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void  LVStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void LVON(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void LVOFF(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setParameters(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void getParameters(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void downloadDB(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setControlRegister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void dbStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void  builderStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void status(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void pauseTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void resumeTrigger(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void pauseEcal(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void resumeEcal(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void resetTriggerCounters(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerStatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerSpillOn(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerSpillOff(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerBeam(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setThreshold(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setGain(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void registerDataSource(Mongoose::Request &request, Mongoose::JsonResponse &response);

  //Calibration
  void triggerSpillRegister(Mongoose::Request &request, Mongoose::JsonResponse &response);
   void triggerHardReset(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerCalibCount(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerCalibOn(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerCalibRegister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerReloadCalib(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void tdcSet6bDac(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void tdcSetVthTime(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void tdcSetMode(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void tdcSetMask(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void setRunHeader(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void tdcstatus(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerSetRegister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void triggerGetRegister(Mongoose::Request &request, Mongoose::JsonResponse &response);
  void resetTdc(Mongoose::Request &request, Mongoose::JsonResponse &response);

  
  std::string state(){return _fsm->state();}
  void forceState(std::string s){_fsm->setState(s);}
  // Virtual from baseAPplication
  virtual void  userCreate(zdaq::fsmmessage* m);
private:
  zdaq::fsmweb* _fsm;
  fsmwebCaller* _dbClient,*_zupClient,*_cccClient,*_mdccClient,*_builderClient,*_gpioClient;
  std::vector<fsmwebCaller*> _DIFClients;
  std::vector<fsmwebCaller*> _tdcClients;
 
  std::string _dbstate;
  uint32_t _ctrlreg,_run;
  Json::Value _jConfigContent;
  // Add-ons for Tomuvol
  TmvAccessSql _tmv;
};
};
#endif
