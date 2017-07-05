
#include <iostream>
#include <string.h>
#include<stdio.h>
#include "MessageHandler.h"

#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <boost/thread.hpp>

#include <netlink/socket.h>
#include <netlink/socket_group.h>
#include <string>

using namespace std;

static uint32_t transaction_id_=1;
boost::mutex global_stream_lock;



MessageHandler::MessageHandler()
{
  this->registerCommand("SUBSCRIBE",boost::bind(&MessageHandler::StructureHandler,this,_1));
  this->registerCommand("SERVICES",boost::bind(&MessageHandler::StructureHandler,this,_1));
}
NetMessage* MessageHandler::StructureHandler(NetMessage* m)
{
  if(m->getName().compare("SUBSCRIBE")==0)
    {

      std::string name((const char*) m->getPayload(),m->getPayloadSize());
      printf("%s I got a SUBSCRIBE COMMAND %s %d \n",__PRETTY_FUNCTION__,name.c_str(), m->getPayloadSize());
      this->subscribeService(name,readSocket_);
      return NULL;
    }
  if(m->getName().compare("SERVICES")==0)
    {
      std::string name((const char*) m->getPayload(),m->getPayloadSize());
      this->subscribeService(name,readSocket_);
      return NULL;
    }
     
  return NULL;
}
void MessageHandler::processMessage(NL::Socket* socket) throw (std::string)
{
  global_stream_lock.lock(); 
  size_t ier=0;
  uint32_t size_remain=4;
  while (size_remain>0)
    {
       try 
	 {
	   ier=socket->read(&buf_[4-size_remain],size_remain);
	 }
       catch (NL::Exception e)
	 {
	   printf("%s Error message when reading block %s \n",__PRETTY_FUNCTION__,e.msg().c_str());
	   return;
	 }
      if (ier<0)
	break;
      size_remain -=ier;
    }
  memcpy(&msgLen_,buf_,4);
  DEBUG("Receive msg length of %d %d \n",ier,msgLen_);

 size_remain=msgLen_;
  while (size_remain>0)
    {
       try 
	 {
	   ier=socket->read(&buf_[msgLen_-size_remain],size_remain);
	 }
       catch (NL::Exception e)
	 {
	   printf("%s Error message when reading block %s \n",__PRETTY_FUNCTION__,e.msg().c_str());
	   throw e.msg();
	   return;
	 }
      if (ier<0)
	break;
      size_remain -=ier;
    }
  global_stream_lock.unlock();
  INFO("Receive data of %d \n",ier);
  //  if (ier!=msgLen_)
  //  throw std::string("wrong message length");
  NetMessage m( boost::asio::mutable_buffer(&buf_[4],msgLen_-8));
  uint32_t* ibuf =(uint32_t*) buf_;
  DEBUG("%d %d %d \n",ibuf[0],ibuf[1],ibuf[2]);
  DEBUG("%s \n",&buf_[8]);
  DEBUG("%d Type \n",m.getType());
  if (m.getType()==NetMessage::COMMAND || (m.getType()==NetMessage::SERVICE)|| (m.getType()==NetMessage::COMMAND_ACKNOWLEDGE) ) 
    {
      readSocket_=socket;
      std::map<std::string,boost::function<NetMessage*(NetMessage*)> >::iterator it=commandHandler_.find(m.getName());
      INFO("On cherhce le handler pour %s \n",m.getName().c_str());
      if (it!=commandHandler_.end())
	{
	  boost::function<NetMessage*(NetMessage*)> f =(it->second);
	  INFO("On appelle le handler pour %s \n",m.getName().c_str());
	  NetMessage* rep=f(&m);
	  if (rep!=NULL)
	    {
	      INFO("On poste la reponse pour %s \n",m.getName().c_str());
	      try {
	      MessageHandler::postMessage(socket,rep);
	      }
	      catch (NL::Exception e)
		{
		  std::cout<<e.msg()<<std::endl;
		}
	      delete rep;
	      
	    }
	}
    }


} 
void MessageHandler::registerCommand(std::string cmd,boost::function<NetMessage* (NetMessage*)> f)
{
  std::map<std::string,boost::function<NetMessage*(NetMessage*)> >::iterator it=commandHandler_.find(cmd);
  if(it==commandHandler_.end())
    {
      std::pair<std::string,boost::function<NetMessage*(NetMessage*)> > p(cmd,f);
      commandHandler_.insert(p);
    }
}


