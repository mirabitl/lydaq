#ifndef _MessageHandler_h_
#define _MessageHandler_h_
/*!
* \file MessageHandler.hh
 * \brief Handelers of netlink library events
 * \author L.Mirabito
 * \version 1.0
*/
#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>
#include "debug.hh"

/*! \namespace lytdc
  *
  * \brief namespace grouping all netlink handlers used for FebCms
*/
namespace lytdc
{

/*!
* \brief purely virtual class to handle FEB messages
*/
class MessageHandler
{
public:
  virtual void processMessage(NL::Socket *socket) { ; }
  virtual void removeSocket(NL::Socket *sock) { ; }
};

/**
 * \brief Net link accept handler
 * 
 * */
class OnAccept : public NL::SocketGroupCmd
{

public:
  OnAccept(MessageHandler *msh);
  void exec(NL::Socket *socket, NL::SocketGroup *group, void *reference);

private:
  MessageHandler *_msh;
};

/**
 * \brief Buffer processing handler
 * */
class OnRead : public NL::SocketGroupCmd
{
public:
  OnRead(MessageHandler *msh);
  void exec(NL::Socket *socket, NL::SocketGroup *group, void *reference);

public:
  unsigned char _readBuffer[0x10000];

private:
  MessageHandler *_msh;
};

/**
 * \brief Server disconnection
 * */
class OnDisconnect : public NL::SocketGroupCmd
{
public:
  OnDisconnect(MessageHandler *msh);
  void exec(NL::Socket *socket, NL::SocketGroup *group, void *reference);
  bool disconnected() { return _disconnect; }

private:
  MessageHandler *_msh;
  bool _disconnect;
};

/** 
 * \brief Client Disconnected 
 * */

class OnClientDisconnect : public NL::SocketGroupCmd
{
public:
  OnClientDisconnect();
  void exec(NL::Socket *socket, NL::SocketGroup *group, void *reference);
  bool disconnected() { return _disconnect; }

private:
  bool _disconnect;
};
}; // namespace lytdc
#endif
