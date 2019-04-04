#ifndef _WIZNET_MESSAGE_HANDLER_HH
#define _WIZNET_MESSAGE_HANDLER_HH
#include "MessageHandler.hh"
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/bind.hpp>

#include <map>
#include <string>
/*!
 * \brief Buffer class with size , char[]
 * */
typedef std::pair<uint32_t, unsigned char *> ptrBuf;

/*!
 * \brief boost functor to handle completed FEB buffer
 * */
typedef boost::function<void(uint64_t, uint16_t, char *)> FEBFunctor;

namespace lydaq
{
/*!
* \brief Implementation of MessageHandler
*/
class WiznetMessageHandler : public lytdc::MessageHandler
{
public:
  /// Constructor
  WiznetMessageHandler(std::string directory);
  /// message processing
  virtual void processMessage(NL::Socket *socket);
  /// Utility to convert IP in integer
  static uint32_t convertIP(std::string hname);
  /// Add a buffer processor
  void addHandler(uint64_t id, FEBFunctor f);
  /// Socket handling
  void removeSocket(NL::Socket *socket);
  /**
     * \brief Scan of the network
     * \param class C network (ex 192.168.10.)
     * \return a map of IP address (integer) and devices names
     */
  static std::map<uint32_t, std::string> scanNetwork(std::string base);

private:
  std::string _storeDir;
  std::map<uint64_t, ptrBuf> _sockMap;      ///< map {IP (integer), Buffer}
  std::map<uint64_t, FEBFunctor> _handlers; ///< map {IP(integer), Handler}
  uint64_t _npacket;
};
}; // namespace lydaq
#endif