void MessageHandler::postMessage(NL::Socket* socket,NetMessage* m) throw (std::string)
{
  global_stream_lock.lock();
  int theLength=m->getSize()+8;
  unsigned char buf[4];
  memcpy(buf,&theLength,4);
  try {
  socket->send((const void*)&theLength,4);
  }
  catch (NL::Exception e)
    {
      global_stream_lock.unlock();  
      throw e.msg();
    }
  
  //if (ier!=4)
  //  throw std::string("Cannot send message length");
  
  boost::shared_ptr<unsigned char> p(new unsigned char[m->getSize()+8]) ;
  memcpy(&(p.get())[4],m->getMessage(),m->getSize());
  int* ibuf=(int*) p.get();
  ibuf[0]=transaction_id_++;
  //ibuf[(m->getSize()+8)/4-1]=transaction_id_;
  boost::asio::mutable_buffer bbuf( p.get(),m->getSize()+8);


  DEBUG("%s sending %d bytes %lx %lx \n",__PRETTY_FUNCTION__,ibuf[0],(unsigned long) p.get(),(unsigned long) boost::asio::buffer_cast<uint32_t*>(bbuf));
  // global_stream_lock.lock();

  try
    {
  socket->send((const void*) p.get(),theLength);
  }
  catch (NL::Exception e)
    {
      global_stream_lock.unlock();  
      throw e.msg();
    }

  //global_stream_lock.unlock();
  //if (ier!=theLength)
  //  throw std::string("Cannot send message");
  global_stream_lock.unlock();  
}
void MessageHandler::destroyService(std::string s)
{
  serviceHandler_.erase(s);
}
void MessageHandler::registerService(std::string s)
{
  std::vector<NL::Socket*> v;
  std::pair<std::string,std::vector<NL::Socket*> > p(s,v);
  serviceHandler_.insert(p);
}
void MessageHandler::subscribeService(std::string s,NL::Socket* sock)
{
  std::map<std::string,std::vector<NL::Socket*> >::iterator it=serviceHandler_.find(s);
  if(it==serviceHandler_.end()) return;
  it->second.push_back(sock);
}
void MessageHandler::updateService(std::string s,NetMessage* m)
{
  std::map<std::string,std::vector<NL::Socket*> >::iterator it=serviceHandler_.find(s);
  if (it==serviceHandler_.end()) 
    {
      printf("Service %s not found\n",s.c_str());
      return;
    }
  //printf("Service %s  found\n",s.c_str());

  for (std::vector<NL::Socket*>::iterator jt=it->second.begin();jt!=it->second.end();jt++)
    {

      //printf("sending message %s \n",m->getName().c_str());
      MessageHandler::postMessage((*jt),m);
    }
}
void MessageHandler::removeSocket(NL::Socket* s)
{
  for(std::map<std::string,std::vector<NL::Socket*> >::iterator it=serviceHandler_.begin();it!=serviceHandler_.end();it++)
    {
      for (std::vector<NL::Socket*>::iterator jt=it->second.begin();jt!=it->second.end();)
	{
	  if ((*jt)==s)
	    it->second.erase(jt);
	  else
	    ++jt;
	}
    }
}



OnAccept::OnAccept(MessageHandler* msh) : msh_(msh) {}
void OnAccept:: exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) 
{
  NL::Socket* newConnection = socket->accept();
  group->add(newConnection);
  cout << "\nConnection " << newConnection->hostTo() << ":" << newConnection->portTo() << " added...";
  cout.flush();
}




OnRead::OnRead(MessageHandler* msh) : msh_(msh) {}
void OnRead::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  msh_->processMessage(socket);
  /*
    cout << "\nREAD -- ";
    cout.flush();
    unsigned char buffer[256];
    buffer[255] = '\0';
    memset(theReadBuffer_,0,0x10000);
    //socket->read(buffer, 255);
    size_t msgLen =socket->read(theReadBuffer_,0x10000);
    std::string sread(buffer);
    cout << "Message from " << socket->hostTo() << ":" << socket->portTo() << ". Text received: " << sread<<std::endl;
    cout.flush();
		
    for(unsigned i=1; i < (unsigned) group->size(); ++i)
    {
    if(group->get(i) != socket)
    {
    printf(" \t sending %d  %d %s \n",i,sread.size(),sread.c_str());
    group->get(i)->send(sread.c_str(), sread.size());
    }
    }
  */
}

OnDisconnect::OnDisconnect(MessageHandler* msh) : msh_(msh) {}
void OnDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  group->remove(socket);
  cout << "\nClient " << socket->hostTo() << " disconnected...";
  msh_->removeSocket(socket);
  cout.flush();
  delete socket;
}
void OnClientDisconnect::exec(NL::Socket* socket, NL::SocketGroup* group, void* reference) {

  uint32_t* i =(uint32_t*) reference;
  (*i)=0xDEAD;
}
