#include "Zup.hh"


using namespace lydaq;
lydaq::Zup::Zup(std::string device,uint32_t address)
{

  fd1=open(device.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);

  if (fd1 == -1 )

    {

      perror("open_port: Unable to open /dev/ttyS0 – ");

    }

  else

    {

      fcntl(fd1, F_SETFL,0);
      printf("Port 1 has been sucessfully opened and %d is the file description\n",fd1);
    }


  int portstatus = 0;

  struct termios options;
  // Get the current options for the port...
  tcgetattr(fd1, &options);
  // Set the baud rates to 115200...
  cfsetispeed(&options, B9600);
  cfsetospeed(&options, B9600);
  // Enable the receiver and set local mode...
  options.c_cflag |= (CLOCAL | CREAD);

  options.c_cflag &= ~PARENB;
  options.c_cflag &= ~CSTOPB;
  options.c_cflag &= ~CSIZE;
  options.c_cflag |= CS8;
  //options.c_cflag |= SerialDataBitsInterp(8);           /* CS8 - Selects 8 data bits */
  options.c_cflag &= ~CRTSCTS;                            // disable hardware flow control
  options.c_iflag &= ~(IXON | IXOFF | IXANY);           // disable XON XOFF (for transmit and receive)
  //options.c_cflag |= CRTSCTS;                     /* enable hardware flow control */


  options.c_cc[VMIN] = 1;     //min carachters to be read
  options.c_cc[VTIME] = 1;    //Time to wait for data (tenths of seconds)


  // Set the new options for the port...
  tcsetattr(fd1, TCSANOW, &options);


  //Set the new options for the port...
  tcflush(fd1, TCIFLUSH);
  if (tcsetattr(fd1, TCSANOW, &options)==-1)
    {
      perror("On tcsetattr:");
      portstatus = -1;
    }
  else
    portstatus = 1;


  char hadr[20];
  memset(hadr,0,20);
  sprintf(hadr,":ADR%.2d;\n",address);

  wr=write(fd1,hadr,7);usleep(50000);
  printf("%d Bytes sent are %d \n",portstatus,wr);

}
lydaq::Zup::~Zup()
{
  if (fd1>0)
    close(fd1);
}
void lydaq::Zup::ON()
{
    
  wr=write(fd1,":OUT1;",6);usleep(50000);
  //printf("Bytes sent are %d \n",wr);
}
void lydaq::Zup::OFF()
{

  wr=write(fd1,":OUT0;",6);usleep(50000);
  //printf("Bytes sent are %d \n",wr);

}

void lydaq::Zup::readCommand(std::string cmd)
{
  //std::cout<<cmd<<std::endl;
  wr=write(fd1,cmd.substr(0,6).c_str(),6);
  //std::cout<<"sleep "<<std::endl;
  for (int i=0;i<500;i++) usleep(100);
  memset(buff,0,100);
  int32_t nchar=0,rd=0;
  while (nchar==0)
    {
      rd=read(fd1,&buff[nchar],100);
      //std::cout<<"rd "<<rd<<" nch"<<nchar<<std::endl;
      if (rd>0)
	nchar=rd;
      else
	usleep(50);
    }
  // Repete la lecture tant qu'il ya des char
  while (rd>1)
    {
      rd=read(fd1,&buff[nchar],100);
      //std::cout<<"rd "<<rd<<" nch"<<nchar<<std::endl;
      if (rd>1) nchar+=rd;

    }
  //
  memset(&buff[nchar-1],0,100-(nchar-1));
  //buff[nchar-1]=0;
  std::string ret(buff);
  std::cout<<ret<<std::endl;
  //printf("%s\n",buff);
}
void lydaq::Zup::INFO()
{
  this->readCommand(":MDL?;");
  this->readCommand(":VOL!;");
  this->readCommand(":VOL?;");
  this->readCommand(":CUR?;");
  /*
    wr=write(fd1,":MDL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":VOL!;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":VOL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
    wr=write(fd1,":CUR?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
  */
}
float lydaq::Zup::ReadVoltageSet()
{
  this->readCommand(":VOL!;");
  /*
    wr=write(fd1,":VOL!;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
  */
  float v;
  sscanf(buff,"SV%f",&v);
  return v;
}
float lydaq::Zup::ReadVoltageUsed()
{
  this->readCommand(":VOL?;");
  /*
    wr=write(fd1,":VOL?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
  */
  float v;
  sscanf(buff,"AV%f",&v);
  return v;
}
float lydaq::Zup::ReadCurrentUsed()
{
  this->readCommand(":CUR?;");
  /*
    wr=write(fd1,":CUR?;",6);usleep(50000);
    memset(buff,0,100);rd=read(fd1,buff,100); printf("%s \n",buff);
  */
  float v;
  sscanf(buff,"AA%f",&v);
  return v;
}
