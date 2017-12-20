#include  "N1470.hh"

using namespace std;
using namespace lydaq;
void lydaq::N1470::setIos()
{
  struct termios oldtio,newtio;
  char buf[255];
  /* 
     Open modem device for reading and writing and not as controlling tty
     because we don't want to get killed if linenoise sends CTRL-C.
  */
            
  tcgetattr(_fd,&oldtio); /* save current serial port settings */
  bzero(&newtio, sizeof(newtio)); /* clear struct for new port settings */
        
  /* 
     BAUDRATE: Set bps rate. You could also use cfsetispeed and cfsetospeed.
     CRTSCTS : output hardware flow control (only used if the cable has
     all necessary lines. See sect. 7 of Serial-HOWTO)
     CS8     : 8n1 (8bit,no parity,1 stopbit)
     CLOCAL  : local connection, no modem contol
     CREAD   : enable receiving characters
  */
  //newtio.c_cflag = B9600 | CRTSCTS | CS8 | CLOCAL | CREAD;
  newtio.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
	 
  /*
    IGNPAR  : ignore bytes with parity errors
    ICRNL   : map CR to NL (otherwise a CR input on the other computer
    will not terminate input)
    otherwise make device raw (no other input processing)
  */
  //  newtio.c_iflag = IGNPAR | ICRNL;
         
  /*
    Raw output.
  */
  newtio.c_oflag = 0;
         
  /*
    ICANON  : enable canonical input
    disable all echo functionality, and don't send signals to calling program
  */
  newtio.c_lflag = ICANON;
         
  /* 
     initialize all control characters 
     default values can be found in /usr/include/termios.h, and are given
     in the comments, but we don't need them here
  */
  newtio.c_cc[VINTR]    = 0;     /* Ctrl-c */ 
  newtio.c_cc[VQUIT]    = 0;     /* Ctrl-\ */
  newtio.c_cc[VERASE]   = 0;     /* del */
  newtio.c_cc[VKILL]    = 0;     /* @ */
  newtio.c_cc[VEOF]     = 4;     /* Ctrl-d */
  newtio.c_cc[VTIME]    = 0;     /* inter-character timer unused */
  newtio.c_cc[VMIN]     = 1;     /* blocking read until 1 character arrives */
  newtio.c_cc[VSWTC]    = 0;     /* '\0' */
  newtio.c_cc[VSTART]   = 0;     /* Ctrl-q */ 
  newtio.c_cc[VSTOP]    = 0;     /* Ctrl-s */
  newtio.c_cc[VSUSP]    = 0;     /* Ctrl-z */
  newtio.c_cc[VEOL]     = 0;     /* '\0' */
  newtio.c_cc[VREPRINT] = 0;     /* Ctrl-r */
  newtio.c_cc[VDISCARD] = 0;     /* Ctrl-u */
  newtio.c_cc[VWERASE]  = 0;     /* Ctrl-w */
  newtio.c_cc[VLNEXT]   = 0;     /* Ctrl-v */
  newtio.c_cc[VEOL2]    = 0;     /* '\0' */
        
  /* 
     now clean the modem line and activate the settings for the port
  */
  tcflush(_fd, TCIFLUSH);
  tcsetattr(_fd,TCSANOW,&newtio);
         
}
lydaq::N1470::N1470(std::string device,uint32_t board) : _board(board)
{

  _fd=open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

  if (_fd == -1 )

    {

      perror("open_port: Unable to open /dev/ttyS0 – ");

    }

  else

    {

      fcntl(_fd, F_SETFL,0);
      printf("Port 1 has been sucessfully opened and %d is the file description\n",_fd);
    }

  int portstatus = 0;
  setIos();


  stringstream s;
  s<<"$BD:"<<_board<<",CMD:MON,PAR:BDCTR\r";
  this->readCommand(s.str());
  std::cout<<this->readValue()<<std::endl;
}
lydaq::N1470::~N1470()
{
  if (_fd>0)
    close(_fd);
}
void lydaq::N1470::readCommand(std::string cmd)
{
  memset(_buff,0,1024);
  fd_set set;
  struct timeval timeout;
  int rv;

  FD_ZERO(&set); /* clear the set */
  FD_SET(_fd, &set); /* add our file descriptor to the set */

  timeout.tv_sec = 0;
  timeout.tv_usec = 10000;

  // rv = select(_fd + 1, &set, NULL, NULL, &timeout);
  // if(rv == -1)
  //   perror("select"); /* an error accured */
  // else if(rv != 0)
  //   {
  //     read( _fd, _buff, 100 ); /* there was data to read */
    
  //     //std::cout<<"Y avait "<<_buff<<std::endl;
  //   }
  wr=write(_fd,cmd.c_str(),cmd.length());
  //std::cout<<"sleep "<<cmd<<std::endl;
  //for (int i=0;i<20;i++) usleep(1000);
  memset(_buff,0,1024);
  int32_t nchar=0,rd=0;
  if (1)
    {
      FD_ZERO(&set); /* clear the set */
      FD_SET(_fd, &set); /* add our file descriptor to the set */
	
      timeout.tv_sec = 0;
      timeout.tv_usec = 480000;


      rv = select(_fd + 1, &set, NULL, NULL, &timeout);
      if(rv == -1)
	{
	  perror("select"); /* an error accured */
	}
      else if(rv == 0)
	{
	  //printf("Nothing in select \n"); /* a timeout occured */
	  return;
	}
      else
	{

	  rd=read(_fd,&_buff[nchar],100);
	  //printf(" rd = %d nchar %d %s\n",rd,nchar,_buff);
	  if (rd>0)
	    nchar+=rd;
	}
      usleep(1);
    }

    
  //printf("nchar %d OOOLLAA %s\n",nchar,_buff);

  int istart=0;
  char bufr[100];
  memset(bufr,0,100);

  for (int i=0;i<nchar;i++)
    if (_buff[i]<0x5f) {bufr[istart]=_buff[i];istart++;}
  //memcpy(bufr,&_buff[istart],nchar-istart);
  std::string toto;toto.assign(bufr,istart-1);
  //printf(" %d %d Corrected %s\n",istart,nchar,toto.c_str());
    
  _value=toto;
}

