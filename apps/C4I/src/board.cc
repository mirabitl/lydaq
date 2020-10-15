#include "C4iInterface.hh"
using namespace lydaq;

/// Board

lydaq::c4i::board::board(std::string ip) : _ip(ip)
{
  fprintf(stderr,"Creating registeraccess at address %s  \n",ip.c_str());

  _regh=new c4i::registerHandler(ip);
    fprintf(stderr,"Creating slcaccess at address %s  \n",ip.c_str());
  _slch=new c4i::slcHandler(ip);
    fprintf(stderr,"Creating dataaccess at address %s  \n",ip.c_str());
  _datah=new c4i::dataHandler(ip);
}


				   
