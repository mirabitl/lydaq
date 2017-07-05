#ifndef _LocalHardwareException_h
#define _LocalHardwareException_h

#include <iostream>
#include <sstream>
#include <stdint.h>
#include <exception>
 
class LocalHardwareException : public std::exception
{
public:
  LocalHardwareException ( ) {};

  LocalHardwareException( const std::string name,
                           const std::string message, 
                           const std::string module,
                           const int lin,
			  const std::string func) : name_(name),message_(message), module_(module),function_(func),line_(lin)
    {
        std::ostringstream oss;
        oss << "Exception " << name_ << " : "
            << message<<" : "<<module_<<" : "<< function_<<" Line "<<line_;
        this->msg = oss.str();
    }
 
    virtual ~LocalHardwareException() throw()
    {
 
    }
 
    virtual const char * what() const throw()
    {
        return this->msg.c_str();
    }
    const std::string message() {return this->msg;}
private:

    std::string name_,message_,module_,function_;
    uint32_t line_;
    std::string msg;
};
 
#endif