void lydaq::N1470::setOn(uint32_t ch)
{
  std::stringstream s;
  s<<"$BD:"<<_board<<",CMD:SET,CH:"<<ch<<",PAR:ON\r\n";
  this->readCommand(s.str());
  //::sleep(1);
  //this->status(ch);
}
void lydaq::N1470::setOff(uint32_t ch)
{
  std::stringstream s;
  s<<"$BD:"<<_board<<",CMD:SET,CH:"<<ch<<",PAR:OFF\r\n";
  this->readCommand(s.str());
  //::sleep(1);
  //this->status(ch);
}
void lydaq::N1470::setVoltage(uint32_t ch,float val)
{
  std::stringstream s;
  s<<"$BD:"<<_board<<",CMD:SET,CH:"<<ch<<",PAR:VSET,VAL:"<<val<<"\r\n";
  this->readCommand(s.str());
  ::sleep(1);
  this->status(ch);
}
void lydaq::N1470::setCurrent(uint32_t ch,float val)
{
  std::stringstream s;
  s<<"$BD:"<<_board<<",CMD:SET,CH:"<<ch<<",PAR:ISET,VAL:"<<val<<"\r\n";
  this->readCommand(s.str());
  ::sleep(1);
  this->status(ch);
}
void lydaq::N1470::setRampUp(uint32_t ch,float val)
{
  std::stringstream s;
  s<<"$BD:"<<_board<<",CMD:SET,CH:"<<ch<<",PAR:RUP,VAL:"<<val<<"\r\n";
  this->readCommand(s.str());
  ::sleep(1);
  this->status(ch);
}
void lydaq::N1470::setRampDown(uint32_t ch,float val)
{
  std::stringstream s;
  s<<"$BD:"<<_board<<",CMD:SET,CH:"<<ch<<",PAR:RDW,VAL:"<<val<<"\r\n";
  this->readCommand(s.str());
  ::sleep(1);
  this->status(ch);
}

