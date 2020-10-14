#include "C4iInterface.hh"
using namespace lydaq;

/// Board

lydaq::c4i::board(std::string ip) : _ip(ip)
{
  _regh=new c4i::registerHandler(_ip);
  _slch=new c4i::slcHandler(_ip);
  _datah=new c4i::dataHandler(_ip);
}


				   
