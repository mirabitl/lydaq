#ifndef _TDCCHANNEL_HH
#define _TDCCHANNEL_HH
#define TDC_COARSE_TIME 2.5
#define TDC_FINE_TIME 0.009765625
#include <stdint.h>
using namespace std;
namespace lydaq {
  class TdcChannel
{
public:
  TdcChannel() :_fr(NULL),_used(false) {;}
  TdcChannel(uint8_t*  b) :_fr(b),_used(false) {;}
  inline uint8_t channel() {return  (_fr[0]&0XFF);}
  inline uint8_t length(){return 8;}
  inline uint64_t coarse() const {return ((uint64_t)_fr[6])|((uint64_t)_fr[5]<<8)|((uint64_t)_fr[4]<<16)|((uint64_t)_fr[3]<<24);}
  inline uint8_t fine() const {return _fr[7];}
  #ifdef BCIDFROMCOARSE
  inline uint16_t bcid(){return (uint16_t) (coarse()*TDC_COARSE_TIME/200);}
  #else
  inline  uint16_t bcid() const {return (_fr[2]|(_fr[1]<<8));}
  #endif
  inline  double tdcTime() const { return (coarse()*TDC_COARSE_TIME+fine()*TDC_FINE_TIME);}
  inline uint8_t* frame(){ return _fr;}
  inline bool used(){return _used;}
  inline void setUsed(bool t){_used=t;}
  bool operator<(const TdcChannel &ipaddr){
    if( coarse() < ipaddr.coarse())
    return true;
  else
    return false;
}
private:
  uint8_t* _fr;
  bool _used;
};
};
#endif