void lydaq::N1470::status(uint32_t ch)
{
  std::size_t found;
  std::stringstream sout;
  do {
    std::stringstream s;
    s<<"$BD:"<<_board<<",CMD:MON,CH:"<<ch<<",PAR:POL\r\n";
    this->readCommand(s.str());
    found=_value.find("OK");
    if (found == std::string::npos) continue;;
     found=_value.find("VAL:");
    if (found == std::string::npos) continue;;
    sout<<"Channel :"<<ch<<"\nPolarity :"<< _value.substr(found+4,_value.size()-(found+4)).c_str()<<std::endl;
    // Status
    s.str("");
    s<<"$BD:"<<_board<<",CMD:MON,CH:"<<ch<<",PAR:STAT\r\n";
    this->readCommand(s.str());
    found=_value.find("OK");
    if (found == std::string::npos) continue;;
    //std::cout<<_value<<std::endl;
    found=_value.find("VAL:");
    if (found == std::string::npos) continue;;
    sscanf( _value.substr(found+4,_value.size()-(found+4)).c_str(),"%d",&_statusBits);

    sout<<"Status :"<<_statusBits<<std::endl;
    // VSET
    s.str("");
    s<<"$BD:"<<_board<<",CMD:MON,CH:"<<ch<<",PAR:VSET\r\n";
    this->readCommand(s.str());
    found=_value.find("OK");
    if (found == std::string::npos) continue;;
    //std::cout<<_value<<std::endl;
    found=_value.find("VAL:");
    if (found == std::string::npos) continue;;
    sscanf( _value.substr(found+4,_value.size()-(found+4)).c_str(),"%f",&_vSet);
    sout<<"Vset :"<<_vSet<<std::endl;
    // ISET
    s.str("");
    s<<"$BD:"<<_board<<",CMD:MON,CH:"<<ch<<",PAR:ISET\r\n";
    this->readCommand(s.str());
    found=_value.find("OK");
    if (found == std::string::npos) continue;;
    //std::cout<<_value<<std::endl;
    found=_value.find("VAL:");
    if (found == std::string::npos) continue;;
    sscanf( _value.substr(found+4,_value.size()-(found+4)).c_str(),"%f",&_iSet);
    sout<<"Iset :"<<_iSet<<std::endl;
    // VMON
    s.str("");
    s<<"$BD:"<<_board<<",CMD:MON,CH:"<<ch<<",PAR:VMON\r\n";
    this->readCommand(s.str());
    found=_value.find("OK");
    if (found == std::string::npos) continue;;
    //std::cout<<_value<<std::endl;
    found=_value.find("VAL:");
    if (found == std::string::npos) continue;;
    sscanf( _value.substr(found+4,_value.size()-(found+4)).c_str(),"%f",&_vMon);
    sout<<"Vmon :"<<_vMon<<std::endl;
    
    // IMON
    s.str("");
    s<<"$BD:"<<_board<<",CMD:MON,CH:"<<ch<<",PAR:IMON\r\n";
    this->readCommand(s.str());
    found=_value.find("OK");
    if (found == std::string::npos) continue;;
    //std::cout<<_value<<std::endl;

    found=_value.find("VAL:");
    if (found == std::string::npos) continue;;
    sscanf( _value.substr(found+4,_value.size()-(found+4)).c_str(),"%f",&_iMon);
    sout<<"Imon :"<<_iMon<<std::endl;
    // RUP
    s.str("");
    s<<"$BD:"<<_board<<",CMD:MON,CH:"<<ch<<",PAR:RUP\r\n";
    this->readCommand(s.str());
    found=_value.find("OK");
    if (found == std::string::npos) continue;;
    //    std::cout<<_value<<std::endl;

    found=_value.find("VAL:");
    if (found == std::string::npos) continue;;
    sscanf( _value.substr(found+4,_value.size()-(found+4)).c_str(),"%f",&_rampUp);
    sout<<"Ramp Up :"<<_rampUp<<std::endl;

    // RDW
    s.str("");
    s<<"$BD:"<<_board<<",CMD:MON,CH:"<<ch<<",PAR:RDW\r\n";
    this->readCommand(s.str());
    found=_value.find("OK");
    if (found == std::string::npos) continue;;

    found=_value.find("VAL:");
    if (found == std::string::npos) continue;;
    sscanf( _value.substr(found+4,_value.size()-(found+4)).c_str(),"%f",&_rampDown);
    sout<<"Ramp Down :"<<_rampDown<<std::endl;
    
  } while (found==std::string::npos);
  _lastInfo=time(0);
  std::cout<<sout.str();
  /*
    wr=write(_fd,":MDL?;",6);usleep(50000);
    memset(_buff,0,100);rd=read(_fd,_buff,100); printf("%s \n",_buff);
    wr=write(_fd,":VOL!;",6);usleep(50000);
    memset(_buff,0,100);rd=read(_fd,_buff,100); printf("%s \n",_buff);
    wr=write(_fd,":VOL?;",6);usleep(50000);
    memset(_buff,0,100);rd=read(_fd,_buff,100); printf("%s \n",_buff);
    wr=write(_fd,":CUR?;",6);usleep(50000);
    memset(_buff,0,100);rd=read(_fd,_buff,100); printf("%s \n",_buff);
  */
}
float lydaq::N1470::voltageSet(uint32_t ch)
{
  if (time(0)-_lastInfo > 5) this->status(ch);
  return _vSet;
}
float lydaq::N1470::voltageUsed(uint32_t ch)
{
  if (time(0)-_lastInfo > 5) this->status(ch);
  return _vMon;
}
float lydaq::N1470::currentSet(uint32_t ch)
{
  if (time(0)-_lastInfo > 5) this->status(ch);
  return _iSet;
}
float lydaq::N1470::currentUsed(uint32_t ch)
{
  if (time(0)-_lastInfo > 5) this->status(ch);
  return _iMon;
}
uint32_t lydaq::N1470::statusBits(uint32_t ch)
{
  if (time(0)-_lastInfo > 5) this->status(ch);
  return _statusBits;
}

float lydaq::N1470::rampUp(uint32_t ch)
{
  if (time(0)-_lastInfo > 5) this->status(ch);
  return _rampUp;
}
float lydaq::N1470::rampDown(uint32_t ch)
{
  if (time(0)-_lastInfo > 5) this->status(ch);
  return _rampDown;
}

std::string lydaq::N1470::readValue(){return _value;}
