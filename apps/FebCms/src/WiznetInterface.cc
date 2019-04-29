#include "WiznetInterface.hh"
using namespace lytdc;
#include <unistd.h>
#include <sys/dir.h>
#include <sys/param.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <iostream>
#include <sstream>
#include <arpa/inet.h>
#include <boost/format.hpp>
#include "ReadoutLogger.hh"

using namespace lydaq;

void lydaq::WiznetInterface::dolisten()
{

  while (true)
  {

    if (!_group->listen(2000))
      LOG4CXX_DEBUG(_logFeb, "No messages received last 2 seconds");
    //std::cout << "\nNo msg recieved during the last 2 seconds";
  }
}
void lydaq::WiznetInterface::listen()
{
  g_store.create_thread(boost::bind(&lydaq::WiznetInterface::dolisten, this));
  _running = true;
  // Comment out for ZDAQ running
  //g_run.create_thread(boost::bind(&lydaq::TdcManager::doStart, this));
}

lydaq::WiznetInterface::WiznetInterface() : _group(NULL)
{
  // Initialise NetLink
  NL::init();
  _msh = new lydaq::WiznetMessageHandler("/dev/shm");
}

void lydaq::WiznetInterface::initialise()
{

  // Initialise the network
  if (_group != NULL)
    delete _group;
  _group = new NL::SocketGroup();
  _onRead = new lytdc::OnRead(_msh);
  _onClientDisconnect = new lytdc::OnClientDisconnect();
  _onDisconnect = new lytdc::OnDisconnect(_msh);
  _onAccept = new lytdc::OnAccept(_msh);
  _group->setCmdOnRead(_onRead);
  _group->setCmdOnAccept(_onAccept);
  _group->setCmdOnDisconnect(_onClientDisconnect);
  // Loop on Asic Map and find existing DIF
  // Register their slow control socket (10001) and readout one (10002)
  _vsCtrl.clear();
  _vsTdc.clear();
}
void lydaq::WiznetInterface::addCommunication(std::string address, uint16_t port)
{
  uint64_t id = ((uint64_t)lydaq::WiznetMessageHandler::convertIP(address) << 32) | port;

  NL::Socket *sc = new NL::Socket(address, port);
  _group->add(sc);
  std::pair<uint64_t, NL::Socket *> p1(id, sc);
  LOG4CXX_INFO(_logFeb, "Communication socket added on" << address << ":" << port);
  //printf("Communication on %s %d \n", address.c_str(), port);
  _vsCtrl.insert(p1);
}
void lydaq::WiznetInterface::addDataTransfer(std::string address, uint16_t port)
{
  uint64_t id = ((uint64_t)lydaq::WiznetMessageHandler::convertIP(address) << 32) | port;
  NL::Socket *stdc = new NL::Socket(address, port);
  _group->add(stdc);
  std::pair<uint64_t, NL::Socket *> p1(id, stdc);
  LOG4CXX_INFO(_logFeb, "Data transfert socket added on" << address << ":" << port);

  //printf("Data on %s %d \n", address.c_str(), port);
  _vsTdc.insert(p1);
}

void lydaq::WiznetInterface::sendMessage(WiznetMessage *m)
{
  std::map<uint64_t, NL::Socket *>::iterator itsock = _vsCtrl.find(m->_address);
  uint32_t addr = (m->_address >> 32) & 0xFFFFFFFF;
  uint16_t port = m->_address & 0XFFFF;
  if (itsock == _vsCtrl.end())
  {
    LOG4CXX_ERROR(_logFeb, "Address not found =>" << addr << ":" << port);
    return;
    //printf("Address %x on port %d not found \n", (m->_address >> 32) & 0xFFFFFFFF, m->_address & 0XFFFF);
  }
  // Send the Buffer
  try
  {
    itsock->second->send((const void *)m->_buf, m->_length * sizeof(uint16_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
  LOG4CXX_DEBUG(_logFeb, "Buffer sent with " << m->_length << " bytes @ " << addr << ":" << port);
  //printf("Buffer sent %d bytes at Address %lx on port %d \n", m->_length, (m->_address >> 32) & 0xFFFFFFF, m->_address & 0XFFFF);
}

void lydaq::WiznetInterface::registerDataHandler(std::string address, uint16_t port, FEBFunctor f)
{
  uint64_t id = ((uint64_t)lydaq::WiznetMessageHandler::convertIP(address) << 32) | port;
  _msh->addHandler(id, f);
}

void lydaq::WiznetInterface::writeRamAvm(NL::Socket *sctrl, uint16_t *_slcAddr, uint16_t *_slcBuffer, uint32_t _slcBytes)
{
  if (_slcBuffer[1] < 2)
    return;
  uint16_t sockbuf[0x20000];
  sockbuf[0] = htons(0xFF00);
  sockbuf[1] = htons(_slcBytes);
  int idx = 2;
  for (int i = 0; i < _slcBytes; i++)
  {
    sockbuf[idx] = htons(_slcAddr[i]);
    sockbuf[idx + 1] = htons(_slcBuffer[i]);
    idx += 2;
  }
  //
  // Creating file  with name = SLC
  std::stringstream sb;
  for (int i = 0; i < _slcBytes - 1; i++)
  {

    sb << std::hex << (int)(_slcBuffer[i] & 0xFF);
  }
  char name[2512];
  memset(name, 0, 2512);
  sprintf(name, "/dev/shm/%s/%d/%s", sctrl->hostTo().c_str(), sctrl->portTo(), sb.str().c_str());
  int fd = ::open(name, O_CREAT | O_RDWR | O_NONBLOCK, S_IRWXU);
  LOG4CXX_DEBUG(_logFeb, " Creating SHM area " << name);

  ::close(fd);
  // Send the Buffer
  try
  {
    sctrl->send((const void *)sockbuf, idx * sizeof(uint16_t));
  }
  catch (NL::Exception e)
  {
    throw e.msg();
  }
}
