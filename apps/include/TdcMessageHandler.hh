#ifndef _TDC_MESSAGE_HANDLER_HH
#define _TDC_MESSAGE_HANDLER_HH
#include "MessageHandler.hh"
#include <map>
#include <string>
#include "TdcFpga.hh"
typedef std::pair<uint32_t,unsigned char*> ptrBuf;
namespace lydaq
{
class TdcMessageHandler : public lytdc::MessageHandler
  {
  public:
    TdcMessageHandler(std::string directory);
    virtual void processMessage(NL::Socket* socket) throw (std::string);
    virtual void removeSocket(NL::Socket* sock);
    void parseTdcData(NL::Socket* socket,ptrBuf& p) throw (std::string);
    void parseSlowControl(NL::Socket* socket,ptrBuf& p) throw (std::string);

    static uint32_t convertIP(std::string hname);
    uint32_t readout(uint64_t idsock){return _readout[idsock];}
    void setReadout(uint64_t idsock,uint32_t e){_readout[idsock]=e;}
    std::map<uint64_t,uint32_t>& readoutMap(){return _readout;}
    TdcFpga* tdc(uint32_t i){return _tdc[i];}
    void setMezzanine(uint8_t mezz,std::string host);
    
    void connect(zmq::context_t* c,std::string dest);
  private:
    std::string _storeDir;
    std::map<uint64_t, ptrBuf> _sockMap;
    std::map<uint64_t,uint32_t> _readout;
    TdcFpga* _tdc[2];
  };
};
#endif
