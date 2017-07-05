#ifndef HVCAENINTERFACE_H
#define HVCAENINTERFACE_H
#include <string>


namespace lydaq
{
  class HVCaenInterface
  {
  public:
    HVCaenInterface(std::string host,std::string user,std::string pwd);
    virtual ~HVCaenInterface();
    virtual void Connect();
    virtual void Disconnect();
    virtual void SetOn(uint32_t channel);
    virtual void SetOff(uint32_t channel);
    virtual void SetCurrent(uint32_t channel,float imax);
    virtual void SetVoltage(uint32_t channel,float v0);
    virtual void SetVoltageRampUp(uint32_t channel,float v0);
    virtual float GetCurrentSet(uint32_t channel);
    virtual float GetVoltageSet(uint32_t channel);
    virtual float GetCurrentRead(uint32_t channel);
    virtual float GetVoltageRead(uint32_t channel);
    virtual float GetVoltageRampUp(uint32_t channel);
    virtual std::string GetName(uint32_t channel);
    virtual uint32_t GetStatus(uint32_t channel);
    bool isConnected(){ return connected_;}
  private:
    std::string theHost_,theUser_,thePassword_,theIp_;
    int32_t theID_;
    int32_t theHandle_;
    bool connected_;
  };
};
#endif
