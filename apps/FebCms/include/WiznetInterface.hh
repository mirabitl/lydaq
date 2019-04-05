#ifndef _WIZNET_INTERFACE_HH
#define _WIZNET_INTERFACE_HH

#include "WiznetMessageHandler.hh"
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <zlib.h>
#include <iostream>
#include "ReadoutLogger.hh"

/*!
* \file WiznetInterface.hh
 * \brief Method to interact with the Wiznet ethernet interface
 * \author L.Mirabito
 * \version 1.0
*/
#define MAX_BUFFER_LEN 0x3FFC
namespace lydaq
{
/**
   * \class WiznetMessage
   * \brief Handling address and message buffer
   * */
class WiznetMessage
{
public:
  WiznetMessage() : _address(0), _length(0) { ; }
  uint64_t _address;
  uint16_t _length;
  uint16_t _buf[MAX_BUFFER_LEN];
};

/**
   * \class WiznetInterface
   * \brief Handling NETLINK socket initialisation, communication and data handler 
   * */
class WiznetInterface
{
public:
  /// Constructor
  WiznetInterface();
  ~WiznetInterface() { ; }
  /// Create socket group and WiznetMessageHandler
  void initialise();
  /// Add a FEB socket for communication (10001)
  void addCommunication(std::string address, uint16_t port);
  /// Add a FEB socket for data transfer (10002)
  void addDataTransfer(std::string address, uint16_t port);
  /// start listenning
  void listen();
  /// Send a message, the address is encoded in the message
  void sendMessage(WiznetMessage *wmsg);
  /// Add a data packet handler for the WiznetMessageHandler
  void registerDataHandler(std::string address, uint16_t port, FEBFunctor f);
  /// Return the list of communication sockets
  inline std::map<uint64_t, NL::Socket *> &controlSockets() { return _vsCtrl; }
  /// Send FEB slow control
  void writeRamAvm(NL::Socket *sctrl, uint16_t *_slcAddr, uint16_t *_slcBuffer, uint32_t _slcBytes);

private:
  /// Thread to listen packet on socket (select)
  void dolisten();

  std::map<uint64_t, NL::Socket *> _vsCtrl;
  std::map<uint64_t, NL::Socket *> _vsTdc;

  NL::SocketGroup *_group;

  lydaq::WiznetMessageHandler *_msh;
  lytdc::OnRead *_onRead;
  lytdc::OnAccept *_onAccept;
  lytdc::OnClientDisconnect *_onClientDisconnect;
  lytdc::OnDisconnect *_onDisconnect;
  boost::thread_group g_store;
  boost::thread_group g_run;
  bool _running;
};
}; // namespace lydaq
#endif
